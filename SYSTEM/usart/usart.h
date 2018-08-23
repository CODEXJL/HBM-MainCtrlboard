#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "sys.h" 

#define BUFFSIZE 16
#define CAN_BUFFSIZE 8

typedef struct 
{
	u8 RelayCtrl[CAN_BUFFSIZE];
	u8 hPowerCtrl[CAN_BUFFSIZE];
	u8 IVCtrl[CAN_BUFFSIZE];			
}CAN_MsgTypeDef;

extern CAN_MsgTypeDef CAN_Msg;

void uart_init(u32 bound);
void USART_Data_Analyse(void);
static void USART_Data_Flush(u8 Data[]);
static void USART_SendReceive(void);
static void USART_SendFlashRead(u8 FlashDataRead[]);

void USART_CMD_RelayCtrl(void);
void USART_CMD_Test(void);
void USART_CMD_hPower(void);
void USART_CMD_IVCtrl(void);
void USART_CMD_Calibration(void);

void TEST_CMD_BEEP(void);
void TEST_CMD_ModelType_Id(u8 cmd);
void TEST_CMD_Reset(void);

void Cal_CMD_ReadCalDate(void);
void Cal_CMD_WriteCalData(void);

#endif

