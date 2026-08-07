[PDF Info] Total: 21 pages | Text: 21 | OCR: 0

## Page 1: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
User manual                                                              www.alientek.com 
  
ii 
ii
ATK-PMW3901 光流模块用户手册 
 
用户手册
超轻低功耗光流模块 
ATK-PMW3901 光流模块用户手册 
User 
Manual

## Page 2: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
User manual                                                              www.alientek.com 
  
ii 
ii
ATK-PMW3901 光流模块用户手册 
目  录 
1. 特性参数 ................................................................................................................... 1 
2. 使用说明 ................................................................................................................... 2 
2.1 模块硬件说明 ........................................................................................................................... 2 
2.2 模块和MiniFly 连接示意图 .................................................................................................... 4 
2.3 PMW3901 光流传感器和MiniFly 通信 .................................................................................. 4 
2.4 VL53LXX 激光传感器和MiniFly 通信 ................................................................................ 11 
2.4.1. VL53LXX 初始化 ....................................................................................................... 12 
2.4.2. VL53L0X 任务 ............................................................................................................ 13 
2.4.3. VL53L1X 任务 ............................................................................................................ 15 
2.4.4. VL53LXX 读取数据 ................................................................................................... 17 
3. 其他 ......................................................................................................................... 19 
3.1 购买地址 ................................................................................................................................. 19 
3.2 资料下载 ................................................................................................................................. 19 
3.3 技术支持 ................................................................................................................................. 19

## Page 3: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 1 / 21 
 
ATK-PMW3901 光流模块用户手册 
1. 特性参数 
ATK-PMW3901 是ALIENTEK 推出的一款超轻多功能低功耗光流模块（以下简称光流
模块），此模块集成一个高精度低功耗光学追踪传感器PMW3901 和一个高精度激光传感器
VL53LXX（2m 版本使用VL53L0X, 4m 版本使用VL53L1X，以下统称VL53LXX）, PMW3901
光流传感器负责测量水平移动，VL53LXX 激光传感器负责测量距离。4m 版本光流模块是
2m 版本光流的升级版本，测量距离更远，抗干扰能力更强。MiniFly 搭配概模块即可实现稳
定悬停（遥控器设置为定点模式）。 
 
ATK-PMW3901 光流模块各项参数如表1.1 所示： 
ATK-PMW3901 
接口特性 
3.3V~4.2VDC @20mA 
通信接口 
PMW3901 传感器: SPI2 @2Mhz 
VL53LXX 传感器: 模拟IIC@400Khz(max) 
测量范围 
PMW3901 传感器: > 8cm (建议400cm 以内使用) 
VL53L0X 传感器: 3cm~200cm（30Hz） 
VL53L1X 传感器: 4cm~400cm（30Hz） 
输出速率 
PMW3901 传感器: > 100Hz(>60Lux) 
VL53L0X 传感器: 30Hz (长距离模式，室内) 
VL53L1X 传感器: 30Hz (长距离模式，室内) 
尺寸 
长x 宽：27.5mm x 16.5mm 
重量 
重量1.6g 
表1.1 ATK-PMW3901 光流模块参数 
 
此模块搭配MiniFly 使用时请注意以下事项： 
1) 请先将MiniFly 以及遥控器固件升级到固件V1.3 及以上版本，固件版本升级请参考
“ATK-MiniFly 微型四轴固件说明_V1.3.pdf”，升级完成之后，遥控器设置控制模
式为定点模式； 
2) 四轴固件1.3 及以上版本支持用户打开/关闭激光传感器功能，用户可在遥控器端开
启/关闭VL53LXX（遥控器固件先升级到V1.3 或者以上版本）。 
3) 搭配该模块定点时，会用到激光传感器测距，测距模式为长距离模式，可以测量
2m/4m，但是长距离测量模式要求在黑暗且无红外光的环境，在室外强光下，激光
传感器会受到很大干扰，测量精度急剧降低，所以室外飞行建议使用气压定高。 
4) 激光测量的距离为测量参考平面到激光传感器的距离，MiniFly 在定点或者定高飞
行且激光可测范围内，MiniFly 会根据激光测量高度实时调整自身高度，比如改变
激光测量的参考平面，拿一个障碍物放到模块下面，那么MiniFly 会自动升高到以
新的参考平面为基准的设定高度，但是不建议这样做，因为这样同时会影响到光流
的数据测量。 
5) 光流传感器需要一定的光照条件（>60Lux）,光线过低会影响定点效果； 
6) 1.2 及以上版本的固件，扩展模块支持热插拔，也就是四轴开机之后再插上扩展模
块就可以直接使用，不用重启四轴。

## Page 4: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 2 / 21 
 
ATK-PMW3901 光流模块用户手册 
2. 使用说明 
2.1 模块硬件说明 
 
ATK-PMW3901 光流模块实物图如图2.1.1（2m 版本）和图2.1.2（4m 版本）所示： 
 
 
图2.1.1  2m 版本光流模块实物图 
 
 
 
图2.1.2  4m 版本光流模块实物图 
现在改进了光流镜头，用的是图2.1.2 的镜头，新的镜头无需盖片，而之前的光流模块，
我们有在光流模组上增加一个盖片，防止光学镜片进入灰尘，影响传感器精度，如图2.1.3
所示： 
 
图2.1.3 光流加盖片效果 
注意： 
现在使用的光流模块，使用新的镜头，不再使用盖片，如果用户使用的是早期的光流镜
头 ，盖片上还有一层保护膜，使用之间请撕掉这个保护膜，且不能刮花保护盖片。 
 
光流模块电路图如下图2.1.4 所示。

