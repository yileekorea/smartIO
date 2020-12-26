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
#include "mqtt.h"
#include "config.h"
#include "input.h"
#include "ota.h"
#include "web_server.h"

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <PubSubClient.h>             // MQTT https://github.com/knolleary/pubsubclient PlatformIO lib: 89
//#include <WiFiClient.h>
#include <WiFiClientSecure.h>


//sudo openssl x509 -in io2better.net.crt -sha1 -noout -fingerprint

//const char* fingerprint = "E9:7E:7C:E4:53:98:D7:B3:A4:82:5B:87:29:D5:AE:F2:91:83:F6:FB"; //mqtt.io2life.com 20190512

//ubuntu@ip-172-26-7-181:/etc/mosquitto/conf.d$ sudo openssl x509 -in /etc/letsencrypt/live/mqtt.ipecsacademia.site/cert.pem -sha1 -noout -fingerprint
//SHA1 Fingerprint=00:74:9E:CB:E2:6E:91:31:0A:DD:83:0C:92:0D:A2:B1:EF:04:BE:85
const char* fingerprint = "00:74:9E:CB:E2:6E:91:31:0A:DD:83:0C:92:0D:A2:B1:EF:04:BE:85"; //mqtt.ipecsacademia.site fingerprint 20201110

long lastMqttReconnectAttempt = 0;
int clientTimeout = 0;
int i = 0;
int MQTT_PORT = 8883;

const char* do_web_update = "/do_web_update";
const char* do_fw_update = "/do_fw_update";
const char* do_reboot = "/do_reboot";
const char* do_erase_all = "/do_erase_all";
const char* do_format = "/do_format";

String topic_sub = "/S_";
String topic_pub = "/P_";
String MAC;
String strID = String(WiFi.macAddress());
String exec_command = "";
byte userTempset = 1;
byte initSending = 10;


//WiFiClient espClient;                 // Create client for MQTT
WiFiClientSecure espClient;                 // Create client for MQTT
//PubSubClient mqttclient(espClient);   // Create client for MQTT
PubSubClient mqttclient(mqtt_server.c_str(), MQTT_PORT, mqttCallback, espClient);

