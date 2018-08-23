/*******************************************************************************
|  File Name  :  usart.c
|  Description: 
|-------------------------------------------------------------------------------
| (c) This software is proprietary of xyz.
|     All rights are reserved by xyz.
|-------------------------------------------------------------------------------
| Initials      Name                   Company
| --------      --------------------   -----------------------------------------
| XXX           XXXX                   SHRIME
|-------------------------------------------------------------------------------
|               R E V I S I O N   H I S T O R Y
| Date           Version      Author     Description
| ------------   --------     -------    ------------------------------------
| 2018-05-26     01.00.00     xujialiang        Creation
*******************************************************************************/
#include "sys.h"
#include "usart.h"
#include "CRC.h"
#include "stmflash.h"
#include "delay.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"
/*******************************************************************************
|    Other Header File Inclusion
*******************************************************************************/

/*******************************************************************************
|    Compile Option or configuration Section  (??)
*******************************************************************************/
#if 1//print func suppot
#pragma import(__use_no_semihosting)    
              
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
 
_sys_exit(int x) 
{ 
	x = x; 
} 

int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0); 
    USART1->DR = (u8) ch;      
	return ch;
}
#endif
/*******************************************************************************
|    Local Macro Definition
*******************************************************************************/
#define Parameter_NUM 10 //预置参数数量

/*************************adpater APIs******************************************/

/*******************************************************************************
|    Local Enum Definition
*******************************************************************************/

/*******************************************************************************
|    Local Typedef Definition
*******************************************************************************/
typedef struct
{
	u8 Rx_Count;					
	u8 Rev_Flag;					
	u8 RxBuf[BUFFSIZE];				
}USART_ReceiveTypeDef;

typedef struct 
{
	u8 Data1[BUFFSIZE];
}Cal_ParameterTypeDef;

/*******************************************************************************
|    global variables Declaration
*******************************************************************************/
USART_ReceiveTypeDef USART_Receive;
CAN_MsgTypeDef CAN_Msg;
Cal_ParameterTypeDef Cal_Parameter;

u8 Calibration_Parameter[Parameter_NUM][BUFFSIZE];//校正数据表
u8 DataRead[BUFFSIZE]= {0x00};

u8 LED_Count = 0;
/*******************************************************************************
|    static local variables Declaration
*******************************************************************************/

/*******************************************************************************
|    Function Source Code
*******************************************************************************/
	
/*******************************************************************************
| FUNCTION NAME   :uart_init		
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :串口初始化
*******************************************************************************/
void uart_init(u32 bound){
    //结构体声明
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
	USART_DeInit(USART1);
	//USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	
  
	//串口模式配置
	USART_InitStructure.USART_BaudRate = bound;//
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

    USART_Init(USART1, &USART_InitStructure); 
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);                    
}

/*******************************************************************************
| FUNCTION NAME   :USART_IRQHandler		
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :串口中断服务程序
*******************************************************************************/
void USART1_IRQHandler(void)    
{  
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
			 if(USART_Receive.Rx_Count > 15)
			 {
				 USART_Receive.Rx_Count = 0;
			 }
			 USART1 -> DR;
			 USART_Receive.RxBuf[USART_Receive.Rx_Count++] = USART_ReceiveData(USART1);
		} 
} 

/*******************************************************************************
| FUNCTION NAME   :USART_Data_Analyse
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :串口数据解析 
*******************************************************************************/
void USART_Data_Analyse(void)
{
	if((USART_Receive.RxBuf[14] + USART_Receive.RxBuf[15] == 0x82) && (USART_Receive.Rev_Flag == OFF)) //0XAA + 0xAA 
	{
		USART_Receive.Rev_Flag = ON;

		/*帧头CRC解析*/							
		switch (Get_Crc8(USART_Receive.RxBuf,2)) 				
		{
			case 0x2f:
					USART_CMD_IVCtrl();break; //IV测试
			case 0x42:
					USART_CMD_RelayCtrl();break;//RC继电器控制
			case 0x36:
					USART_CMD_hPower();break;//HP高压板控制
			case 0x75:
					USART_CMD_Test();break;//TS测试流程
			case 0xD6:
					USART_CMD_Calibration();break;//CA校正数据操作
			default:
					USART_Data_Flush(USART_Receive.RxBuf);break;	
		}
	}
	else USART_Data_Flush(USART_Receive.RxBuf);
}

/*******************************************************************************
| FUNCTION NAME   :USART_CMD_IVCtrl
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :IV测试
*******************************************************************************/
void USART_CMD_IVCtrl(void)
{
	u8 i,j;
	j =  4;
	for(i = 0;i < 8;i++)
	{
		CAN_Msg.IVCtrl[i] = USART_Receive.RxBuf[j];
		j++;
	}
	CAN1_Send_Msg(CAN_Msg.IVCtrl,CAN_BUFFSIZE,IV_Ctrl_Id);
	USART_SendReceive();
	USART_Data_Flush(USART_Receive.RxBuf);
}

