/*
 * RC.h
 *************************************************************************************************
 *  Created on: Mar 19, 2020
 *      Author: Nejc Tovornik
 *************************************************************************************************
 *  RC library for controlling multiple devices that support RC PWM standard on one microcontroller.
 *
 *  Copyright (C) 2020  Nejc Tovornik
 *
 * 	This program is free software: you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *************************************************************************************************
 */

#ifndef INC_RC_H_
#define INC_RC_H_

#include "stm32f7xx.h"
#include "usbd_cdc_if.h"

typedef enum bool {
	false = 0, true
} bool;

typedef struct RC {
	TIM_HandleTypeDef *htim;
	uint32_t channel;
	uint32_t pulse;
	uint16_t minTime;
	uint16_t maxTime;
	bool isRunning;
} RC;

#define USB_CONTROL_RC_LIST_STACK_SIZE 10

bool PULSE_IN_PERCENT;
RC *USB_CONTROL_RC;
RC *USB_CONTROL_RC_LIST[USB_CONTROL_RC_LIST_STACK_SIZE];

void RC_Start(RC *dev);
void RC_Stop(RC *dev);
void RC_USB_FS_Control(RC *dev)__attribute__((deprecated));
void RC_setPulse(RC *dev, uint16_t val);
int RC_getPulse(RC *dev);
void RC_setPercentMode(bool val);

void RC_addToControlList(RC *dev);
void RC_removeFromControlList(RC *dev);
int RC_getSelectedIndexFromControlList(void);
void RC_setSelectedIndexInControlList(int index);
void RC_setSelectedDevInControlList(RC *dev);

#endif /* INC_RC_H_ */
