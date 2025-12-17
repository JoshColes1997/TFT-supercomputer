/***************************************************************************//**
* \file DisplayInterface.h
* \version 1.0
*
* \brief
* Objective:
*    This is display software i8080 interface source file
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


#include "cy8ckit_028_tft.h"
#include <mbed_wait_api.h>
#include "mbed.h"
#include "PortInOut.h"


DigitalInOut LCD_REG0(P9_0);
DigitalInOut LCD_REG1(P9_1);
DigitalInOut LCD_REG2(P9_2);
DigitalInOut LCD_REG3(P9_4);
DigitalInOut LCD_REG4(P9_5);
DigitalInOut LCD_REG5(P0_2);
DigitalInOut LCD_REG6(P13_0);
DigitalInOut LCD_REG7(P13_1);
PortInOut P0(Port0, 0x04);
PortInOut P9(Port9, 0x37);
PortInOut P13(Port13, 0x03);

DigitalOut LCD_NWR(P12_0);
DigitalOut LCD_DC(P12_1);
DigitalOut LCD_RESET(P12_2);
DigitalOut LCD_NRD(P12_3);

/*******************************************************************************
* Function Name: DataWrite
****************************************************************************/
/**
*
* \brief
*   Writes one byte of data to the software i8080 interface.
*
* \details
*   This function:
*       - Writes data to the data bus
*       - Sends low pulse to the LCD_NWR line to write data
*
*   Changed from individual bit banging to port masked writes to 
*   P9[5,4,2,1,0], P13[1,0], P0[2] to optimise slightly 
* 
* \todo
*   All this should be replaced with a udb register to save all the shifting
*   and individual bit writing.
*
*******************************************************************************/
void DataWrite(U8 data)
{
//    LCD_REG0 = (data & 0x01);
//    LCD_REG1 = ((data>>1) & 0x01);
//    LCD_REG2 = ((data>>2) & 0x01);
//    LCD_REG3 = ((data>>3) & 0x01);
//    LCD_REG4 = ((data>>4) & 0x01);

/* read the appropriate port and only change the bits we need to then write the
 * affected bits back to the port retaining any unaffected bit values
 */
    int pbyte = P9.read();
    int bit012 = (data & 0x07);
    int bit34 = (data & 0x18) << 1;
    pbyte = (pbyte & 0xc8) | bit34 | bit012;
    P9.write(pbyte);
//    LCD_REG5 = ((data>>5) & 0x01);
    pbyte = P0.read();
    int bit5 = (data & 0x20) >> 3 ;
    pbyte = (pbyte & 0xfb) | bit5 ;
    P0.write(pbyte);

//    LCD_REG6 = ((data>>6) & 0x01);
//    LCD_REG7 = ((data>>7) & 0x01);
    pbyte = P13.read();
    int bit67 = (data & 0xc0) >> 6 ;
    pbyte = (pbyte & 0xfc) | bit67 ;
    P13.write(pbyte);
    LCD_NWR = 0u;
    LCD_NWR = 1u;

}


/*******************************************************************************
* Function Name: DataRead
****************************************************************************//**
*
* \brief
*   Reads one byte of data from the software i8080 interface.
*
* \details
*   This function:
*       - Changes data bus GPIO pins drive mode to digital Hi-Z with enabled input 
*         buffer
*       - Sends low pulse to LCD_NRD line to read data
*       - Reads data from the data bus
*       - Sends low pulse to the LCD_NWR line to write data
*       - Changes data bus GPIO pins drive mode back to to Strong Drive mode
* 
* \todo
*   All this should be replaced with a udb register to save all the shifting
*   and individual bit reading.
*
*******************************************************************************/
U8 DataRead(void)
{
    U8 data = 0u;

    /* enable input */
    LCD_REG0.input();
    LCD_REG1.input();
    LCD_REG2.input();
    LCD_REG3.input();
    LCD_REG4.input();
    LCD_REG5.input();
    LCD_REG6.input();
    LCD_REG7.input();

    LCD_NRD = 0u;  //  Pulse read line low then read the data port
    
    data = (U8)LCD_REG0.read();
    data |= (U8)LCD_REG1.read()<<1;
    data |= (U8)LCD_REG2.read()<<2;
    data |= (U8)LCD_REG3.read()<<3;
    data |= (U8)LCD_REG4.read()<<4;
    data |= (U8)LCD_REG5.read()<<5;
    data |= (U8)LCD_REG6.read()<<6;
    data |= (U8)LCD_REG7.read()<<7;
    
    LCD_NRD = 1u;  // Raise the read line and then go back to output port
       
    LCD_REG0.output();
    LCD_REG1.output();
    LCD_REG2.output();
    LCD_REG3.output();
    LCD_REG4.output();
    LCD_REG5.output();
    LCD_REG6.output();
    LCD_REG7.output();

    return data;
}


