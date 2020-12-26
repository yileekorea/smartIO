/*
 * -------------------------------------------------------------------
 * ESP-main Serial to ESP-main-2 gateway
 * -------------------------------------------------------------------
 * Adaptation of Chris Howells OpenEVSE ESP Wifi
 * by Trystan Lea, Glyn Hudson, OpenEnergyMonitor
 * All adaptation GNU General Public License as below.
 *
 * -------------------------------------------------------------------
 *
 * This file is part of ESP-main-web project.
 * ESP-main is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * ESP-main is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with ESP-main; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _EMONESP_INPUT_H
#define _EMONESP_INPUT_H

#include <Arduino.h>
#define interOFF_Timer_15 900000UL //15min = 60000UL x 15 -> 900000UL
#define interOFF_Timer_20 1200000UL //20min = 60000UL x 20 -> 1200000UL
#define interOFF_Timer_30 1800000UL //30min = 60UL x 30 -> 1800UL

//#define interOpenTimer 300000UL //5min = 60000UL x 5 -> 300000UL
#define interOpenTimer 300000UL //5min = 60UL x 5 -> 300UL

// -------------------------------------------------------------------
// Support for reading input
// -------------------------------------------------------------------

extern String last_datastr;
extern String input_string;
//extern byte numberofSensors;
extern String sName[];
extern float old_celsius[];
extern float celsius[];
extern float old_rStatus[];
extern float rStatus[];
extern float L_Temp[];
extern byte address[9][8];
extern int autoOff_OnTimer;
extern byte isOFF[];
extern unsigned long Timer_1[];
extern unsigned long Timer_2[];

extern float accCountValue;
extern volatile byte INTstateHistory;

//extern String testMacaddress = "5C:CF:7F:23:F1:36";	//"5C:CF:7F:23:F1:36";
//extern String yileeMacaddress = "2C:3A:E8:08:E3:3D";	//"2C:3A:E8:08:E3:3D";

// -------------------------------------------------------------------
// Read input sent via the web_server or serial.
//
// data: if true is returned data will be updated with the new line of
//       input
// -------------------------------------------------------------------
void accCount();
extern void INTsetup();

extern boolean input_get(String& data);
extern void setON_OFFstatus(byte Sensor);

extern String readFromOneWire();
extern void readOneWireAddr();
extern void readoutTemperature(byte Sensor);
extern void measureTemperature(byte Sensor);




#endif // _EMONESP_INPUT_H