## Page 5: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 3 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
图2.1.4 ATK-PMW3901 光流模块电路图 
可以看到，光流模块原理图还是比较简单的，一颗光流模组PMW3901,一颗激光测距传
感器VL53LXX, 2 颗LDO, 一组2.0 间距排母，一个ADC 采集点，以及电阻、电容、二三
极管和MOS 管。 
PMW3901 是PixArt 公司最新的高精度低功耗光学追踪模组，可直接获取xy 方向运动
信息，测量范围8cm 以上，工作电流< 9mA ，工作电压VDD(1.8~2.1VDC) 和
VDDIO(1.8~3.6VDC)，使用4 线SPI 接口通信。原理图可以看出，2 颗LDO，一颗1.8V 的
LDO 提供给VDD，一颗3.0V 的LDO 提供给VDDIO。SPI 接口使用硬件SPI2，片选信号
PA8，另外LED1 为红外LED，默认没有焊接。 
VL53L0X 是ST 公司的第二代激光测距传感器，该芯片集成了激光发射器和SPAD 红外
接收器，采用FlightSense™技术，通过接收器接收到的光子时间来计算距离，实现更快、更
远、更精确的测距功能，该传感器工作电压AVDD 由3.0V 的LDO 经过一个10R 电阻得到，
大概是2.8V，也恰好是VL53LXX 工作最佳电压，该传感器测量距离3cm~200cm，使用IIC
通信接口，最大通信速率400K，我们使用模拟IIC（SDA/PB4,SCL/PB5）和飞控端通信。 
而VL53L1X 是ST 公司的第三代激光测距传感器，除了拥有第二代传感器的技术外，
还集成了物理红外滤波器和光学元件，无论目标颜色和反射率如何，都可以进行距离测量，
抗干扰能力更强，VL53L1X 和VL53L0X Pin-to-pin 兼容，VL53L1X 是VL53L0X 的升级版，
测量距离更远（4~400cm）,测量速度更快（短距离20ms，长距离33ms）,抗干扰更强。 
此模块同ATK-LED-RING 灯环模块，ATK-WIFI-MODULE 摄像头模块一样，有一个
ADC 采集脚，根据不同的ADC 电压值来识别不同的模块ID，然后飞控根据模块ID 来控制
模块，光流模块的ADC 值为2.06V，ADC 的值由电阻R6 和R7 分压得到。模块的电源通过
MOS1 控制通断，控制信号为E_SCL/PB0，高电平时，MOS 管导通。 
另外可以看到电感L1 和L2，通过L1 和L2 来选择VDDIO，如果使用L1，那么VDDIO
电源来自于飞控VCC3.0，如果使用L2，那么VDDIO 电源来自光流模块3.0V LDO IC2，默
认焊接L2，这样可以得到更稳定的电源。

## Page 6: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 4 / 21 
 
ATK-PMW3901 光流模块用户手册 
2.2 模块和MiniFly 连接示意图 
 
模块自带2x8 排母，用来对接MiniFly 的2x8 排针，但是要注意模块是有方向的，我们
需要按照模块箭头和四轴机头箭头保持一致的方式安装模块，安装效果如图2.2.1 所示： 
 
图2.2.1 模块装机图 
2.3 PMW3901 光流传感器和MiniFly 通信 
 
PMW3901 光流传感器和MiniFly 之间采用SPI 的通信方式，使用硬件SPI2，光流传感
器和飞控通信源码主要分2 个部分，底层SPI 驱动和光流数据通信。 
 
底层SPI 驱动配置比较简单，2 线全双工，2M 波特率，主机模式，这部分就不贴代码
了，下去看spi.c 即可。 
然后SPI 的收发都使用DMA 的方式，接收DMA 使用DMA1 数据流3，发送DMA 使
用DMA1 数据流4，详细内容请看源码spi.c。需要注意的是，SPI 的DMA 发送中断和灯环
的中断共用DMA1 数据流4 中断，所以我们需要在DMA1 数据流4 中断里面进行分开处理，
处理方式如下： 
static enum expModuleID lastModuleID = NO_MODULE; 
void  DMA1_Stream4_IRQHandler(void) 
{ 
 
if(getModuleID() == LED_RING) 
 
{ 
 
 
lastModuleID = LED_RING; 
 
 
ws2812DmaIsr(); 
 
} 
 
 
 
else if(getModuleID() == OPTICAL_FLOW)

## Page 7: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 5 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
{ 
 
 
lastModuleID = OPTICAL_FLOW; 
 
 
spiTxDmaIsr(); 
 
} 
 
else if(getModuleID() == NO_MODULE) 
 
{ 
 
 
if(lastModuleID == LED_RING) 
 
 
{ 
 
 
 
 
ws2812DmaIsr();  
 
 
 
 
DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, DISABLE);  
 
 
 
 
 
DMA_Cmd(DMA1_Stream4,DISABLE); 
 
 
 
 
 
 
 
 
} 
 
 
 
 
else if(lastModuleID == OPTICAL_FLOW) 
 
 
{ 
 
 
 
spiTxDmaIsr(); 
 
 
 
DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, DISABLE); 
 
 
 
DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, DISABLE); 
 
 
 
DMA_Cmd(DMA1_Stream3,DISABLE); 
 
 
 
