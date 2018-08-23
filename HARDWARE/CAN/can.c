/*******************************************************************************
|  File Name  :  can.c
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
| 2018-06-3     01.00.00     xujialiang        Creation
*******************************************************************************/
#include "can.h"
/*******************************************************************************
|    Other Header File Inclusion
*******************************************************************************/

#include "usart.h"
/*******************************************************************************
|    Compile Option or configuration Section  (??)
*******************************************************************************/

/*******************************************************************************
|    Local Macro Definition
*******************************************************************************/

/*************************adpater APIs******************************************/

/*******************************************************************************
|    Local Enum Definition
*******************************************************************************/

/*******************************************************************************
|    Local Typedef Definition
*******************************************************************************/

/*******************************************************************************
|    global variables Declaration
*******************************************************************************/
CanRxMsg CanReceiveMsg;
u8 CAN_ReceiveFlag;
/*******************************************************************************
|    static local variables Declaration
*******************************************************************************/

/*******************************************************************************
|    Function Source Code
*******************************************************************************/
	
/*******************************************************************************
| FUNCTION NAME   :can_init		
| CALLED BY 	  	: 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  	:can接口初始化
*******************************************************************************/
void can_init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef  	CAN_FilterInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//使能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);
	//CAN_RX 上拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	//CAM_TX 复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	CAN_InitStructure.CAN_TTCM=DISABLE;
	CAN_InitStructure.CAN_ABOM=ENABLE;  
	CAN_InitStructure.CAN_AWUM=DISABLE;
	CAN_InitStructure.CAN_NART=ENABLE;
	CAN_InitStructure.CAN_RFLM=DISABLE;
	CAN_InitStructure.CAN_TXFP=DISABLE;	
	CAN_InitStructure.CAN_Mode= mode;	
	
	CAN_InitStructure.CAN_SJW=tsjw;	
	CAN_InitStructure.CAN_BS1=tbs1; 
	CAN_InitStructure.CAN_BS2=tbs2;
	CAN_InitStructure.CAN_Prescaler=brp; 
	CAN_Init(CAN1, &CAN_InitStructure);   
		
	CAN_FilterInitStructure.CAN_FilterNumber=0;	
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; 	
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; 
	CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;	
	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);			

	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);// FIFO消息与挂号中断允许
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}	

/*******************************************************************************
| FUNCTION NAME   :USB_LP_CAN1_RX0_IRQHandler
| CALLED BY 	  	: 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  	:can接收中断
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	if((CAN_MessagePending(CAN1,CAN_FIFO0)!=0) &&(CAN_ReceiveFlag == OFF))
	{
		CAN_Receive(CAN1,CAN_FIFO0,&CanReceiveMsg);
		switch(CanReceiveMsg.StdId)
		{
			case hPowerCtrl_Id:
				CAN_ReceiveFlag = ON;break;
			case RelayCtrlBoard1_Id:
				CAN_ReceiveFlag = ON;break;
			default:
				printf("ERROR!\r\n");break;
		}
	}
}

/*******************************************************************************
| FUNCTION NAME   :CAN1_Send_Msg
| CALLED BY 	 		: 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  	:can发送报文
*******************************************************************************/
u8 CAN1_Send_Msg(u8* msg,u8 len,u32 StdId)
{	
  u8 mbox;
  u16 i=0;
  CanTxMsg TxMessage;

  CAN_ReceiveFlag = OFF;	//发送报文前先复位接收标志位 
  TxMessage.StdId = StdId;	
  TxMessage.ExtId=0x00;	 
  TxMessage.IDE=CAN_Id_Standard;		
  TxMessage.RTR=CAN_RTR_DATA;		 
  TxMessage.DLC=len;							
  for(i=0;i<len;i++)
  TxMessage.Data[i]=msg[i];				
  mbox= CAN_Transmit(CAN1, &TxMessage);   
  i=0;
  while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	
  if(i>=0XFFF)return 1;
  return 0;		
}

/*******************************************************************************
| FUNCTION NAME   :Can_Receive_Msg
| CALLED BY 	  	: 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  	:can接收报文
*******************************************************************************/
u8 Can_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
	
    if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    for(i=0;i<8;i++)
    buf[i]=RxMessage.Data[i];  
	return RxMessage.DLC;	
}
