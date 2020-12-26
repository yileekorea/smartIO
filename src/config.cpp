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
#include "config.h"
#include "input.h"

#include <Arduino.h>
#include <EEPROM.h>                   // Save config settings

#include <FS.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

char config_Temp[8];

// Wifi Network Strings
String esid = "";
String epass = "";

// Web server authentication (leave blank for none)
String www_username = "";
String www_password = "";

// ESP-main-2 SERVER strings
String emoncms_server = "";
String emoncms_node = "";
String emoncms_apikey = "";
String emoncms_fingerprint = "";

// MQTT Settings
String mqtt_server = "";
String mqtt_topic = "";
String mqtt_user = "";
String mqtt_pass = "";
String mqtt_feed_prefix = "";


#define EEPROM_ESID_SIZE          32
#define EEPROM_EPASS_SIZE         64
#define EEPROM_EMON_API_KEY_SIZE  32
#define EEPROM_EMON_SERVER_SIZE   45
#define EEPROM_EMON_NODE_SIZE     32
#define EEPROM_MQTT_SERVER_SIZE   45
#define EEPROM_MQTT_TOPIC_SIZE    32
#define EEPROM_MQTT_USER_SIZE     32
#define EEPROM_MQTT_PASS_SIZE     64
#define EEPROM_EMON_FINGERPRINT_SIZE  60
#define EEPROM_MQTT_FEED_PREFIX_SIZE  10
#define EEPROM_WWW_USER_SIZE      16
#define EEPROM_WWW_PASS_SIZE      16
#define EEPROM_SIZE 512

#define EEPROM_ESID_START         0
#define EEPROM_ESID_END           (EEPROM_ESID_START + EEPROM_ESID_SIZE)
#define EEPROM_EPASS_START        EEPROM_ESID_END
#define EEPROM_EPASS_END          (EEPROM_EPASS_START + EEPROM_EPASS_SIZE)
#define EEPROM_EMON_API_KEY_START EEPROM_EPASS_END
#define EEPROM_EMON_API_KEY_END   (EEPROM_EMON_API_KEY_START + EEPROM_EMON_API_KEY_SIZE)
#define EEPROM_EMON_SERVER_START  EEPROM_EMON_API_KEY_END
#define EEPROM_EMON_SERVER_END    (EEPROM_EMON_SERVER_START + EEPROM_EMON_SERVER_SIZE)
#define EEPROM_EMON_NODE_START    EEPROM_EMON_SERVER_END
#define EEPROM_EMON_NODE_END      (EEPROM_EMON_NODE_START + EEPROM_EMON_NODE_SIZE)
#define EEPROM_MQTT_SERVER_START  EEPROM_EMON_NODE_END
#define EEPROM_MQTT_SERVER_END    (EEPROM_MQTT_SERVER_START + EEPROM_MQTT_SERVER_SIZE)
#define EEPROM_MQTT_TOPIC_START   EEPROM_MQTT_SERVER_END
#define EEPROM_MQTT_TOPIC_END     (EEPROM_MQTT_TOPIC_START + EEPROM_MQTT_TOPIC_SIZE)
#define EEPROM_MQTT_USER_START    EEPROM_MQTT_TOPIC_END
#define EEPROM_MQTT_USER_END      (EEPROM_MQTT_USER_START + EEPROM_MQTT_USER_SIZE)
#define EEPROM_MQTT_PASS_START    EEPROM_MQTT_USER_END
#define EEPROM_MQTT_PASS_END      (EEPROM_MQTT_PASS_START + EEPROM_MQTT_PASS_SIZE)
#define EEPROM_EMON_FINGERPRINT_START  EEPROM_MQTT_PASS_END
#define EEPROM_EMON_FINGERPRINT_END    (EEPROM_EMON_FINGERPRINT_START + EEPROM_EMON_FINGERPRINT_SIZE)
#define EEPROM_MQTT_FEED_PREFIX_START  EEPROM_EMON_FINGERPRINT_END
#define EEPROM_MQTT_FEED_PREFIX_END    (EEPROM_MQTT_FEED_PREFIX_START + EEPROM_MQTT_FEED_PREFIX_SIZE)
#define EEPROM_WWW_USER_START     EEPROM_MQTT_FEED_PREFIX_END
#define EEPROM_WWW_USER_END       (EEPROM_WWW_USER_START + EEPROM_WWW_USER_SIZE)
#define EEPROM_WWW_PASS_START     EEPROM_WWW_USER_END
#define EEPROM_WWW_PASS_END       (EEPROM_WWW_PASS_START + EEPROM_WWW_PASS_SIZE)