DMA_Cmd(DMA1_Stream4,DISABLE); 
 
 
 
 
 
} 
 
 
 
 
 
} 
} 
可以看到，中断里面，我们根据模块的ID 来分开处理，getModuleID()就是读取当前模
块的ID，现在MiniFly 支持ATK-LED-RING 灯环模块，ATK-WIFI-MODULE 摄像头模块以
及ATK-PMW3901 光流模块，可以看到中断里多了对未检测到模块（NO_MODULE）做处
理的内容，这部分内容的作用就是，保证我们在拔掉模块后，可以继续进入中断，完成拔掉
模块之前没有执行完的中断内容。 
然后再来说光流数据通信，通信相关内容在optical_flow.c 里面，主要包括光流电源控
制函数opticalFlowPowerControl，读写寄存器函数registerRead，registerWrite，突发读取12
字节运动数据函数readMotion ，寄存器初始化函数InitRegisters ，光流任务函数
opticalFlowTask，读取光流数据函数getFlowData，以及光流初始化函数opticalFlowInit，我
们主要讲解下几个比较重要的函数。  
光流初始化函数，源码如下： 
void opticalFlowInit(void) 
{ 
 
if (!isInit) /*第一次初始化通用IO*/ 
 
{ 
 
 
GPIO_InitTypeDef GPIO_InitStructure; 
 
 
 
//初始化CS 引脚  
 
 
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能时钟 
 
 
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能时钟 
 
 
 
 
 
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;

## Page 8: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 6 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
 
 
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
 
 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;  
 
 
GPIO_Init(GPIOA, &GPIO_InitStructure);  
 
 
 
 
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
 
 
GPIO_Init(GPIOB, &GPIO_InitStructure);  
 
 
} 
 
else  
 
{ 
 
 
resetOpFlowData(); 
 
 
opFlow.isOpFlowOk = true;  
 
 
 
 
} 
 
 
 
opticalFlowPowerControl(true); 
/*打开电源*/ 
 
vTaskDelay(50); 
 
 
 
NCS_PIN = 1; 
 
spi2Init(); 
 
vTaskDelay(40); 
 
 
uint8_t chipId = registerRead(0); 
 
uint8_t invChipId = registerRead(0x5f); 
// 
printf("Motion chip is: 0x%x\n", chipId); 
// 
printf("si pihc noitoM: 0x%x\n", invChipId); 
 
 
// 上电复位 
 
registerWrite(0x3a, 0x5a); 
 
vTaskDelay(5); 
 
 
InitRegisters(); 
 
vTaskDelay(5); 
 
 
 
if (isInit)  
 
{ 
 
 
vTaskResume(opFlowTaskHandle); 
/*恢复光流任务*/ 
 
} 
 
else if(opFlowTaskHandle == NULL) 
 
{ 
 
 
xTaskCreate(opticalFlowTask, 
"OPTICAL_FLOW", 
300, 
NULL, 
4, 
&opFlowTaskHandle); /*创建光流模块任务*/ 
 
} 
 
 
 
 
vl53lxxInit(); /*初始化vl53lxx*/

## Page 9: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 7 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
 
isInit = true; 
} 
 
isInit 是个静态bool 变量，用于判断是否是第一次使用光流模块，如果是第一次使用
（isInit=false）则初始化相关GPIO 引脚，然后打开光流电源，初始化SPI2 接口，初始化光
流寄存器，然后再次判断是否第一次使用光流模块，如果第一次使用，则创建光流通信任务，
如果之前已经创建了光流通信任务，则直接将挂起的任务恢复即可，最后初始化激光传感器
VL53LXX(vl53l0x 或者vl53l1x)，关于激光传感器的控制，我在2.4 章节给大家说明。这样
光流初始化就完成了。 
 
接着我们看下光流通信任务，源码如下： 
void opticalFlowTask(void *param) 
{ 
 
 
static u16 count = 0; 
 
 
u32 lastWakeTime = getSysTickCnt(); 
 
 
 
 
opFlow.isOpFlowOk = true; 
 
 
 
while(1)  
 
{ 
 
 
vTaskDelayUntil(&lastWakeTime, 10); 
/*100Hz 10ms 周期延时*/ 
 
 
 
 
 
readMotion(&currentMotion); 
 
 
 
if(currentMotion.minRawData == 0 && currentMotion.maxRawData == 0) 
 
 
{ 
 
 
 
if(count++ > 100 && opFlow.isOpFlowOk == true) 
 
 
 
{ 
 
 
 
 
count = 0; 
 
 
 
 
opFlow.isOpFlowOk = false;  
/*光流出错*/ 
 
 
 
 
vTaskSuspend(opFlowTaskHandle); 
/*挂起光流任务*/ 
 
 
 
} 
 
 
 
 
}else 
 
 
{ 
 
 
 
count = 0; 
 
 
} 
 
 
/*连续2 帧之间的像素变化，根据实际安装方向调整 (pitch:x)  (roll:y)*/ 
 
 
int16_t pixelDx = currentMotion.deltaY; 
 
 
int16_t pixelDy = -currentMotion.deltaX; 
 
 
 
if (ABS(pixelDx) < OULIER_LIMIT && ABS(pixelDy) < OULIER_LIMIT)  
 
 
{ 
 
 
 
opFlow.pixSum[X] += pixelDx; 
 
 
 
opFlow.pixSum[Y] += pixelDy; 
 
 
}else

## Page 10: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 8 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
{ 
 
 
 
outlierCount++; 
 
 
} 
 
} 
} 
 
先说一下opFlow 光流结构体，定义如下： 
typedef struct opFlow_s  
{ 
 
float pixSum[2]; 
 
/*累积像素*/ 
 
float pixComp[2];  
/*像素补偿*/ 
 
float pixValid[2];  
/*有效像素*/ 
 
float pixValidLast[2]; 
/*上一次有效像素*/ 
 
 
 
float deltaPos[2]; 
 
/*2 帧之间的位移 单位cm*/ 
 
float deltaVel[2]; 
 
/*速度 单位cm/s*/ 
 
float posSum[2]; 
 
/*累积位移 单位cm*/ 
 
float velLpf[2]; 
 
/*速度低通 单位cm/s*/ 
 
 
 
bool isOpFlowOk;  
/*光流状态*/ 
 
bool isDataValid;  
/*数据有效*/ 
 
} opFlow_t; 
看注释大家也应该大概明白各变量的含义了，我再简单说明一下： 
累积像素，就是自四轴起飞后的累积像素； 
像素补偿，就是补偿由于飞机倾斜导致的像素误差； 
有效像素，指经过补偿的实际像素； 
2 帧之间的位移，这个就是由像素转换出来的实际位移，单位cm； 
速度，这个速度是瞬时速度，由位移变化量微分得到，单位cm/s； 
累积位移，实际位移，单位cm； 
速度低通，对速度进行低通，增加数据平滑性； 
光流状态，光流是否正常工作； 
数据有效，在一定高度范围内，数据有效； 
 