/*******************************************************************************
* Function Name: DisplayIntf_Init
****************************************************************************//**
*
* \brief
*   Initializes software i8080 interface.
*
* \details
*   This function:
*       - Initializes interface GPIO pins
*
*******************************************************************************/
void DisplayIntf_Init(void)
{
    /* All pins are initialized by the Device Configurator. */
    LCD_RESET = 1u;
    LCD_NRD = 1u;
    LCD_NWR = 1u;
    LCD_DC = 0u;
    LCD_REG0.output();
    LCD_REG1.output();
    LCD_REG2.output();
    LCD_REG3.output();
    LCD_REG4.output();
    LCD_REG5.output();
    LCD_REG6.output();
    LCD_REG7.output();
    wait_ms(20);
    LCD_RESET = 0u;
    wait_ms(100);
    
    LCD_RESET = 1u;
    wait_ms(100);

    DisplayIntf_Write8_A0(0x28);
    DisplayIntf_Write8_A0(0x11);    /* Exit Sleep mode */
    wait_ms(100);
    DisplayIntf_Write8_A0(0x36);
    DisplayIntf_Write8_A1(0xA0);    /* MADCTL: memory data access control */
    DisplayIntf_Write8_A0(0x3A);
    DisplayIntf_Write8_A1(0x65);    /* COLMOD: Interface Pixel format */
    DisplayIntf_Write8_A0(0xB2);
    DisplayIntf_Write8_A1(0x0C);
    DisplayIntf_Write8_A1(0x0C);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0x33);
    DisplayIntf_Write8_A1(0x33);    /* PORCTRK: Porch setting */
    DisplayIntf_Write8_A0(0xB7);
    DisplayIntf_Write8_A1(0x35);    /* GCTRL: Gate Control */
    DisplayIntf_Write8_A0(0xBB);
    DisplayIntf_Write8_A1(0x2B);    /* VCOMS: VCOM setting */
    DisplayIntf_Write8_A0(0xC0);
    DisplayIntf_Write8_A1(0x2C);    /* LCMCTRL: LCM Control */
    DisplayIntf_Write8_A0(0xC2);
    DisplayIntf_Write8_A1(0x01);
    DisplayIntf_Write8_A1(0xFF);    /* VDVVRHEN: VDV and VRH Command Enable */
    DisplayIntf_Write8_A0(0xC3);
    DisplayIntf_Write8_A1(0x11);    /* VRHS: VRH Set */
    DisplayIntf_Write8_A0(0xC4);
    DisplayIntf_Write8_A1(0x20);    /* VDVS: VDV Set */
    DisplayIntf_Write8_A0(0xC6);
    DisplayIntf_Write8_A1(0x0F);    /* FRCTRL2: Frame Rate control in normal mode */
    DisplayIntf_Write8_A0(0xD0);
    DisplayIntf_Write8_A1(0xA4);
    DisplayIntf_Write8_A1(0xA1);    /* PWCTRL1: Power Control 1 */
    DisplayIntf_Write8_A0(0xE0);
    DisplayIntf_Write8_A1(0xD0);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0x05);
    DisplayIntf_Write8_A1(0x0E);
    DisplayIntf_Write8_A1(0x15);
    DisplayIntf_Write8_A1(0x0D);
    DisplayIntf_Write8_A1(0x37);
    DisplayIntf_Write8_A1(0x43);
    DisplayIntf_Write8_A1(0x47);
    DisplayIntf_Write8_A1(0x09);
    DisplayIntf_Write8_A1(0x15);
    DisplayIntf_Write8_A1(0x12);
    DisplayIntf_Write8_A1(0x16);
    DisplayIntf_Write8_A1(0x19);    /* PVGAMCTRL: Positive Voltage Gamma control */
    DisplayIntf_Write8_A0(0xE1);
    DisplayIntf_Write8_A1(0xD0);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0x05);
    DisplayIntf_Write8_A1(0x0D);
    DisplayIntf_Write8_A1(0x0C);
    DisplayIntf_Write8_A1(0x06);
    DisplayIntf_Write8_A1(0x2D);
    DisplayIntf_Write8_A1(0x44);
    DisplayIntf_Write8_A1(0x40);
    DisplayIntf_Write8_A1(0x0E);
    DisplayIntf_Write8_A1(0x1C);
    DisplayIntf_Write8_A1(0x18);
    DisplayIntf_Write8_A1(0x16);
    DisplayIntf_Write8_A1(0x19);    /* NVGAMCTRL: Negative Voltage Gamma control */
    DisplayIntf_Write8_A0(0x2B);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0xEF);    /* Y address set */
    DisplayIntf_Write8_A0(0x2A);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0x00);
    DisplayIntf_Write8_A1(0x01);
    DisplayIntf_Write8_A1(0x3F);    /* X address set */
    wait_ms(10);
    DisplayIntf_Write8_A0(0x29);


}