/*******************************************************************************
| FUNCTION NAME   :USART_CMD_RelayCtrl
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS:na 														   
| RETURN VALUE	  :na
| DESCRIPTION	  :将串口的数据用CAN接口发送到继电器控制板
*******************************************************************************/
void USART_CMD_RelayCtrl(void)
{
	u8 i,j;
	u8 RelayCtrlBoard_Id;
	
	j =  5;
	RelayCtrlBoard_Id = USART_Receive.RxBuf[4];//获取继电器子板ID

	/*UART的Byte5~Byte12为CAN的数据报文*/
	for(i = 0;i < 8;i++)
	{
		CAN_Msg.RelayCtrl[i] = USART_Receive.RxBuf[j];
		j++;
	}
	/*发送CAN报文*/
	CAN1_Send_Msg(CAN_Msg.RelayCtrl,CAN_BUFFSIZE,RelayCtrlBoard_Id);
	while(CAN_ReceiveFlag != OFF);
	USART_SendReceive();
	USART_Data_Flush(USART_Receive.RxBuf);
	//fake
	LED_Count++;
	switch(LED_Count)
	{
		case 0x01:LED1 = !LED1;break;
		case 0x02:LED2 = !LED2;break;
		case 0x03:LED3 = !LED3;break;
		case 0x04:LED4 = !LED4;break;
	}
	if(LED_Count>=0x04) LED_Count = 0x00;
	
}

/*******************************************************************************
| FUNCTION NAME   :USART_CMD_hPower
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :高压源控制
*******************************************************************************/
void USART_CMD_hPower(void)
{
	u8 i,j;
	j =  4;
	for(i = 0;i < 8;i++)
	{
		CAN_Msg.hPowerCtrl[i] = USART_Receive.RxBuf[j];
		j++;
	}
	CAN1_Send_Msg(CAN_Msg.hPowerCtrl,CAN_BUFFSIZE,hPowerCtrl_Id);
	USART_SendReceive();
	USART_Data_Flush(USART_Receive.RxBuf);
}

/*******************************************************************************
| FUNCTION NAME   :USART_CMD_Test
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :主机测试流程
*******************************************************************************/
void USART_CMD_Test(void)
{
	char CMD_id[2];
	int id;

	/*功能位解析*/
	CMD_id[0] = USART_Receive.RxBuf[2];
	CMD_id[1] = USART_Receive.RxBuf[3];
	id = atoi(CMD_id);
	switch (id)
	{
		case 1://主机型号
				TEST_CMD_ModelType_Id(0x00);break;
		case 2://主机编号
				TEST_CMD_ModelType_Id(0x01);break;
		case 3://蜂鸣器开关
				TEST_CMD_BEEP();break;
		case 4://主机复位
				TEST_CMD_Reset();break;
		default:
			break;
	}
	USART_Data_Flush(USART_Receive.RxBuf);//清空串口接收缓存
	USART_Receive.Rev_Flag = OFF;
}

/*******************************************************************************
| FUNCTION NAME   :USART_CMD_Calibration
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :校正功能
*******************************************************************************/
void USART_CMD_Calibration(void)
{
	char CMD_id[2];
	int id;

	/*功能位解析*/
	CMD_id[0] = USART_Receive.RxBuf[2];
	CMD_id[1] = USART_Receive.RxBuf[3];
	id = atoi(CMD_id);
	switch(id)
	{
		case 1://读预置参数
				Cal_CMD_ReadCalDate();break;
		case 2://写预置参数
				Cal_CMD_WriteCalData();break;
		default:
		break;				
	}
	USART_Data_Flush(USART_Receive.RxBuf);
	USART_Receive.Rev_Flag = OFF;
}

/*******************************************************************************
| FUNCTION NAME   :Cal_CMD_ReadCalDate
| CALLED BY 	  	: 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS:												   
| RETURN VALUE	  :
| DESCRIPTION	  	:读主机预置参数
*******************************************************************************/
void Cal_CMD_ReadCalDate(void)
{
	char id[2];
	int ID;

	/*串口数据位byte4,byte5为读出校正数据的编号*/
	id[0] = USART_Receive.RxBuf[4];
	id[1] = USART_Receive.RxBuf[5];
  ID = atoi(id);

  /*将读出的数据装入CalDataTemp*/
	STMFLASH_Read(FLASH_SAVE_ADDR+16*ID,(u16*)DataRead,BUFFSIZE);
	delay_ms(5);//读出延时
	USART_SendFlashRead(DataRead);
}