接着看光流任务内容，可以看到，我们设定光流传感器更新速率100Hz，然后就是使用
突发模式一次性读取多字节运动数据了，这个运动数据是1 个由12 个字节组成的结构体，
此结构体定义如下： 
typedef __packed struct motionBurst_s  
{ 
 
__packed union  
 
{ 
 
 
uint8_t motion; 
 
 
__packed struct  
 
 
{ 
 
 
 
uint8_t frameFrom0    : 1;

## Page 11: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 9 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
 
uint8_t runMode       : 2; 
 
 
 
uint8_t reserved1     : 1; 
 
 
 
uint8_t rawFrom0      : 1; 
 
 
 
uint8_t reserved2     : 2; 
 
 
 
uint8_t motionOccured : 1; 
 
 
}; 
 
}; 
 
 
uint8_t observation; 
 
int16_t deltaX; 
 
int16_t deltaY; 
 
 
uint8_t squal; 
 
 
uint8_t rawDataSum; 
 
uint8_t maxRawData; 
 
uint8_t minRawData; 
 
 
uint16_t shutter; 
} motionBurst_t; 
简单说明下结构体内容，motion 指的是运动信息，同时又是一个联合体，这样就可以
根据不同的位去判断运动信息，包括帧判别，运行模式和运动信息检测等，这个我们暂时不
需要用到；然后就是observation，这个是用于检测IC 是否出现EFT/B 或者ESD 问题，传感
器正常工作时，读取出来的值为0xBF；接着就是我们要用到的int16_t类型的运动信息deltaX, 
deltaY 了，这个就是光流检测到图像的X 和Y 方向的运动信息；然后是squal 是指运动信息
质量，简单说就是运动信息的可信度；rawDataSum 这个是原数据求和，可用作对一帧数据
求平均值；至于maxRawData 和minRawData ，都知道是最大和最小原始数据了；然后shutter
是一个实时自动调整的值，目的是保证平均运动数据在正常可操作范围以内，这个值可以搭
配squal，用来判断运动信息是否可用。 
接着说光流任务里的内容，100Hz 突发模式读取12 字节数据，然后对读取的数据进行
处理，首先是判断最大最小原始数据，如果连续1s 时间都为0，说明光流出故障了，因为
正常工作的时候最大最小原始值不为0，如果出故障了，我们就挂起光流任务；光流正常，
我们就继续分析运动数据，先读出x，y 方向的像素变化，读取的时候需要根据光流的安装
方向做相应的调整，调整方式参考光流手册28 页，上面有描述x，y 的运动正方向，MiniFly
设置pitch 方向为x，roll 方向为y，需要注意的是：这个运动正方向是光流传感器检测到图
像的运动，当我们安装到MiniFly 之后，以地面为参考，地面是不动的 ，MiniFly 在运动，
那么检测到的图像运动是相反的，比如MiniFly 往前走，光流检测的图像是往后的，因为参
考点不同。 
 
读取运动数据之后，我们加了限幅判断，目的是增加数据安全性，接着将原始像素数据
求和并低通；如果数据不安全，则使用变量outlierCount 累加。这些就是光流任务内容了。 
 
我们接着看一下光流数据读取，这儿比较重要了，源码如下： 
bool getOpFlowData(state_t *state, float dt) 
{ 
 
static u8 cnt = 0;

## Page 12: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 10 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
float height = 0.01f * getFusedHeight();/*读取高度信息 单位m*/ 
 
 
 
if(opFlow.isOpFlowOk && height<4.0f) 
/*4m 范围内，光流可用*/ 
 
{ 
 
 
cnt= 0; 
 
 
opFlow.isDataValid = true; 
 
 
 
 
 
float coeff = RESOLUTION * height; 
 
 
float tanRoll = tanf(state->attitude.roll * DEG2RAD); 
 
 
float tanPitch = tanf(state->attitude.pitch * DEG2RAD); 
 
 
 
 
 
opFlow.pixComp[X] = 480.f * tanPitch; 
/*像素补偿，负方向*/ 
 
 
opFlow.pixComp[Y] = 480.f * tanRoll; 
 
 
opFlow.pixValid[X] = (opFlow.pixSum[X] + opFlow.pixComp[X]); /* 实际输出
像素*/ 
 
 
opFlow.pixValid[Y] = (opFlow.pixSum[Y] + opFlow.pixComp[Y]);  
 
 
 
 
 
 
if(height < 0.05f) 
/*光流测量范围大于5cm*/ 
 
 
{ 
 
 
 
coeff = 0.0f; 
 
 
} 
 
 
opFlow.deltaPos[X] = coeff * (opFlow.pixValid[X] - opFlow.pixValidLast[X]);
 
/*2 帧之间位移变化量，单位cm*/ 
 
 
opFlow.deltaPos[Y] = coeff * (opFlow.pixValid[Y] - opFlow.pixValidLast[Y]);
 
 
 
 
opFlow.pixValidLast[X] = opFlow.pixValid[X]; /*上一次实际输出像素*/ 
 
 
opFlow.pixValidLast[Y] = opFlow.pixValid[Y]; 
 
 
opFlow.deltaVel[X] = opFlow.deltaPos[X] / dt; /*速度 cm/s*/ 
 
 
opFlow.deltaVel[Y] = opFlow.deltaPos[Y] / dt; 
 
 
 
 
 
opFlow.velLpf[X] += (opFlow.deltaVel[X] - opFlow.velLpf[X]) * 0.15f; /* 速度
低通 cm/s*/ 
 
 
opFlow.velLpf[Y] += (opFlow.deltaVel[Y] - opFlow.velLpf[Y]) * 0.15f; /* 速度
低通 cm/s*/  
 
 
 
opFlow.velLpf[X] = constrainf(opFlow.velLpf[X], -VEL_LIMIT, VEL_LIMIT);
 
/*速度限幅 cm/s*/ 
 
 
opFlow.velLpf[Y] = constrainf(opFlow.velLpf[Y], -VEL_LIMIT, VEL_LIMIT);
 
/*速度限幅 cm/s*/ 
 
 
 
 
opFlow.posSum[X] += opFlow.deltaPos[X]; /*累积位移 cm*/ 
 
 
opFlow.posSum[Y] += opFlow.deltaPos[Y]; /*累积位移 cm*/ 
 
} 
 
else if(opFlow.isDataValid == true) 
 
{

## Page 13: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 11 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
if(cnt++ > 100) 
/*超过定点高度，切换为定高模式*/ 
 
 
{ 
 
 
 
cnt = 0; 
 
 
 
opFlow.isDataValid = false; 
 
 
} 
 
 
 
resetOpFlowData(); 
 
} 
 
return opFlow.isOpFlowOk; /*返回光流状态*/ 
} 
 
