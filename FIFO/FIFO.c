/**
 * @file    FIFO.c
 * @brief   FIFO队列操作函数实现
 */

#include "FIFO.h"

fifo_t fifo = {0};               // 全局 FIFO
uint8_t fifo_packet[10]={0};        //fifo缓冲区

/**
  * @brief  检查FIFO是否已满
  * @param  fifo: FIFO指针
  * @retval 1=已满, 0=未满
  */
static uint8_t Fifo_Is_Full(fifo_t *fifo)
{
    return (((fifo->head+1)%TX_FIFO_SIZE) == ((fifo->tail)%TX_FIFO_SIZE)) ? 1 : 0;
}

/**
  * @brief  检查FIFO是否为空
  * @param  fifo: FIFO指针
  * @retval 1=为空, 0=不为空
  */
static uint8_t Fifo_Is_Empty(fifo_t *fifo)
{
    return ((fifo->head % TX_FIFO_SIZE) == (fifo->tail % TX_FIFO_SIZE)) ? 1 : 0;
}

/**
  * @brief  向FIFO写入数据（变长）
  * @param  fifo: FIFO指针
  * @param  packet: 待写入的数据包
  * @param  len:   数据包实际长度（6或10字节）
  * @retval 1=写入成功, 0=写入失败
  */
uint8_t Fifo_Write(fifo_t *fifo, uint8_t *packet, uint8_t len)
{
    if(Fifo_Is_Full(fifo))
    {
        return 0;
    }
    else
    {
        memcpy(fifo->buf[fifo->head], packet, len);
        fifo->len[fifo->head] = len;          // 记录本包实际长度
        fifo->head = (fifo->head + 1) % TX_FIFO_SIZE;
        return 1;
    }
}

/**
  * @brief  从FIFO读取数据（变长）
  * @param  fifo:   FIFO指针
  * @param  packet: 用于存储读取数据的缓冲区
  * @param  len:    输出参数，返回本包实际长度
  * @retval 1=读取成功, 0=读取失败
  */
uint8_t Fifo_Read(fifo_t *fifo, uint8_t *packet, uint8_t *len)
{
    if(Fifo_Is_Empty(fifo))
    {
        return 0;
    }
    else 
    {
        uint8_t n = fifo->len[fifo->tail];
        memcpy(packet, fifo->buf[fifo->tail], n);
        *len = n;
        fifo->tail = (fifo->tail + 1) % TX_FIFO_SIZE;
        return 1;
    }
}
