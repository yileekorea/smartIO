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

#ifndef _IO2BETTER_H
#define _IO2BETTER_H
#include <Arduino.h>
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug


// -------------------------------------------------------------------
// General support code used by all modules
// -------------------------------------------------------------------

// Uncomment to use hardware UART 1 for debug else use UART 0
/*
#ifdef DEBUG_SERIAL1
#define DEBUG Serial1
#else
#define DEBUG Serial
#endif
*/

#define ESP_LED 2
#define ESP_RESET_CTL 16   //D0 pin for reset control


extern int numSensor;
extern unsigned long tempTry;
extern int heating_system_status;

extern unsigned long epochTime;
extern String formattedTime;

extern RemoteDebug Debug;

#endif // _IO2BETTER_H