首先读取高度信息，下面会用的，这个高度信息是由激光vl53lxx 测量高度和气压高度
融合得到（激光测高，我在2.4 章节说明）。接着判断光流工作状态以及高度信息范围，说
一下这个融合的高度信息，因为光流传感器也有精度，一定程度上和高度成反比，所以高度
达到一定程度，精度就不怎么好了，我们暂时设定可用范围4m，然后定义变量系数，这个
变量系数用做转换像素信息为实际位移信息，该系数由高度和分辨率RESOLUTION 相乘得
到，RESOLUTION 是怎么来的呢，它是一个宏，定义如下： 
#define RESOLUTION   (0.2131946f) 
/*1m 高度下 1 个像素对应的位移，单位cm*/ 
它的意思就是1m 高度下，一个像素对应的位移，单位cm，这个分辨率怎么来的呢，光流
手册42 页有个光流CPI 和高度的关系图表，根据CPI 和高度信息，计算得到这个分辨率。
因此我们得到高度信息、分辨率以及2 帧之间的像素变化之后，我们就可以算出2 帧之间的
实际位移了。 
 
接着是倾角补偿计算，倾角补偿就的目的是去除由于机体倾斜导致的像素误差，根据
roll，pitch，计算像素补偿，可以看到有个系数480，这个480 如何得到呢，当然是实际测
试得到的，测试过程是：保证在某一高度下，调整这个值，使得我们原地前后左右晃动四轴，
光流输出数据基本保持不变，那么这个值就是我们需要的，因为四轴在原地，水平方向不应
当有位移变化。 
 
有了倾角补偿和运动累积像素，我们就可以得到实际累积像素，减去上次的实际像素，
就可以得到2 帧之间的变化像素，再乘以系数就可以得到2 帧之间的位移变化，可以看到还
有对系数的限制，当高度小于5cm，光流就无法工作了，所以系数设置为0。接着对这个位
移积分得到四轴到起飞点的位移，对这个位移微分得到瞬时速度，对速度进行低通增加数据
的平滑性，对速度进行限幅处理，增加数据安全性。 
我们通过光流就得到了四轴的位置信息和速度信息，把这些位置信息和速度信息融合加
速计（state_estimator.c），得到估测位置和速度，将估测位置和速度参与PID 运算，即可用
于水平方向位置控制，这部分内容请看position_pid.c，源码里面可以直接看到位置环和速度
环PID 的处理过程，这样就可以实现水平定点控制了。 
 
后面是对异常数据做处理，比如高度过高（>400cm）一段时间之后，设定光流数据不
可用，也就是设定opFlow.isDataValid 为false，然后清零光流数据。另外在commander.c 里
面，实时读取opFlow.isDataValid 的状态，根据opFlow.isDataValid 的状态设置是否使用定点
模式。 
2.4 VL53LXX 激光传感器和MiniFly 通信 
 
VL53LXX（包括VL53L0X 和VL53L1X），我们把底层驱动函数分别写到vl53l0x.c 和
vl53l1x.c, 任务函数写到vl53lxx.c。 
激光传感器和MiniFly 之间采用模拟IIC 的通信方式，激光传感器和飞控通信源码主要
分2 个部分，底层IIC 驱动部分和激光测距应用部分。

## Page 14: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 12 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
底层IIC 驱动配置比较简单，使用2 个通用IO（PB4，PB5）来模拟IIC，代码比较简
单，就不贴出来了，下去看vl53l0x_i2c.c 即可。 
 
VL53L0X 底层驱动代码，这部分内容在vl53l0x.c 里面，这部分内容基本移植于
crazyflie2.0，这一个源文件已经集成各种使用该传感器需要用到的函数，ST 官方虽然没有
提供该传感器的寄存器手册，但是提供了传感器的软件API（应用编程接口）以及完整的文
档和例程源码，例程源码是ST 官方的X-NUCLEO-53L0A1 扩展板基于STM32F401RE 和
STM32L476RG Nucleo 开发板进行开发的，也是需要移植一下才可以使用，如果大家感兴趣，
可以参考这个文档：VL53L0XAPIv1.0.2.4823externalx.chm，我们也有单独的VL53L0X 激光
测距模块ATK-VL53L0X，就是通过调用官方的各种API 函数来使用该传感器，资料也是开
源的，可自行下载参考。这儿我们就移植别人写好的驱动，一个.c 文件搞定，使用起来相对
简单，VL53L0X 的寄存器配置特别多，就不贴源码了，可以自己下去看vl53l0x.c。 
VL53L1X 底层驱动代码，这部分内容移植于ST 官方库，像VL53L0X 一样，ST 官方
提供了VL53L1X 的，软件API（应用编程接口）以及完整的文档和例程源码，例程源码是
ST 官方的X-NUCLEO-53L1A1 扩展板基于STM32F401RE 和STM32L476RG Nucleo 开发板
进行开发的，这部分API 函数在源码文件夹VL53L1X 里面，然后平台接口函数vl53l1x，
用于连接上层API 和MCU 的IIC，对API 函数感兴趣的用户，可以参考这2 个文档：
VL53L1X_API.chm 以及VL53L1X API User Manual (UM2356).pdf。 
2.4.1. VL53LXX 初始化 
 
