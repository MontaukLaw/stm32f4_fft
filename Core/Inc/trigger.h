#ifndef __TRIGGER_H_
#define __TRIGGER_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "arm_math.h"
#include "main.h"

void rainbow_color_change(void);

// uint8_t trigger_control(uint16_t triggerLinePos, float32_t triggerValue);

void change_trigger_value(uint8_t inscreaseOrDecrease);

void change_trigger_frequecy(uint8_t inscreaseOrDecrease);

uint8_t trigger_control(uint16_t triggerLinePos);

#endif

