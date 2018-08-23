//* 主控板主程序  *//
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "tim.h"
#include "can.h"
#include "stmflash.h"
#include "gpio.h"

/*待完善功能*/
//u8 SelfTest(void); //开机自检
//void LoadParameter(void); //预置参数加载

 int main(void)
 {	
		NVIC_Configuration();	//全局中断配置
		delay_init();			//延时函数初始化
		IO_Init();				//LED,蜂鸣器初始化
		uart_init(115200);	 	//串口初始化
		can_init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_Normal);//CAN接口初始化 bps = 500K
	 	TIM3_init();			//定时器初始化
		TIM2_init();
	 	//各模块完成一次自检 
		while(1)
		{
		  //CAN1_Send_Msg(CAN_Msg.RelayCtrl,CAN_BUFFSIZE,0x41);
			delay_ms(2);
		}		
}




//***********************************END FILE***********************************//