VL53LXX 初始化如下： 
void vl53lxxInit(void) 
{ 
 
vl53IICInit();  
 
delay_ms(10); 
 
 
 
/*vl53l0x 初始化*/ 
 
vl53lxxId = vl53l0xGetModelID(); 
 
if(vl53lxxId == VL53L0X_ID) 
 
{ 
 
 
if (isInitvl53l0x) 
 
 
{ 
 
 
 
reInitvl53l0x = true; 
 
 
 
vTaskResume(vl53l0xTaskHandle); 
/*恢复激光测距任务*/ 
 
 
} 
 
 
else /*首次接上vl53l0x 光流模块*/ 
 
 
{ 
 
 
 
 
isInitvl53l0x = true; 
 
 
 
xTaskCreate(vl53l0xTask, "VL5310X", 300, NULL, 5, &vl53l0xTaskHandle);
 
/*创建激光测距模块任务*/ 
 
 
} 
 
 
return; 
 
} 
 
 
 
 
delay_ms(10);

## Page 15: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 13 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
/*vl53l1x 初始化*/ 
 
VL53L1_RdWord(&dev, 0x010F, &vl53lxxId); 
 
if(vl53lxxId == VL53L1X_ID) 
 
{ 
 
 
if (isInitvl53l1x) 
 
 
{ 
 
 
 
reInitvl53l1x = true; 
 
 
 
vTaskResume(vl53l1xTaskHandle); 
/*恢复激光测距任务*/ 
 
 
} 
 
 
else /*首次接上vl53l1x 光流模块*/ 
 
 
{ 
 
 
 
 
 
isInitvl53l1x = true; 
 
 
 
 
 
 
xTaskCreate(vl53l1xTask, "VL53L1X", 300, NULL, 5, &vl53l1xTaskHandle);
 
/*创建激光测距模块任务*/ 
 
 
} 
 
 
return; 
 
} 
 
 
 
 
vl53lxxId = 0; 
} 
首先初始化模拟IIC，然后先尝试读取vl53l0x 的ID，因为vl53l0x 和vl53l1x 的ID 不
同，我们就可以根据ID 来初始化对应的传感器。然后根据reInitvl53l1x / reInitvl53l1x 的值
判断是否第一次使用VL53L0X / VL53L1X，如果第一次使用，则创建激光测距任务
vl53l0xTask / vl53l1xTask，否则，恢复之前之间已经创建且挂起的测距任务。 
2.4.2. VL53L0X 任务 
 
vl53l0x 任务函数如下： 
void vl53l0xTask(void* arg) 
{ 
 
TickType_t xLastWakeTime = xTaskGetTickCount(); 
 
 
 
vl53l0xSetParam(); /*设置vl53l0x 参数*/ 
 
 
 
 
while (1)  
 
{ 
 
 
if(reInitvl53l0x == true) 
 
 
{ 
 
 
 
count = 0; 
 
 
 
reInitvl53l0x = false; 
 
 
 
 
 
 
vl53l0xSetParam(); /*设置vl53l0x 参数*/ 
 
 
 
xLastWakeTime = xTaskGetTickCount(); 
 
 
 
 
 
 
}else 
 
 
{

## Page 16: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 14 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
 
range_last = vl53l0xReadRangeContinuousMillimeters() * 0.1f; //单位cm 
 
 
 
 
if(range_last < VL53L0X_MAX_RANGE)  
 
 
 
 
 
 
validCnt++; 
 
 
 
 
 
 
else  
 
 
 
 
 
 
 
inValidCnt++;  
 
 
 
 
 
 
 
 
 
if(inValidCnt + validCnt == 10) 
 
 
 
{ 
 
 
 
 
quality += (validCnt/10.f - quality) * 0.1f; 
/*低通*/ 
 
 
 
 
validCnt = 0; 
 
 
 
 
inValidCnt = 0; 
 
 
 
} 
 
 
 
 
 
 
 
if(range_last >= 6550) /*vl53 错误*/ 
 
 
 
{ 
 
 
 
 
if(++count > 30) 
 
 
 
 
{ 
 
 
 
 
 
count = 0; 
 
 
 
 
 
vTaskSuspend(vl53l0xTaskHandle); 
/*挂起激光测距任务*/ 
 
 
 
 
 
 
 
 
 
} 
 
 
 
 
 
 
 
}else count = 0; 
 
 
 
 
 
 
 
 
 
 
vTaskDelayUntil(&xLastWakeTime, measurement_timing_budget_ms); 
 
 
} 
 
 
 
} 
} 
先是定义一些变量，其中变量validCnt 和inValidCnt 分别用于统计激光数据是否可用，
具体如何统计，我待会儿在下面会给大家说明。然后就开始设置测量参数vl53l0xSetParam，
设置详细内容如下： 
void vl53l0xSetParam(void) /*设置vl53l0x 参数*/ 
{ 
 
vl53l0xTest();  
 
 
vl53l0xSetVcselPulsePeriod(VcselPeriodPreRange, 18); 
/*长距离模式33ms 周期*/ 
 
vl53l0xSetVcselPulsePeriod(VcselPeriodFinalRange, 14); /*长距离模式33ms 周期*/ 
 
vl53l0xStartContinuous(0); 
} 
这里主要就是设置测量模式，包括设定VCSEL 脉冲周期（VcselPeriodPreRange）和设
定设定VCSEL 脉冲周期范围（VcselPeriodFinalRange）；该传感器支持4 种模式，当这2
个参数分别设置为18 和14，最大测量周期33ms，就表示长距离测量模式，测量范围可达
2m。需要注意的是，此模式限制在黑暗且无红外光条件下使用，在户外强光下测量精度会
急剧下降，此模块使用就是这种模式，所以激光测距最好不要在户外使用； 
当这2 个参数分别设置为14 和10 则有可能表示默认模式、高精度模式，或者高速模式，

