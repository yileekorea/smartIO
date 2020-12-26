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

#ifndef _EMONESP_OTA_H
#define _EMONESP_OTA_H

#define BOOT_AFTER_UPDATE false

// -------------------------------------------------------------------
// Support for updating the fitmware os the ESP8266
// -------------------------------------------------------------------

#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <Ticker.h>

extern const char* updateServer;
extern const char* fwImage;
extern String updateServer_fwImage;


void ota_setup();
void ota_loop();
String ota_get_latest_version();
t_httpUpdate_return ota_http_update();
t_httpUpdate_return ota_spiffs_update();

void do_reboot_exe();

void io2LIFEhttpUpdate(const char* updateServer, const char* fwImage);

#endif // _EMONESP_OTA_H
