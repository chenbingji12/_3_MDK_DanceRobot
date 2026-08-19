#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
act40_parser.py -- Parser for Hiwonder "Bus Servo Control" .rob action files
                   (the "ACT-40" format used by LX-16A / LX-224 serial-bus-servo
                   dance robots, e.g. the 18-DOF dance robot with servo IDs 1-19).

Byte-level format (validated against the decompiled official Hiwonder
"Bus Servo Control V3.5" Windows software, 2025 build; saveFileClick /
setActionList methods):

  HEADER (16 bytes):
    [0:6]   magic   = b"ACT-40"          (0x41 0x43 0x54 0x2D 0x34 0x30)
    [6:8]   count   = uint16 little-endian  -> number of frames (action items)
    [8:12]          = 0x00 0x00 0x00 0x00 normally;
                      b"EYPT" marks an encrypted body (TEA-like cipher, see
                      DecryptActionFile in the software)
    [12]    mode    = 0 -> General mode, positions 0..1000   (LX-16A dance robots)
                      1 -> Spider mode (18 servos)
                      2 -> Tony mode (16 servos)
                      3 -> General mode, positions 0..1500
    [13:16]         = reserved (0)

  FRAME (248 bytes each; file offset = 16 + frame_index*248):
    [0:2]   time    = uint16 little-endian, milliseconds (e.g. 0x03E8 = 1000 ms)
    [2:248]         = 41 slots * 6 bytes each; slot index j at frame offset 2+6*j
                      (j = 0..40).  The servo ID equals the SLOT INDEX:
                      slot j <-> servo with ID j.  Slot 0 is unused (always the
                      sentinel).  Servo IDs 1..40 are supported.

      each 6-byte slot:
        [0:2]  position = uint16 LE.  Valid range 0..MAX_ANGLE (1000 for mode 0,
                          1500 for mode 3).  0x5555 (21845) = "servo NOT moved
                          in this frame" (parser skips it).
        [2:4]  dev_x    = int16 LE, servo deviation X (from the software's
                          deviation slider; normally 0)
        [4:6]  dev_y    = int16 LE, servo deviation Y (normally 0)

  File size == 16 + 248*count.  No footer / checksum.

  Time axis: every frame carries ONE duration (ms).  Playback = execute frames
  in order; frame k moves the listed servos to their positions within time[k] ms.

  Older variant ("Hiwonder Servo Control", pre-2023): magic b"ACTION",
  194-byte frames = time(2) + 32 slots*6, positions 500..2500.
"""

import struct
import sys


class ActFrame:
    __slots__ = ("time_ms", "servos")  # servos: dict {servo_id: (pos, dev_x, dev_y)}

    def __init__(self, time_ms, servos):
        self.time_ms = time_ms
        self.servos = servos

    def __repr__(self):
        return "ActFrame(time=%dms, servos=%r)" % (self.time_ms, self.servos)


class ActFile:
    def __init__(self, path, data, mode, count):
        self.path = path
        self.data = data
        self.mode = mode
        self.count = count
        self.frames = []


def parse_act40(path):
    """Parse an ACT-40 .rob file. Returns ActFile with .frames list."""
    with open(path, "rb") as f:
        data = f.read()

    if len(data) < 16 or data[0:6] != b"ACT-40":
        raise ValueError("Not an ACT-40 file (bad magic)")

    count = struct.unpack_from("<H", data, 6)[0]
    mode = data[12]
    if data[8:12] == b"EYPT":
        raise ValueError("Encrypted 'EYPT' action file - not supported by this parser")

    if data[12] == 3:
        max_angle = 1500
    else:
        max_angle = 1000

    if len(data) != 16 + 248 * count:
        raise ValueError("File size %d != expected 16 + 248*%d" % (len(data), count))

    act = ActFile(path, data, mode, count)

    for k in range(count):
        base = 16 + k * 248
        time_ms = struct.unpack_from("<H", data, base)[0]
        servos = {}
        for j in range(1, 41):  # slots 1..40 == servo IDs 1..40; slot 0 unused
            off = base + 2 + j * 6
            pos = struct.unpack_from("<H", data, off)[0]
            if pos <= max_angle:  # 0x5555 / out-of-range => servo not in frame
                dev_x = struct.unpack_from("<h", data, off + 2)[0]
                dev_y = struct.unpack_from("<h", data, off + 4)[0]
                servos[j] = (pos, dev_x, dev_y)
        act.frames.append(ActFrame(time_ms, servos))

    return act


def to_c_table(act, servo_ids=None):
    """Render frames as a C array of {time_ms, pos[19]} rows (like
    Single_action.c in the STM32 dance-robot project). Positions for servos not
    in a frame keep the previous frame's value (hold)."""
    servo_ids = servo_ids or sorted({sid for f in act.frames for sid in f.servos})
    lines = []
    prev = {sid: 500 for sid in servo_ids}  # 500 = centre / hold default
    for f in act.frames:
        for sid, (pos, dx, dy) in f.servos.items():
            if sid in prev:
                prev[sid] = pos
        row = "    {%d, {" % f.time_ms
        row += ", ".join(str(prev[sid]) for sid in servo_ids)
        row += "}},"
        lines.append(row)
    return "\n".join(lines)


def main():
    if len(sys.argv) < 2:
        print("usage: python act40_parser.py <file.rob>")
        return 1
    act = parse_act40(sys.argv[1])
    print("magic ACT-40  frames=%d  mode=%d (0=gen/0-1000, 3=gen/0-1500)" % (act.count, act.mode))
    for i, f in enumerate(act.frames):
        print("frame %2d: time=%4d ms  servos=%s" % (i, f.time_ms, f.servos))
    return 0


if __name__ == "__main__":
    sys.exit(main())