// -------------------------------------------------------------------
// MQTT exec_save_command
// -------------------------------------------------------------------
void exec_save_command(String exec)
{
  exec_command = exec;
  if (exec_command == do_fw_update) {
    Serial.println(">> do_ots_update_exe ");
    ota_http_update();
    mqtt_restart();
  }
  else if (exec_command == do_web_update) {
    Serial.println(">> do_web_update_exe ");
    ota_spiffs_update();
    mqtt_restart();
  }
  else if (exec_command == do_reboot) {
    Serial.println(">> do_reboot_exe ");
    do_reboot_exe();
  }
  else if (exec_command == do_format) {
    Serial.println(">> do_format_exe:SPIFFS.format() ");
    SPIFFS.format();
    do_reboot_exe();
  }
  else if (exec_command == do_erase_all) {
    Serial.println(">> do_erase_all_exec");
    config_reset();
    do_reboot_exe();
  }

}
// -------------------------------------------------------------------
// MQTT reconnect
// -------------------------------------------------------------------
boolean reconnect() {
  byte i=0;
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    Serial.print("Attempting MQTT RE-connection...");

    // Attempt to connect
    if (mqttclient.connect(strID.c_str(), mqtt_user.c_str(), mqtt_pass.c_str())) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        if (mqttclient.publish((char *)topic_pub.c_str(), "hello world againII...")) {
            Serial.println("publish ok2");
        }
        else {
            Serial.println("publish failed2");
        }
        // ... and resubscribe
        if (mqttclient.subscribe((char *)topic_sub.c_str())) {
            Serial.println("Subscribe ok2");
        }
        else {
            Serial.println("Subscribe failed2");
        }

    } else {
        Serial.print("failed, rc=");
        Serial.print(mqttclient.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
        i ++;
        if (i > 10){
           mqttclient.disconnect();
           do_reboot_exe();
           break;
         }
    }
  //server.handleClient();	// checks for WEB incoming messages
  }
    return mqttclient.connected();
}
// -------------------------------------------------------------------
// MQTT send A TempData
// -------------------------------------------------------------------
void send_a_TempData(byte Sensor) {
    byte i = Sensor;
    char pChrBuffer[5];
    char pChrBuffer_acc[8];

    //for ( i = 0; i < numSensor ; i++) {
    //if((abs(old_celsius[i] - celsius[i]) > 0.1) || (old_rStatus[i] != rStatus[i])) { //temp. difference is...
    //if((abs(old_celsius[i] - celsius[i]) > 0.0) || (old_rStatus[i] != rStatus[i]))
    if(((old_celsius[i] != celsius[i]) || (old_rStatus[i] != rStatus[i])) && (85 != celsius[i]))
    { //temp. difference is...

//      char pChrBuffer[5];
//			char pChrBuffer_acc[8];

			String payload = "{\"tbl_name\":";
			payload += "\"";
			payload += MAC;
			payload += "\"";

			payload += ",\"id\":";
			payload += i+1;   // id

			payload += ",\"numSensor\":";
			payload += numSensor;   // numSensor

			payload += ",\"cTemps\":";
			if ( isnan(celsius[i]) )
				payload += "0";
			else {
				dtostrf(celsius[i] , 3, 1, pChrBuffer);
				payload += pChrBuffer;   // *C
			}
/*
			payload += ",\"sName\":";
			payload += "\"";
			payload += sName[i];   // sensor name
			payload += "\"";
*/
			payload += ",\"cStatus\":";
			payload += rStatus[i];   // room status


      payload += ",\"cCount\":";
      BuildVersion();
      if(buildVersion !="")
        payload += buildVersion;
      else
        payload += "1969";

/*
			if ( isnan(accCountValue) )
				payload += "0";
			else {
				dtostrf(accCountValue , 7, 2, pChrBuffer_acc);
				payload += pChrBuffer_acc;
			}
*/
      payload += ",\"hOF\":";
			payload += heating_system_status;   // heating_system_status ON_OFF

			payload += "}";

			sendmqttMsg((char *)topic_pub.c_str(), (char *)payload.c_str());

			old_celsius[i] = celsius[i];
		}
    else
    {
        Serial.print("NOT send a TempData : ");
        Serial.print(i);
        Serial.print(",  Temps = ");
        Serial.print(celsius[i]);
        Serial.println(" 'C");
/*
  			String payload = "{\"tbl_name\":";
  			payload += "\"";
  			payload += MAC;
  			payload += "\"";

  			payload += ",\"id\":";
  			payload += i+1;   // id

        payload += ",\"Timer_1\":";
  			dtostrf((epochTime-Timer_1[i]) , 4, 1, pChrBuffer_acc);
  			payload += pChrBuffer_acc;   // *C

  			payload += "}";

  			sendmqttMsg((char *)topic_pub.c_str(), (char *)payload.c_str());
*/
    }
		//old_celsius[i] = celsius[i];
		old_rStatus[i] = rStatus[i];

	//} //numSensor
}

// -------------------------------------------------------------------
// MQTT sendTempData
// -------------------------------------------------------------------
void sendTempData() {
    byte i;
    Serial.print("sendTempData numSensor : ");
    Serial.println(numSensor);

    for ( i = 0; i < numSensor ; i++) {
      send_a_TempData(i);
      delay(50);
/*
		//if(old_celsius[i] != celsius[i]){
    //if(abs(old_celsius[i] - celsius[i]) > 0.1 )
    {

			char pChrBuffer[5];
			String payload = "{\"tbl_name\":";
			payload += "\"";
			payload += MAC;
			payload += "\"";

			payload += ",\"id\":";
			payload += i+1;   // id

			payload += ",\"numSensor\":";
			payload += numSensor;   // numSensor

			payload += ",\"cTemps\":";
			if ( isnan(celsius[i]) )
				payload += "0";
			else {
				dtostrf(celsius[i] , 3, 1, pChrBuffer);
				payload += pChrBuffer;   // *C
			}

			payload += ",\"sName\":";
			payload += "\"";
			payload += sName[i];   // sensor name
			payload += "\"";

			payload += ",\"cStatus\":";
			payload += rStatus[i];   // room status
			payload += "}";

			sendmqttMsg((char *)topic_pub.c_str(), (char *)payload.c_str());
      old_celsius[i] = celsius[i];

		} //if(abs(old_celsius[i] - celsius[i]) > 0.1 )
		sName[i] = "";
		//old_celsius[i] = celsius[i];
    */

	} //for ( i = 0; i < numSensor...
}
// -------------------------------------------------------------------
// MQTT sendmqttMsg sending
// -------------------------------------------------------------------
void sendmqttMsg(char* topictosend, String payload)
{

  if (mqttclient.connected()) {
    if (DEBUG_PRINT) {
      Serial.println();
      Serial.print("Sending payload: ");
      Serial.print(payload);
    }

    unsigned int msg_length = payload.length();

    byte* p = (byte*)malloc(msg_length);
    memcpy(p, (char*) payload.c_str(), msg_length);

    if (DEBUG_PRINT) {
      Serial.print(" length: ");
      Serial.println(msg_length);
    }

    if (mqttclient.publish(topictosend, p, msg_length)) {
      Serial.println("Publish length TEMP ok");
      free(p);
      //return 1;
    }
    else {
      free(p);
      Serial.println("Publish length TEMP failed,,, trying mqtt REBOOT");
      Timer_1_SPIFFS();
      //mqtt_restart(); // try to disconnect
      //delay(5000);
      do_reboot_exe();
      //return 0;
    }
  }
}