## Page 17: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 15 / 21 
 
ATK-PMW3901 光流模块用户手册 
这几种模式测量范围都只有1.2m，如何区分这几种模式呢，当然是根据设置传感器最大测
量周期来区分这几种模式，当最大测量周期设置为20ms，则表示高速模式，测量误差±5%；
最大测量周期设置为30ms，表示默认模式，测量精度±4%；最大测量周期设置为200ms，
表示高精度模式，此模式测量精度高，测量误差小于±3%，但耗时多。 
接着设置采集模式为连续模式，也就是以最快的速率读取数据。 
然后进入while()循环，先判断reInitvl53l0x 这个变量，如果为真，表示再次使用激光测
距，那么我们需要设置一下测量模式，接着才是以30Hz 读取激光数据，接着对激光数据分
析，先判断激光数据是否有效，判断依据测量值是否小于最大值VL53L0X_MAX_RANGE，
这是一个宏，在vl53l0x.h 中定义，宏的内容为220，表示最大测量距离220cm，如果测量数
据有效变量validCnt 加1，否则无效变量inValidCnt 加1，当有效无效变量相加等于10，我
们进行一次数据可信度计算，计算很简单，将数据有效次数除以总次数，然后低通处理，这
样我们得到激光数据可信度quality，范围0~1.0；可以看到后边还有一次对测量数据的判断，
这个主要用于识别vl53l0x 是否掉线，当vl53l0x 掉线（比如拔掉光流模块）之后，这个测
量数据一直大于6550，此时，我们将激光测距任务挂起，直到激光传感器恢复正常。 
 
2.4.3. VL53L1X 任务 
 
