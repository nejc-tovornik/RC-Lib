/*
 * RC.c
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

#include <RC.h>

//========================== GLOBALNE SPREMENLJIVKE ==========================

char rxBuffer[5];
char txBuffer[100];
int setVal;
int stack_top = -1;

//============================== POMOŽNE FUNKCIJE ==============================

/*
 * @brief Pomožna funkcija za mapiranje vrednosti.
 *
 * @param x: Vrednost ki jo želimo mapirati
 * @param in_min: Minimalna vrednost, ki jo zavzame x
 * @param in_max: Maksimalna vrednost, ki jo zavzame x
 * @param out_min: Minimalna vrednost izhoda
 * @param out_max: Maksimalna vrednost izhoda
 *
 * @retval long Mapirana vrednost
 */
static long map(long x, long in_min, long in_max, long out_min, long out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
 * @brief Pomožna funkcija za omejevanje vrednosti
 *
 * @param val: Vrednost katero želimo omejiti
 * @param min: Spodnja meja
 * @param max: Zgornja meja
 *
 * @retval uint16_t Vrne omejeno vrednost med min in max
 *
 */
static uint16_t constrain(uint16_t val, uint16_t min, uint16_t max) {
	if (val < min) {
		return min;
	} else if (val > max) {
		return max;
	} else {
		return val;
	}
}

/*
 * @brief Funkcija push doda kazalec naprave na stack.
 *
 * @param *dev: Kazalec
 *
 * @retval None
 */
static void push(RC *dev) {
	if (stack_top == USB_CONTROL_RC_LIST_STACK_SIZE - 1)
		return;
	else {
		//Preveri če že v stacku
		bool _inStack = false;
		for (int i = 0; i < stack_top + 1; i++) {
			if (USB_CONTROL_RC_LIST[i] == dev) {
				_inStack = true;
			}
		}
		if (!_inStack) {
			stack_top += 1;
			USB_CONTROL_RC_LIST[stack_top] = dev;
		}
	}
}

/*
 * @brief Funkcija pop odstrani kazalec naprave iz stacka.
 *
 * @param *dev: Kazalec
 *
 * @retval None
 */
static void pop(RC *dev) {
	if (stack_top == -1)
		return;

	//Odstrani napravo iz stacka
	int indexItem = 0;
	for (int i = 0; i < stack_top + 1; i++) {
		if (USB_CONTROL_RC_LIST[i] == dev) {
			USB_CONTROL_RC_LIST[i] = 0x00;
			indexItem = i;
			break;
		}
	}

	//Če je bil element zbrisan iz sredine stacka postisni stack za 1 dol
	if (stack_top != indexItem) {
		for (int i = indexItem; i < stack_top + 1; i++) {
			USB_CONTROL_RC_LIST[i] = USB_CONTROL_RC_LIST[i + 1];
		}
		stack_top = stack_top - 1;
	} else {
		stack_top = stack_top - 1;
	}

}

/*
 * @brief Počisti podani buffer
 *
 * @param *buf: Kazalec
 *
 * @retval None
 */
static void flush(char *buf) {
	for (int i = 0; i < sizeof(buf); i++) {
		buf[i] = '\0';
	}
}

//============================== GLAVNE FUNKCIJE ==============================

/*
 * @brief Določi katera naprava bo komuniciral z USBjem
 *
 * @param *dev: Kazalec
 *
 * @retval None
 */
void RC_USB_FS_Control(RC *dev) {
	USB_CONTROL_RC = dev;
}

/**
 * @brief Nastavitev širine pulza.
 *
 * @param *dev: Kazalec
 *
 * @param val: Širina pulza (odvisna od PULSE_IN_PERCENT)
 *
 * @retval None
 */
void RC_setPulse(RC *dev, uint16_t val) {

	if (PULSE_IN_PERCENT) {
		// Če nastavljamo pulzno širino v procentih
		dev->pulse = map(constrain(val, 0, 100), 0, 100, dev->minTime,
				dev->maxTime);
		switch (dev->channel) {
		case TIM_CHANNEL_1:
			dev->htim->Instance->CCR1 = map(constrain(val, 0, 100), 0, 100,
					dev->minTime, dev->maxTime);
			break;
		case TIM_CHANNEL_2:
			dev->htim->Instance->CCR2 = map(constrain(val, 0, 100), 0, 100,
					dev->minTime, dev->maxTime);
			break;
		case TIM_CHANNEL_3:
			dev->htim->Instance->CCR3 = map(constrain(val, 0, 100), 0, 100,
					dev->minTime, dev->maxTime);
			break;
		case TIM_CHANNEL_4:
			dev->htim->Instance->CCR4 = map(constrain(val, 0, 100), 0, 100,
					dev->minTime, dev->maxTime);
			break;
		case TIM_CHANNEL_5:
			dev->htim->Instance->CCR5 = map(constrain(val, 0, 100), 0, 100,
					dev->minTime, dev->maxTime);
			break;
		case TIM_CHANNEL_6:
			dev->htim->Instance->CCR6 = map(constrain(val, 0, 100), 0, 100,
					dev->minTime, dev->maxTime);
			break;
		default:
			break;
		}
	} else {
		// Če nastavljamo pulzno širino v us
		dev->pulse = constrain(val, dev->minTime, dev->maxTime);
		switch (dev->channel) {
		case TIM_CHANNEL_1:
			dev->htim->Instance->CCR1 = constrain(val, dev->minTime,
					dev->maxTime);
			break;
		case TIM_CHANNEL_2:
			dev->htim->Instance->CCR2 = constrain(val, dev->minTime,
					dev->maxTime);
			break;
		case TIM_CHANNEL_3:
			dev->htim->Instance->CCR3 = constrain(val, dev->minTime,
					dev->maxTime);
			break;
		case TIM_CHANNEL_4:
			dev->htim->Instance->CCR4 = constrain(val, dev->minTime,
					dev->maxTime);
			break;
		case TIM_CHANNEL_5:
			dev->htim->Instance->CCR5 = constrain(val, dev->minTime,
					dev->maxTime);
			break;
		case TIM_CHANNEL_6:
			dev->htim->Instance->CCR6 = constrain(val, dev->minTime,
					dev->maxTime);
			break;
		default:
			break;
		}
	}
}

/*
 * @brief Ustavi časovnik naprave.
 *
 * @param *dev: Kazalec
 *
 * @retval None
 */
void RC_Stop(RC *dev) {
	HAL_TIM_PWM_Stop(dev->htim, dev->channel);
	dev->isRunning = false;
}

/**
 * @brief Nastavi in zažene časovnik za generiranje pwm signala.
 *
 * @param *dev: Kazalec
 *
 * @retval None
 */
void RC_Start(RC *dev) {

	HAL_TIM_PWM_Start(dev->htim, dev->channel);

	switch (dev->channel) {
	case TIM_CHANNEL_1:
		dev->htim->Instance->CCR1 = dev->pulse;
		break;
	case TIM_CHANNEL_2:
		dev->htim->Instance->CCR2 = dev->pulse;
		break;
	case TIM_CHANNEL_3:
		dev->htim->Instance->CCR3 = dev->pulse;
		break;
	case TIM_CHANNEL_4:
		dev->htim->Instance->CCR4 = dev->pulse;
		break;
	case TIM_CHANNEL_5:
		dev->htim->Instance->CCR5 = dev->pulse;
		break;
	case TIM_CHANNEL_6:
		dev->htim->Instance->CCR6 = dev->pulse;
		break;
	default:
		break;
	}
	dev->isRunning = true;
}

/*
 * @brief Vrne širino pulza. Če je PULSE_IN_PERCENT = true, funkcija vrne
 * 		  število od 0-100, v nasprotju pa od minTime do maxTime.
 *
 * @param *dev: Kazalec
 *
 * @retval int Trenutna širina
 */
int RC_getPulse(RC *dev) {
	if (PULSE_IN_PERCENT) {
		return map(dev->pulse, dev->minTime, dev->maxTime, 0, 100);
	} else {
		return dev->pulse;
	}
}

/*
 * @brief Vklopi ali izklopi procentni način.
 *
 * @param val: bool
 *
 * @retval None
 */
void RC_setPercentMode(bool val) {
	PULSE_IN_PERCENT = val;
}

/*
 * @brief Dodaj napravo na seznalm. Preko VCP dostopamo do seznama in zberemo s
 * 		  katerio napravo komuniciramo.
 *
 * @param *dev: Kazalec
 *
 * @retval None
 */
void RC_addToControlList(RC *dev) {
	if (stack_top == -1) {
		push(dev);
		RC_setSelectedIndexInControlList(0);
	} else {
		for (int i = 0; i < stack_top + 1; i++) {
			if (USB_CONTROL_RC_LIST[i] != dev) {
				push(dev);
				if (RC_getSelectedIndexFromControlList() == -1) {
					RC_setSelectedIndexInControlList(i);
				}
			}
		}
	}
}

/*
 * @brief Odstrani napravo s seznalma. Odstranjena naprava ne bo več viden preko VCP
 *
 * @param *dev: Kazalec
 *
 * @retval None
 */
void RC_removeFromControlList(RC *dev) {
	if (stack_top == -1)
		return;
	for (int i = 0; i < stack_top + 1; i++) {
		if (USB_CONTROL_RC_LIST[i] == dev) {
			pop(dev);
		}
	}
}

/*
 * @brief Vrne indeks izbrane naprave na seznamu.
 *
 * @param *dev: Kazalec
 *
 * @retval int indeks
 */
int RC_getSelectedIndexFromControlList(void) {
	if (stack_top == -1) {
		if (USB_CONTROL_RC != 0x00) {
			push(USB_CONTROL_RC);
		}
	}
	for (int i = 0; i < stack_top + 1; i++) {
		if (USB_CONTROL_RC_LIST[i] == USB_CONTROL_RC) {
			return i;
		}
	}
	return -1;
}

/*
 * @brief Izberi napravo s seznama za VCP krmiljenje
 *
 * @param index: Indeks željene naprave
 *
 * @retval None
 */
void RC_setSelectedIndexInControlList(int index) {
	if (stack_top == -1) {
		if (USB_CONTROL_RC != 0x00) {
			push(USB_CONTROL_RC);
		}
	}
	if (index > stack_top || index < 0) {
		return;
	} else {
		USB_CONTROL_RC = USB_CONTROL_RC_LIST[index];
	}
}

//============================== PREJEMANJE UKAZOV ==============================

/*
 * @brief Funkcija za prejemanje ukazov preko VCP
 *
 * @param *buf: Kazalec poslanega podatka
 * @param len: Dolžina poslanega podatka
 *
 * @retval None
 */
void CDC_ReceiveCallback(uint8_t *buf, uint32_t len) {
	sprintf(rxBuffer, (char*) buf);

	for (int i = 0; i < sizeof(rxBuffer); i++) {
		if (rxBuffer[i] == '\r' || rxBuffer[i] == '\n') {
			rxBuffer[i] = '\0';
		}
	}

	switch (rxBuffer[0]) {
	case 'S':
		//Če sscanf najde številko vrne 1
		if (rxBuffer[1] == '\r' || rxBuffer[1] == '\n' || rxBuffer[1] == '\0') {
			flush(txBuffer);
			sprintf(txBuffer, "ERROR:0\n");
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
		} else {
			rxBuffer[0] = '+'; //Nadomestim S z + zato, da sscanf prebere kot št.
			if (sscanf(rxBuffer, "%d", &setVal)) {
				RC_setPulse(USB_CONTROL_RC, setVal);
				flush(txBuffer);
				sprintf(txBuffer, "Set OK\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			} else {
				flush(txBuffer);
				sprintf(txBuffer, "ERROR:1\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			}
		}
		flush(rxBuffer);
		break;
	case 'R':
		if (rxBuffer[1] == '\r' || rxBuffer[1] == '\n' || rxBuffer[1] == '\0') {
			flush(txBuffer);
			sprintf(txBuffer, "%d\n", RC_getPulse(USB_CONTROL_RC));
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
		} else {
			flush(txBuffer);
			sprintf(txBuffer, "ERROR:2\n");
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
		}
		flush(rxBuffer);
		break;
	case 'H':
		if (rxBuffer[1] == '\r' || rxBuffer[1] == '\n' || rxBuffer[1] == '\0') {
			if (USB_CONTROL_RC->isRunning) {
				flush(txBuffer);
				sprintf(txBuffer, "ON\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			} else {
				flush(txBuffer);
				sprintf(txBuffer, "OFF\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			}
		} else {
			flush(txBuffer);
			sprintf(txBuffer, "ERROR:3\n");
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
		}
		flush(rxBuffer);
		break;
	case 'P':
		if (rxBuffer[1] == '\r' || rxBuffer[1] == '\n' || rxBuffer[1] == '\0') {
			if (PULSE_IN_PERCENT) {
				flush(txBuffer);
				sprintf(txBuffer, "PIP:1\n");
			} else {
				flush(txBuffer);
				sprintf(txBuffer, "PIP:0\n");
			}
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
		} else {
			rxBuffer[0] = '+'; //Nadomestim P z + zato, da sscanf prebere kot št.
			sscanf(rxBuffer, "%d", &setVal);
			if (setVal >= 0 && setVal <= 1) {
				RC_setPercentMode(constrain(setVal, 0, 1));
				flush(txBuffer);
				sprintf(txBuffer, "OK\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			} else {
				flush(txBuffer);
				sprintf(txBuffer, "ERROR:4\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			}
		}
		flush(rxBuffer);
		break;
	case 'C':
		//Če je poslan samo C vrni indeks izbranega objekta v stacku, drugače izberi objekt iz stacka
		if (rxBuffer[1] == '\r' || rxBuffer[1] == '\n' || rxBuffer[1] == '\0') {
			flush(txBuffer);
			sprintf(txBuffer, "%d\n", RC_getSelectedIndexFromControlList());
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
		} else if (rxBuffer[1] == 'L') {
			flush(txBuffer);
			//Ispis stacka
			char midBuf[20];
			for (int i = 0; i < stack_top + 1; i++) {
				sprintf(midBuf, "%d : %p\n", i, USB_CONTROL_RC_LIST[i]);
				strcat(txBuffer, midBuf);
			}
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
		} else {
			//Določi USB_CONTROL_RC iz stacka
			rxBuffer[0] = '+';
			sscanf(rxBuffer, "%d", &setVal);
			if (setVal >= 0 && setVal <= stack_top) {
				RC_setSelectedIndexInControlList(setVal);
				flush(txBuffer);
				sprintf(txBuffer, "OK\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			} else {
				flush(txBuffer);
				sprintf(txBuffer, "ERROR:5\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			}
		}
		flush(rxBuffer);
		break;
	case 'T':
		if (rxBuffer[1] == '\r' || rxBuffer[1] == '\n' || rxBuffer[1] == '\0') {
			//nedefinirano
			flush(txBuffer);
			sprintf(txBuffer, "ERROR:6\n");
			CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			return;
		} else {
			rxBuffer[0] = '+';
			sscanf(rxBuffer, "%d", &setVal);
			if (setVal >= 0 && setVal <= stack_top) {
				if (setVal) {
					RC_Start(
							USB_CONTROL_RC_LIST[RC_getSelectedIndexFromControlList()]);
				} else {
					RC_Stop(
							USB_CONTROL_RC_LIST[RC_getSelectedIndexFromControlList()]);
				}
				flush(txBuffer);
				sprintf(txBuffer, "OK\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			} else {
				flush(txBuffer);
				sprintf(txBuffer, "ERROR:7\n");
				CDC_Transmit_FS((uint8_t*) txBuffer, strlen(txBuffer));
			}
		}
		flush(rxBuffer);
		break;
	default:
		flush(rxBuffer);
		break;
	}
}