/*******************************************************************************
| FUNCTION NAME   :Cal_CMD_WriteCalData
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :写入预置/校正参数  id(byte4~byte5) start from 1
*******************************************************************************/
void Cal_CMD_WriteCalData(void)
{
	u8 i;
	char id[2];
	int ID;
	u8 CalDataTemp[BUFFSIZE] = {0x00}; //待写入FALSH的校正数据

	/*串口数据位byte4,byte5为写入校正数据的编号*/
	id[0] = USART_Receive.RxBuf[4];
	id[1] = USART_Receive.RxBuf[5];
  	ID = atoi(id);
	
	/*将串口数据装入校正数据表*/
	for(i = 0;i < BUFFSIZE;i++)
	{ 
		Calibration_Parameter[ID][i] = USART_Receive.RxBuf[i];
		CalDataTemp[i] = Calibration_Parameter[ID][i];
	}
	
	/*写入MCU内部FLASH*/
	STMFLASH_Write(FLASH_SAVE_ADDR+16*ID,(u16*)CalDataTemp,BUFFSIZE);
	delay_ms(5);//数据写入延时
	USART_SendReceive();
}

/*******************************************************************************
| FUNCTION NAME   :TEST_CMD_BEEP
| CALLED BY 	  : 										
| PRECONDITIONS   : 										  
| INPUT PARAMETERS:													   
| RETURN VALUE	  :
| DESCRIPTION	  :蜂鸣器提示 USART数据位最后两位表示开关量
*******************************************************************************/
void TEST_CMD_BEEP(void)
{
	char CMD_id[2];
	int status;

	CMD_id[0] = USART_Receive.RxBuf[12];
	CMD_id[1] = USART_Receive.RxBuf[13];
	status = atoi(CMD_id);
	switch(status)
	{
		case 1://蜂鸣器打开
				TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);break;
		case 2://蜂鸣器关闭
				BEEP = 1;
				LED1 = 0;LED2 = 0;LED3 = 0;LED4 = 0;
				TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE);break;
		default://默认关闭
				BEEP = 1;//off
				TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE);break;		
	}
	USART_SendReceive();
}

/*******************************************************************************
| FUNCTION NAME   :TEST_CMD_ModelType
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS:												   
| RETURN VALUE	  :
| DESCRIPTION	  :主机型号
*******************************************************************************/
void TEST_CMD_ModelType_Id(u8 cmd)
{
	switch(cmd)
	{
		case 0x00://型号
			printf("TS010000000HBMAA");break;
		case 0x01://编号
			printf("TS020000000001AA");break;	
	}
}
/*******************************************************************************
| FUNCTION NAME   :TEST_CMD_Reset
| CALLED BY 	  	: 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS:												   
| RETURN VALUE	  :
| DESCRIPTION	  	:主机复位
*******************************************************************************/
void TEST_CMD_Reset(void)
{
		//todo
		//reset relayCtrl
		//reset IVtest
		//reset HighvoltCtrl
		USART_SendReceive();
}

/********************************USART Functicn Code***************************/
/*******************************************************************************
| FUNCTION NAME   :USART_Data_Flush
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :
*******************************************************************************/
static void USART_Data_Flush(u8 Data[])
{
	u8 i = 0;
	for(i = 0;i < BUFFSIZE;i++)
	{
		Data[i] = 0x00;
	}
	USART_Receive.Rx_Count = 0x00;
	USART_Receive.Rev_Flag = OFF;
}

/*******************************************************************************
| FUNCTION NAME   :USART_SendReceive
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :send receivebuf
*******************************************************************************/
static void USART_SendReceive(void)
{
	u8 i = 0;
	u8 TX_BUF[BUFFSIZE];
	for(i = 0;i<BUFFSIZE;i++)
	{
		TX_BUF[i] = USART_Receive.RxBuf[i];
	}
	for(i = 0;i<BUFFSIZE;i++)
	{
		USART1 -> SR;
		USART1 -> DR = TX_BUF[i];
	  	while((USART1->SR&0x40)==0);
	}
}

/*******************************************************************************
| FUNCTION NAME   :USART_SendFlashRead
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :send Flash Read Data
*******************************************************************************/
static void USART_SendFlashRead(u8 FlashDataRead[])
{
	u8 t;
	u8 TX_BUF[BUFFSIZE];
	TX_BUF[0] = 'C';
	TX_BUF[1] = 'A';
	TX_BUF[2] = '0';
	TX_BUF[3] = '1';
	for(t = 4;t < 14;t++)
	{
		TX_BUF[t] = FlashDataRead[t];
	}
	TX_BUF[14] = 'A';
	TX_BUF[15] = 'A';
	/*Send TX_BUF*/
	for(t = 0; t < 16; t++)
	{
		USART1 -> SR;
		USART1 -> DR = TX_BUF[t];
		while((USART1 ->SR & 0x40) == OFF);			
	}
}