vl53l0x 任务函数如下： 
void vl53l1xTask(void* arg) 
{ 
 
int status; 
 
u8 isDataReady = 0; 
 
TickType_t xLastWakeTime = xTaskGetTickCount();; 
 
static VL53L1_RangingMeasurementData_t rangingData; 
 
 
vl53l1xSetParam(); /*设置vl53l1x 参数*/ 
 
 
 
while(1)  
 
{ 
 
 
if(reInitvl53l1x == true) 
 
 
{ 
 
 
 
count = 0; 
 
 
 
reInitvl53l1x = false; 
 
 
 
 
 
 
vl53l1xSetParam(); /*设置vl53l1x 参数*/ 
 
 
 
xLastWakeTime = xTaskGetTickCount(); 
 
 
}else 
 
 
{ 
 
 
 
 
status = VL53L1_GetMeasurementDataReady(&dev, &isDataReady); 
 
 
 
 
 
 
 
 
 
 
if(isDataReady) 
 
 
 
{ 
 
 
 
 
status = VL53L1_GetRangingMeasurementData(&dev, &rangingData); 
 
 
 
 
if(status==0) 
 
 
 
 
{

## Page 18: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 16 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
 
 
 
range_last = rangingData.RangeMilliMeter * 0.1f; 
/*单位cm*/ 
 
 
 
 
} 
 
 
 
 
status = VL53L1_ClearInterruptAndStartMeasurement(&dev); 
 
 
 
} 
 
 
 
 
 
 
 
 
if(range_last < VL53L1X_MAX_RANGE)  
 
 
 
 
 
 
validCnt++; 
 
 
 
 
 
 
else  
 
 
 
 
 
 
 
inValidCnt++;  
 
 
 
 
 
 
 
 
 
if(inValidCnt + validCnt == 10) 
 
 
 
{ 
 
 
 
 
quality += (validCnt/10.f - quality) * 0.1f; 
/*低通*/ 
 
 
 
 
validCnt = 0; 
 
 
 
 
inValidCnt = 0; 
 
 
 
} 
 
 
 
 
 
 
 
if(getModuleID() != OPTICAL_FLOW) 
 
 
 
{ 
 
 
 
 
if(++count > 10) 
 
 
 
 
{ 
 
 
 
 
 
count = 0; 
 
 
 
 
 
VL53L1_StopMeasurement(&dev); 
 
 
 
 
 
vTaskSuspend(vl53l1xTaskHandle); 
/*挂起激光测距任务*/ 
 
 
 
 
 
 
 
 
 
} 
 
 
 
 
 
 
 
}else count = 0; 
 
 
 
 
 
 
 
 
 
 
vTaskDelayUntil(&xLastWakeTime, 50); 
 
 
} 
 
 
 
} 
} 
这部分内容和vl53l0x 大同小异，先设置vl53l1x 测量参数vl53l1xSetParam，设置详细
内容如下： 
int vl53l1xSetParam(void) 
/*设置vl53l1x 参数*/ 
{ 
 
int status; 
 
 
 
status = VL53L1_WaitDeviceBooted(&dev); 
 
status = VL53L1_DataInit(&dev); 
 
status = VL53L1_StaticInit(&dev); 
 
status = VL53L1_SetDistanceMode(&dev, VL53L1_DISTANCEMODE_LONG); 
 
status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(&dev, 45000); 
 
status = VL53L1_SetInterMeasurementPeriodMilliSeconds(&dev, 50);

## Page 19: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 17 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
status = VL53L1_StopMeasurement(&dev); 
 
status = VL53L1_StartMeasurement(&dev); 
 
 
return status; 
} 
这些API 函数都可以在上面说的文档里面找到说明，简单说你个重要的API 函数： 
VL53L1_SetDistanceMode：设置测量模式，包括3 个模式，短距离模式(<136cm)、中距
离模式(<290cm)和长距离模式(<400cm)，默认设置为长距离模式，测量距离可以到400cm。 
VL53L1_SetMeasurementTimingBudgetMicroSeconds：设置测量时间，单位us，不同模
式对测量时间有不同的要求，测量距离越大，需要的时间越长，短距离模式要求测量时间不
小于20ms，所有工作模式正常工作要求测量时间不小于33ms，测量时间设置越大，数据越
精确，我们默认设置为45ms。 
VL53L1_SetInterMeasurementPeriodMilliSeconds：设置连续测量模式间隔，因为除了测
量时间，读取也需要时间，所以这个时间必须大于测量时间，我们设置为50ms。 
参数设置完成后进入while()循环，先判断reInitvl53l1x 这个变量，如果为真，表示再次
使用激光测距，那么我们需要设置一下测量模式，接着才是以20Hz 读取激光数据，接着对
激光数据分析，先判断激光数据是否有效，判断依据测量值是否小于最大值
VL53L1X_MAX_RANGE，这是一个宏，在vl53l1x.h 中定义，宏的内容为410，表示最大测
量距离410cm，如果测量数据有效变量validCnt 加1，否则无效变量inValidCnt 加1，当有
效无效变量相加等于10，我们进行一次数据可信度计算，计算很简单，将数据有效次数除
以总次数，然后低通处理，这样我们得到激光数据可信度quality，范围0~1.0；可以看到后
边还有一次对测量数据的判断，这个主要用于识别vl53l1x 是否掉线，当vl53l1x 掉线（比
如拔掉光流模块）一段时间之后，此时，我们将激光测距任务挂起，直到激光传感器恢复正
常。 
2.4.4. VL53LXX 读取数据 
上面激光测距任务内得到了激光数据和可信度，那要如何用上这个数据和可信度呢，当
然是要调用相应的函数把数据和可信度送到需要的地方，激光数据读取源码如下： 
bool vl53lxxReadRange(zRange_t* zrange) 
{ 
 
if(vl53lxxId == VL53L0X_ID)  
 
{ 
 
 
zrange->quality = quality; 
 
//可信度 
 
 
vl53lxx.quality = quality; 
 
 
 
 
 
if (range_last != 0 && range_last < VL53L0X_MAX_RANGE)  
 
 
{ 
 
 
 
 
 
 
zrange->distance = (float)range_last; 
//单位[cm] 
 
 
 
vl53lxx.distance =  zrange->distance;  
 
 
 
 
return true; 
 
 
} 
 
} 
 
else if(vl53lxxId == VL53L1X_ID)  
 
{

## Page 20: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 18 / 21 
 
ATK-PMW3901 光流模块用户手册 
 
 
zrange->quality = quality; 
 
//可信度 
 
 
vl53lxx.quality = quality; 
 
 
 
 
 
if (range_last != 0 && range_last < VL53L1X_MAX_RANGE)  
 
 
{ 
 
 
 
 
 
 
zrange->distance = (float)range_last; 
//单位[cm] 
 
 
 
 
vl53lxx.distance =  zrange->distance; 
 
 
 
return true; 
 
 
} 
 
} 
 
 
 
return false; 
} 
可以看到，我们直接根据初始化读取到的ID 读取距离信息和可行度信息，内容比较简
单，先说说这个结构体参数zRange_t，定义如下： 
typedef struct zRange_s  
{ 
 
uint32_t timestamp; 
//时间戳 
 
float distance;  
 
//测量距离 
 
float quality;  
 
//可信度 
} zRange_t; 
看注释就可以理解了。 
先读取可信度，这个可信度是实时更新，而测量数据则需要经过判断，如果测量值不为
零且小于VL53L0X_MAX_RANGE / VL53L1X_MAX_RANGE，则读取测量数据。到此，激
光数据测量完成。 
 
激光读取的数据用于高度融合，这部分内容请看state_estmator.c，源码内有详细的高度
融合过程，简单说就是气压和激光数据融合得到高度信息，同时根据z 轴加速度会估测出一
个高度信息和z 轴速度信息，然后用融合的高度信息去校正估测高度和z 轴估测速度，最后
得到姿态高度和姿态速度（z 轴）。最后，同样也是在源码position_pid.c 里面进行高度串级
PID 控制，也就是PID 位置环加PID 速度环的方式，从而实现定高控制。 
 
最后还有一个函数setVl53lxxState，这个函数用户是否使能激光传感器，因为考虑到用
户带着光流模块室外飞行，但是室外红外光对VL53LXX 干扰较大，无法正常使用，这个时
候，我们就可以通过遥控关闭VL53LXX（遥控器固件先升级到 V1.3 或以上版本），然后
使用气压计定高，这样MiniFly 就可以户外定点飞行了（注意定点飞行高度<4m）。

## Page 21: ALIENTEK

ALIENTEK 
超轻低功耗光流模块 
 
 19 / 21 
 
ATK-PMW3901 光流模块用户手册 
3. 其他 
3.1 购买地址 
官方店铺：https://openedv.taobao.com/  
3.2 资料下载 
下载地址：http://www.openedv.com/thread-105197-1-1.html  
3.3 技术支持  
公司网址：www.alientek.com 
技术论坛：www.openedv.com 
传真：020-36773971 
电话：020-38271790