/*******************************************************************************
* Function Name: DisplayIntf_Write8_A0
****************************************************************************//**
*
* \brief
*   Writes one byte of data to the software i8080 interface with the LCD_DC pin 
*   set to 0
*
* \details
*   This function:
*       - Sets LCD_DC pin to 0
*       - Writes one data byte
*
*******************************************************************************/
void DisplayIntf_Write8_A0(U8 data)
{
    LCD_DC = 0u;
    DataWrite(data);
}


/*******************************************************************************
* Function Name: DisplayIntf_Write8_A1
****************************************************************************//**
*
* \brief
*   Writes one byte of data to the software i8080 interface with the LCD_DC pin 
*   set to 1
*
* \details
*   This function:
*       - Sets LCD_DC pin to 1
*       - Writes one data byte
*
*******************************************************************************/
void DisplayIntf_Write8_A1(U8 data)
{
    LCD_DC = 1u;
    DataWrite(data);
}


/*******************************************************************************
* Function Name: DisplayIntf_WriteM8_A1
****************************************************************************//**
*
* \brief
*   Writes multiple bytes of data to the software i8080 interface with the LCD_DC 
*   pin set to 1
*
* \details
*   This function:
*       - Sets LCD_DC pin to 1
*       - Writes data bytes
*
*******************************************************************************/
void DisplayIntf_WriteM8_A1(U8 data[], int num)
{
    int i = 0;

    LCD_DC = 1u;

    for(i = 0; i < num; i++)
    {
        DataWrite(data[i]);
    }
}


/*******************************************************************************
* Function Name: DisplayIntf_Read8_A1
****************************************************************************//**
*
* \brief
*   Reads one byte of data from the software i8080 interface with the LCD_DC pin 
*   set to 1
*
* \details
*   This function:
*       - Sets LCD_DC pin to 1
*       - Reads one data byte
*
*******************************************************************************/
U8 DisplayIntf_Read8_A1(void)
{
    LCD_DC = 1u;
    return DataRead();
}


/*******************************************************************************
* Function Name: DisplayIntf_ReadM8_A1
****************************************************************************//**
*
* \brief
*   Reads multiple bytes of data from the software i8080 interface with the LCD_DC 
*   pin set to 1
*
* \details
*   This function:
*       - Sets LCD_DC pin to 1
*       - Reads data bytes
*
*******************************************************************************/
void DisplayIntf_ReadM8_A1(U8 data[], int num)
{
    int i = 0;

    LCD_DC = 1u;

    for(i = 0; i < num; i++)
    {
        data[i] = DataRead();
    }
}


/* [] END OF FILE */