void accHistory2SPIFFS()
{
  if (SPIFFS.begin())
  {
   Serial.println("Accumulate History json save...");
    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject& json = jsonBuffer.createObject();
    String accHistory = "accHistory";
    //for (byte i = 1; i < numSensor; ++i)
    {
      //String roomNo = "";
      //roomNo = "room_";
      //roomNo += i;
      json[accHistory] = accCountValue;
    }

    File configFile = SPIFFS.open("/accHistory.json", "w");
    if (!configFile) {
      Serial.println("failed to open accHistory file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
}

void SPIFFS2accHistory()
{

  Serial.println("Before .... mounted file system SPIFFS2accHistory");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system for SPIFFS2accHistory");
    if (SPIFFS.exists("/accHistory.json"))
    {
      //file exists, reading and loading
      Serial.println("reading accHistory file");
      File configFile = SPIFFS.open("/accHistory.json", "r");
      if (configFile) {
        Serial.println("opened accHistory file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed accHistory json");

          String accHistory = "accHistory";
          //for (byte i = 1; i < numSensor; ++i)
          {
            //String roomNo = "";
            //roomNo = "room_";
            //roomNo += i;
            //L_Temp[i-1] = atof(strcpy(config_Temp, json[roomNo]));
            //Serial.println(L_Temp[i-1]);
      			//if((json[accHistory]))
            {
      				accCountValue = atof(json[accHistory]);
      				Serial.println(accCountValue);
      			}
          } //for

        } //if (json.success())
        else {
          Serial.println("failed to load json accCountValue");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS for accCountValue");
  }
}

// -------------------------------------------------------------------
//save the system status parameters to FS
// -------------------------------------------------------------------
void systemSTATUS2SPIFFS()
{
  if (SPIFFS.begin())
  {
   Serial.println("system status History json save...");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    String systemSTATUS = "systemSTATUS";
    {

      json[systemSTATUS] = heating_system_status;
    }

    File configFile = SPIFFS.open("/systemSTATUS.json", "w");
    if (!configFile) {
      Serial.println("failed to open systemSTATUS file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
}


void SPIFFS2systemSTATUS()
{

  Serial.println("Before .... mounted file system SPIFFS2systemSTATUS");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system for SPIFFS2systemSTATUS");
    if (SPIFFS.exists("/systemSTATUS.json"))
    {
      //file exists, reading and loading
      Serial.println("reading systemSTATUS file");
      File configFile = SPIFFS.open("/systemSTATUS.json", "r");
      if (configFile) {
        Serial.println("opened systemSTATUS file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed systemSTATUS json");

          String systemSTATUS = "systemSTATUS";
          {
		{
			heating_system_status = atof(json[systemSTATUS]);
			Serial.println(systemSTATUS);
		}
          }
        }
        else {
          Serial.println("failed to load json systemSTATUS");
        }
      }
    } //if (SPIFFS.exists("/systemSTATUS.json"))
    else{
	heating_system_status = 1;
    }

  } else { //if (SPIFFS.begin()) {
    Serial.println("failed to mount FS for systemSTATUS");
  }
}


// -------------------------------------------------------------------
//save the Timer_1[] parameters to FS
// -------------------------------------------------------------------
void Timer_1_SPIFFS()
{
  if (SPIFFS.begin())
  {
   Serial.println("ESP8266 Timer_1 to json save...");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    String timerNo = "Timer_1_";
    for (byte i = 1; i < (numSensor+1); ++i) {
      String timerNo = "";
      timerNo = "Timer_1_";
      timerNo += i;
      json[timerNo] = Timer_1[i-1];
    }

    File configFile = SPIFFS.open("/Timer_1.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
}

// -------------------------------------------------------------------
//save the Timer_1 parameters to FS
// -------------------------------------------------------------------
void SPIFFS_Timer_1()
{
  Serial.println("Before .... mounted file system-SPIFFS_Timer_1");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system for SPIFFS_Timer_1");
    if (SPIFFS.exists("/Timer_1.json"))
    {
      //file exists, reading and loading
      Serial.println("reading config file-Timer_1.json");
      File configFile = SPIFFS.open("/Timer_1.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          String timerNo = "Timer_1_";
          for (byte i = 1; i < (numSensor+1); ++i) {
            String timerNo = "";
            timerNo = "Timer_1_";
            timerNo += i;
            //L_Temp[i-1] = atof(strcpy(config_Temp, json[roomNo]));
            //Serial.println(L_Temp[i-1]);
      			if((json[timerNo])) {
      				Timer_1[i-1] = atof(json[timerNo]);
      				Serial.println(Timer_1[i-1]);
      			}
      			else {
      				break;
      			}
          } //for

        } //if (json.success())
        else {
          Serial.println("failed to load Timer_1.json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS of Timer_1.json");
  }
}

// -------------------------------------------------------------------
//save the custom parameters to FS
// -------------------------------------------------------------------
void L_Temp2SPIFFS()
{
  if (SPIFFS.begin())
  {
   Serial.println("ESP8266 config L_temp.json save...");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    String roomNo = "room_";
    for (byte i = 1; i < (numSensor+1); ++i) {
      String roomNo = "";
      roomNo = "room_";
      roomNo += i;
      json[roomNo] = L_Temp[i-1];
    }

    File configFile = SPIFFS.open("/L_temp.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
}

// -------------------------------------------------------------------
//save the custom parameters to FS
// -------------------------------------------------------------------
void SPIFFS2L_Temp()
{
  Serial.println("Before .... mounted file system");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system for SPIFFS2L_Temp");
    if (SPIFFS.exists("/L_temp.json"))
    {
      //file exists, reading and loading
      Serial.println("reading config file L_temp.json");
      File configFile = SPIFFS.open("/L_temp.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          String roomNo = "room_";
          for (byte i = 1; i < (numSensor+1); ++i) {
            String roomNo = "";
            roomNo = "room_";
            roomNo += i;
            //L_Temp[i-1] = atof(strcpy(config_Temp, json[roomNo]));
            //Serial.println(L_Temp[i-1]);
      			if((json[roomNo])) {
      				L_Temp[i-1] = atof(json[roomNo]);
      				Serial.println(L_Temp[i-1]);
      			}
      			else {
      				break;
      			}
          } //for

        } //if (json.success())
        else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

// -------------------------------------------------------------------
// Reset EEPROM, wipes all settings
// -------------------------------------------------------------------
void ResetEEPROM(){
  //Serial.println("Erasing EEPROM");
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    EEPROM.write(i, 0);
    //Serial.print("#");
  }
  EEPROM.commit();
}

void EEPROM_read_srting(int start, int count, String& val) {
  for (int i = 0; i < count; ++i){
    byte c = EEPROM.read(start+i);
    if (c!=0 && c!=255) val += (char) c;
  }
}

void EEPROM_write_string(int start, int count, String val) {
  for (int i = 0; i < count; ++i){
    if (i<val.length()) {
      EEPROM.write(start+i, val[i]);
    } else {
      EEPROM.write(start+i, 0);
    }
  }
}

// -------------------------------------------------------------------
// Load saved settings from EEPROM
// -------------------------------------------------------------------
void config_load_settings()
{
  //SPIFFS2L_Temp();
  EEPROM.begin(EEPROM_SIZE);

  // Load WiFi values
  EEPROM_read_srting(EEPROM_ESID_START, EEPROM_ESID_SIZE, esid);
  EEPROM_read_srting(EEPROM_EPASS_START, EEPROM_EPASS_SIZE, epass);
/*
  // ESP-main-2 settings
  EEPROM_read_srting(EEPROM_EMON_API_KEY_START, EEPROM_EMON_API_KEY_SIZE, emoncms_apikey);
  EEPROM_read_srting(EEPROM_EMON_SERVER_START, EEPROM_EMON_SERVER_SIZE, emoncms_server);
  EEPROM_read_srting(EEPROM_EMON_NODE_START, EEPROM_EMON_NODE_SIZE, emoncms_node);
  EEPROM_read_srting(EEPROM_EMON_FINGERPRINT_START, EEPROM_EMON_FINGERPRINT_SIZE, emoncms_fingerprint);
*/
  // MQTT settings
  EEPROM_read_srting(EEPROM_MQTT_SERVER_START, EEPROM_MQTT_SERVER_SIZE, mqtt_server);
  EEPROM_read_srting(EEPROM_MQTT_TOPIC_START, EEPROM_MQTT_TOPIC_SIZE, mqtt_topic);
  EEPROM_read_srting(EEPROM_MQTT_FEED_PREFIX_START, EEPROM_MQTT_FEED_PREFIX_SIZE, mqtt_feed_prefix);
  EEPROM_read_srting(EEPROM_MQTT_USER_START, EEPROM_MQTT_USER_SIZE, mqtt_user);
  EEPROM_read_srting(EEPROM_MQTT_PASS_START, EEPROM_MQTT_PASS_SIZE, mqtt_pass);

  // Web server credentials
  EEPROM_read_srting(EEPROM_WWW_USER_START, EEPROM_WWW_USER_SIZE, www_username);
  EEPROM_read_srting(EEPROM_WWW_PASS_START, EEPROM_WWW_PASS_SIZE, www_password);
}

void config_save_emoncms(String server, String node, String apikey, String fingerprint)
{
  emoncms_server = server;
  emoncms_node = node;
  emoncms_apikey = apikey;
  emoncms_fingerprint = fingerprint;

  // save apikey to EEPROM
  EEPROM_write_string(EEPROM_EMON_API_KEY_START, EEPROM_EMON_API_KEY_SIZE, emoncms_apikey);

  // save ESP-main-2 server to EEPROM max 45 characters
  EEPROM_write_string(EEPROM_EMON_SERVER_START, EEPROM_EMON_SERVER_SIZE, emoncms_server);

  // save ESP-main-2 node to EEPROM max 32 characters
  EEPROM_write_string(EEPROM_EMON_NODE_START, EEPROM_EMON_NODE_SIZE, emoncms_node);

  // save ESP-main-2 HTTPS fingerprint to EEPROM max 60 characters
  EEPROM_write_string(EEPROM_EMON_FINGERPRINT_START, EEPROM_EMON_FINGERPRINT_SIZE, emoncms_fingerprint);

  EEPROM.commit();
}

void config_save_mqtt(String server, String topic, String prefix, String user, String pass)
{
  mqtt_server = server;
  mqtt_topic = topic;
  mqtt_feed_prefix = prefix;
  mqtt_user = user;
  mqtt_pass = pass;

  // Save MQTT server max 45 characters
  EEPROM_write_string(EEPROM_MQTT_SERVER_START, EEPROM_MQTT_SERVER_SIZE, mqtt_server);

  // Save MQTT topic max 32 characters
  EEPROM_write_string(EEPROM_MQTT_TOPIC_START, EEPROM_MQTT_TOPIC_SIZE, mqtt_topic);

  // Save MQTT topic separator max 10 characters
  EEPROM_write_string(EEPROM_MQTT_FEED_PREFIX_START, EEPROM_MQTT_FEED_PREFIX_SIZE, mqtt_feed_prefix);

  // Save MQTT username max 32 characters
  EEPROM_write_string(EEPROM_MQTT_USER_START, EEPROM_MQTT_USER_SIZE, mqtt_user);

  // Save MQTT pass max 64 characters
  EEPROM_write_string(EEPROM_MQTT_PASS_START, EEPROM_MQTT_PASS_SIZE, mqtt_pass);

  EEPROM.commit();
}

void config_save_admin(String user, String pass)
{
  www_username = user;
  www_password = pass;

  EEPROM_write_string(EEPROM_WWW_USER_START, EEPROM_WWW_USER_SIZE, user);
  EEPROM_write_string(EEPROM_WWW_PASS_START, EEPROM_WWW_PASS_SIZE, pass);

  EEPROM.commit();
}

void config_save_wifi(String qsid, String qpass)
{
  esid = qsid;
  epass = qpass;

  EEPROM_write_string(EEPROM_ESID_START, EEPROM_ESID_SIZE, qsid);
  EEPROM_write_string(EEPROM_EPASS_START, EEPROM_EPASS_SIZE, qpass);

  EEPROM.commit();
}

void config_reset()
{
  ResetEEPROM();
}
