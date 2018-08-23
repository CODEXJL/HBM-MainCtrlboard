/*******************************************************************************
|  File Name  :  tim.c
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
#include "tim.h"
#include "gpio.h"
/*******************************************************************************
|    Other Header File Inclusion
*******************************************************************************/

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

/*******************************************************************************
|    static local variables Declaration
*******************************************************************************/

/*******************************************************************************
|    Function Source Code
*******************************************************************************/

/*******************************************************************************
| FUNCTION NAME   :TIM3_init	
| CALLED BY 	  	: 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  	:TIM3³õÊ¼»¯   
*******************************************************************************/
void TIM3_init(void)  	
{
 /* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);	
	
  	TIM_TimeBaseStructure.TIM_Period = 999;    //10ms  = (10000*72)/72000000
  	TIM_TimeBaseStructure.TIM_Prescaler = 719;		
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);                    	
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );
    
	TIM_Cmd(TIM3, ENABLE);  
	
}

/*******************************************************************************
| FUNCTION NAME   :TIM3_IRQHandler		
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :TIM3中断服务程序
*******************************************************************************/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);		
		USART_Data_Analyse();
	}
}

/*******************************************************************************
| FUNCTION NAME   :TIM2_init	
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :TIM2初始化  蜂鸣器定时
*******************************************************************************/
void TIM2_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);	

	TIM_TimeBaseStructure.TIM_Period = 4999;     			
  TIM_TimeBaseStructure.TIM_Prescaler = 7199;		
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);                    	
    
	TIM_Cmd(TIM2, ENABLE); 	
}

/*******************************************************************************
| FUNCTION NAME   :TIM2_IRQHandler		
| CALLED BY 	  : 													 
| PRECONDITIONS   : 										  
| INPUT PARAMETERS: 														   
| RETURN VALUE	  :
| DESCRIPTION	  :TIM2中断服务程序
*******************************************************************************/
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		BEEP = !BEEP;
	}
}
