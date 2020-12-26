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

#include "io2better.h"
#include "ota.h"
#include "web_server.h"
#include "wifi.h"
#include "http.h"
#include "config.h"
#include "input.h"

#include <ArduinoOTA.h>               // local OTA update from Arduino IDE
#include <ESP8266httpUpdate.h>        // remote OTA update from server
#include <ESP8266HTTPUpdateServer.h>  // upload update
#include <FS.h>

ESP8266HTTPUpdateServer httpUpdater;  // Create class for webupdate handleWebUpdate()
//#define CHECK_INTERVAL 3600
//#define CHECK_INTERVAL 1800
#define CHECK_INTERVAL 600


// -------------------------------------------------------------------
//OTA UPDATE SETTINGS
// -------------------------------------------------------------------
//UPDATE SERVER strings and interfers for upate server
// Array of strings Used to check firmware version

//String updateServer_fwImage;

//const char* updateServer = "http://iot2better.iptime.org:8003/fwImage/";
const char* fwImage = "firmware.bin";

const char* u_host = "http://thinkway.ipecsacademia.site:9000/";
const char* u_url = "/firmware_SIP.php";
const char* firmware_update_path = "/upload";

extern const char *esp_hostname;

Ticker updateCheck;
boolean doUpdateCheck = false;

void enableUpdateCheck() {
  doUpdateCheck = true;
}

void ota_setup()
{
  // Start local OTA update server
  ArduinoOTA.setHostname(esp_hostname);
  ArduinoOTA.begin();

  // Setup firmware upload
  httpUpdater.setup(&server, firmware_update_path);

  updateCheck.attach(CHECK_INTERVAL, enableUpdateCheck);

}

void ota_loop()
{
  if (WiFi.status() != WL_CONNECTED) {

  } else {
    ArduinoOTA.handle();
    int flashButtonState = digitalRead(0);
    if (flashButtonState == LOW || doUpdateCheck) {
      Serial.println("Going to update firmware...");
      ota_http_update();
      Serial.println("Process of update firmware...completed");
      doUpdateCheck = false;
    }
  }
}

String ota_get_latest_version()
{
  // Get latest firmware version number
  String url = u_url;
  return get_http(u_host, url);
}

t_httpUpdate_return ota_http_update()
{
  t_httpUpdate_return ret;
  Serial.println("WILL start ESP flash update");
  SPIFFS.end(); // unmount filesystem
  ESPhttpUpdate.rebootOnUpdate(BOOT_AFTER_UPDATE);

//  t_httpUpdate_return ret = ESPhttpUpdate.update("http://iot2better.iptime.org:9000/firmware_SPI.php?tag=" + currentfirmware);
  if((WiFi.macAddress() == "2C:3A:E8:08:E3:3D")||(WiFi.macAddress() == "5C:CF:7F:23:F1:36")){
    ret = ESPhttpUpdate.update("http://thinkway.ipecsacademia.site:9000/firmware_smartIO.php?tag=" + currentfirmware);
  }
  else{
    ret = ESPhttpUpdate.update("http://thinkway.ipecsacademia.site:9000/firmware_SPI.php?tag=" + currentfirmware);
  }

  SPIFFS.begin(); //mount-file system

  switch(ret) {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        Serial.println("ESP flash update Error");
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        Timer_1_SPIFFS();
        //ota_spiffs_update();

        //Serial.println("WILL reboot ESP system soon!!!!");
        do_reboot_exe();
        break;
	}

  return ret;
}

void do_reboot_exe()
{
	Serial.println("WILL reboot ESP system soon!!!!");
	pinMode(0, OUTPUT);
	digitalWrite(0, HIGH);
	delay(1000);

	pinMode(5, OUTPUT);
	digitalWrite(ESP_RESET_CTL, LOW);
	delay(100);
	digitalWrite(ESP_RESET_CTL, HIGH);
	Serial.println("digitalWrite(ESP_RESET_CTL!!!!");

	ESP.reset();
}
/*
void io2LIFEhttpUpdate(const char* updateServer, const char* fwImage)
{
  if (WiFi.status() == WL_CONNECTED) {
        updateServer_fwImage = updateServer;
        updateServer_fwImage += fwImage;

		String REMOTE_SERVER = updateServer;
		String SKETCH_BIN = fwImage;
}
*/
t_httpUpdate_return ota_spiffs_update()
{
		ESPhttpUpdate.rebootOnUpdate(BOOT_AFTER_UPDATE);
		//ESPhttpUpdate.rebootOnUpdate(true);

        Serial.println("WILL start SPIFFS update");
        t_httpUpdate_return ret_spiffs = ESPhttpUpdate.updateSpiffs("http://iot2better.iptime.org:9000/spiffs.php?tag=" + currentfirmware);

        switch(ret_spiffs) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("SPIFFS HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                Serial.println("SPIFFS update Error");
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("SPIFFS HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                Serial.println("SPIFFS HTTP_UPDATE_OK");
				        do_reboot_exe();
                break;
        }
        return ret_spiffs;
}
