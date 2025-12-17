/***************************************************************************//**
* \file DisplayInterface.h
* \version 1.0
*
* \brief
* Objective:
*    This is display software i8080 interface header file.
*
********************************************************************************
* \copyright
* Copyright 2018-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/
/*#include "mbed.h"
#ifndef LCD_REG0
DigitalInOut LCD_REG0(P9_0);
DigitalInOut LCD_REG1(P9_1);
DigitalInOut LCD_REG2(P9_2);
DigitalInOut LCD_REG3(P9_4);
DigitalInOut LCD_REG4(P9_5);
DigitalInOut LCD_REG5(P0_2);
DigitalInOut LCD_REG6(P13_0);
DigitalInOut LCD_REG7(P13_1);

DigitalOut LCD_NWR(P12_0);
DigitalOut LCD_DC(P12_1);
DigitalOut LCD_RESET(P12_2);
DigitalOut LCD_NRD(P12_3);
#endif
*/
#include <DigitalInOut.h>
#include <DigitalOut.h>

extern mbed::DigitalInOut LCD_REG0;
extern mbed::DigitalInOut LCD_REG1;
extern mbed::DigitalInOut LCD_REG2;
extern mbed::DigitalInOut LCD_REG3;
extern mbed::DigitalInOut LCD_REG4;
extern mbed::DigitalInOut LCD_REG5;
extern mbed::DigitalInOut LCD_REG6;
extern mbed::DigitalInOut LCD_REG7;


extern mbed::DigitalOut LCD_NWR;
extern mbed::DigitalOut LCD_DC;
extern mbed::DigitalOut LCD_RESET;
extern mbed::DigitalOut LCD_NRD;

#ifndef DISPLAYINTERFACE_H
#define DISPLAYINTERFACE_H


#include "GUI_Type.h"
//#include "cycfg_pins.h"
 /*           "LCD_DATA_0":        "P9_0",
            "LCD_DATA_1":        "P9_1",
            "LCD_DATA_2":        "P9_2",
            "LCD_DATA_3":        "P9_4",
            "LCD_DATA_4":        "P9_5",
            "LCD_DATA_5":        "P0_2",
            "LCD_DATA_6":        "P13_0",
            "LCD_DATA_7":        "P13_1",
            "LCD_NWR":           "P12_0",
            "LCD_DC":            "P12_1",
            "LCD_RESET":         "P12_2",
            "LDC_NRD":           "P12_3",
  */


void DisplayIntf_Init(void);
void DisplayIntf_Write8_A0(U8 data);
void DisplayIntf_Write8_A1(U8 data);
void DisplayIntf_WriteM8_A1(U8 data[], int num);
U8 DisplayIntf_Read8_A1(void);
void DisplayIntf_ReadM8_A1(U8 data[], int num);

#endif

/* [] END OF FILE */