// -------------------------------------------------------------------
// MQTT mqttCallback
// -------------------------------------------------------------------
void mqttCallback(char* topic_sub, byte* payload, unsigned int length)
{
	int i = 0;
  int r_Sensor;
	int r_hOF; //heatingON_OFF

    char buffer[80];
	  float tmp_accCountValue;

    int len = length >= 79 ? 79 : length;
    memcpy(buffer, payload, len);
    buffer[length] = 0;

    Serial.print(">> Topic: ");
    Serial.println(topic_sub);

    Serial.print(">> Payload: ");
    Serial.println(buffer);

	char * pch=0;
	//printf ("Looking for the ':' chars in \"%s\"...\n",buffer);
	pch=strchr(buffer,':');
	if(pch) {
		tempTry = 0;
		r_Sensor = atoi(buffer);	// number of Sensors
		printf ("number of sensor %d\n",r_Sensor);

    r_hOF = atoi(pch+1);	// heating_system_status
    heating_system_status = r_hOF;
    systemSTATUS2SPIFFS();
    printf ("heating_system_status %d\n",r_hOF);
    pch=strchr(pch+1,':');

    autoOff_OnTimer = atof(pch+1);
    printf ("autoOff_OnTimer %d\n",autoOff_OnTimer);
    pch=strchr(pch+1,':');

    tmp_accCountValue = atof(pch+1);
    if(tmp_accCountValue != accCountValue)
    {
		    accCountValue = tmp_accCountValue;
		      //INTstateHistory = 1;
	  }
	Serial.print("accCountValue : ");
	Serial.println(accCountValue);
	pch=strchr(pch+1,':');


	while (pch!=NULL)
	{
			L_Temp[i] = atof(pch+1);
			Serial.println(L_Temp[i]);
			//printf ("found at %d\n",pch-buffer+1);
			pch=strchr(pch+1,':');
			old_celsius[i] = 0;
			++i;
	}
	old_celsius[i] = 0;

    for ( i = 0; i < numSensor; i++) {
      isOFF[i] = 0;
      //Timer_2[i] += 5000000UL;
    }

	   userTempset = 1;
     initSending = 1; //3times send all sensor data
     L_Temp2SPIFFS(); //store received L_Temp to SPIFFS
	}
	else {
		String exec = String(buffer);
    exec_save_command(exec);
/*
		if (exec_command == do_fw_update) {
			Serial.println(">> do_ots_update_exe ");
      ota_http_update();
      mqtt_restart();
		}
    else if (exec_command == do_web_update) {
			Serial.println(">> do_web_update_exe ");
			io2LIFEhttpUpdate(updateServer, fwImage);
      mqtt_restart();
		}
		else if (exec_command == do_reboot) {
			Serial.println(">> do_reboot_exe ");
			do_reboot_exe();
		}
		else if (exec_command == do_format) {
			Serial.println(">> do_format_exe:SPIFFS.format() ");
			SPIFFS.format();
			do_reboot_exe();
		}
*/
	}
}

