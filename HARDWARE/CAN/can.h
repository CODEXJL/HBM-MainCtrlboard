#ifndef _CAN_H_
#define _CAN_H_

#include "stm32f10x_can.h"

#define CAN_BUFFSIZE  8

#define hPowerCtrl_Id 0x1
#define IV_Ctrl_Id 0x2

#define RelayCtrlBoard1_Id 0x41				//¼ÌµçÆ÷µ¥Ôª°åIDºÅ

extern CanRxMsg CanReceiveMsg;
extern u8 CAN_ReceiveFlag;

void can_init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode);
u8 CAN1_Send_Msg(u8* msg,u8 len,u32 StdId);
u8 Can_Receive_Msg(u8 *buf);

#endif
