#ifndef _GPIO_H_
#define _GPIO_H_

#include "stm32f10x_gpio.h"

#define LED1 PBout(9)
#define LED2 PBout(8)
#define LED3 PBout(7)
#define LED4 PBout(6)
#define BEEP PBout(11)

void IO_Init(void);

#endif