// -------------------------------------------------------------------
// MQTT verifyFingerprint
// -------------------------------------------------------------------
void verifyFingerprint() {
  //const char* host = MQTT_SERVER;

  Serial.print("Connecting to ");
  Serial.println(mqtt_server);

  //WiFiManager wifiManager;

  if (! espClient.connect(mqtt_server.c_str(), MQTT_PORT))
  {
	  Serial.println(mqtt_server);
	  Serial.println(MQTT_PORT);

	  //Serial.println("MQTT Connection failed. Halting execution.");
	  //wifiManager.resetSettings();
	  //wifiManager.startConfigPortal("AutoConnectAP","password");

	  //while(1);
  }

  if (espClient.verify(fingerprint, mqtt_server.c_str()))
  {
    Serial.println("Connection secure.");
  } else {
	Serial.println("verify Connection insecure! Halting execution.");
	//wifiManager.resetSettings();
	//wifiManager.startConfigPortal("AutoConnectAP","password");

    //while(1);
  }
}
// -------------------------------------------------------------------
// MQTT macToTopic
// -------------------------------------------------------------------
void macToTopic()
{
  String result;
  MAC = "";
  result = WiFi.macAddress();
  for (int i = 0; i < 16; ) {
  MAC += result.substring(i, i+2);
  i +=3;
  }
  topic_pub += MAC;
  topic_sub += MAC;
}
// -------------------------------------------------------------------
// MQTT Connect
// -------------------------------------------------------------------
boolean mqtt_connect()
{
  macToTopic();

  mqttclient.setServer(mqtt_server.c_str(), 8883);
  Serial.println("MQTT Connecting...");
  //String strID = String(ESP.getChipId());
  //String strID = String(WiFi.macAddress());
  if (mqttclient.connect(strID.c_str(), mqtt_user.c_str(), mqtt_pass.c_str())) {  // Attempt to connect
    Serial.println("MQTT connected");
		// Publish
		//if (mqttclient.publish((char *)topic_pub.c_str(), "hello MQTT connected...")) {
		if (mqttclient.publish((char *)topic_pub.c_str(), (char *)topic_pub.c_str())) {
        mqttclient.publish((char *)topic_pub.c_str(), "hello MQTT connected...");
	      Serial.println("publish ok");
		}
		else {
        Serial.println("publish failed");
		}
		// Subscribe
		if (mqttclient.subscribe((char *)topic_sub.c_str())) {
        Serial.println("Subscribe ok");
		}
		else {
        Serial.println("Subscribe failed");
		}
		//mqttclient.publish(mqtt_topic.c_str(), "connected"); // Once connected, publish an announcement..
  } else {
    Serial.print("MQTT failed: ");
    Serial.println(mqttclient.state());
    return(0);
  }
  return (1);
}

// -------------------------------------------------------------------
// Publish to MQTT
// Split up data string into sub topics: e.g
// data = CT1:3935,CT2:325,T1:12.5,T2:16.9,T3:11.2,T4:34.7
// base topic = emon/ESP-main
// MQTT Publish: emon/ESP-main/CT1 > 3935 etc..
// -------------------------------------------------------------------
void mqtt_publish(String data)
{
  String mqtt_data = "";
  String topic = mqtt_topic + "/" + mqtt_feed_prefix;
  int i=0;
  while (int(data[i])!=0)
  {
    // Construct MQTT topic e.g. <base_topic>/CT1 e.g. ESP-main/CT1
    while (data[i]!=':'){
      topic+= data[i];
      i++;
      if (int(data[i])==0){
        break;
      }
    }
    i++;
    // Construct data string to publish to above topic
    while (data[i]!=','){
      mqtt_data+= data[i];
      i++;
      if (int(data[i])==0){
        break;
      }
    }
    // send data via mqtt
    //delay(100);
    Serial.printf("%s = %s\r\n", topic.c_str(), mqtt_data.c_str());
    mqttclient.publish(topic.c_str(), mqtt_data.c_str());
    topic = mqtt_topic + "/" + mqtt_feed_prefix;
    mqtt_data="";
    i++;
    if (int(data[i])==0) break;
  }

  String ram_topic = mqtt_topic + "/" + mqtt_feed_prefix + "freeram";
  String free_ram = String(ESP.getFreeHeap());
  mqttclient.publish(ram_topic.c_str(), free_ram.c_str());

  		if (mqttclient.publish((char *)topic_pub.c_str(), "hello world...")) {
			Serial.println("publish ok");
		}
}

// -------------------------------------------------------------------
// MQTT state management
//
// Call every time around loop() if connected to the WiFi
// -------------------------------------------------------------------
void mqtt_loop()
{
  if ((!mqttclient.connected())&& (mqtt_server)) {
    long now = millis();
    // try and reconnect continuously for first 5s then try again once every 10s
    if ( (now < 50000) || ((now - lastMqttReconnectAttempt)  > 100000) ) {
      lastMqttReconnectAttempt = now;
      if (mqtt_connect()) { // Attempt to reconnect
        lastMqttReconnectAttempt = 0;

      }
    }
  } else {
    // if MQTT connected
    mqttclient.loop();
  }
}

void mqtt_restart()
{
  if (mqttclient.connected()) {
    Serial.println(">> mqttclient.disconnect... ");
    mqttclient.disconnect();
    if (!mqttclient.connected()) {
    reconnect();
    }
  }
}

boolean mqtt_connected()
{
  return mqttclient.connected();
}
