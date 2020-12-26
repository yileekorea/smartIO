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
 //#include <time.h>
#include "NTPClient.h"
#include <WiFiUdp.h>

#include "io2better.h"
#include "input.h"
#include "mqtt.h"
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include "OneWire.h"

#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

#define a_min 60000UL //1min


String testMacaddress = "5C:CF:7F:23:F1:36";	//"5C:CF:7F:23:F1:36";
String yileeMacaddress = "2C:3A:E8:08:E3:3D";	//"2C:3A:E8:08:E3:3D";

byte Father_room = 0; //Fatherroom
byte Mother_room = 1; //Motherroom
byte Master_room = 3; //livingroom2
byte YJ_room = 4; //Yejinroom2
byte HJ_room = 5; //Hyunjunroom

OneWire  ds(2);  // on pin 2 (a 4.7K resistor is necessary)

String sName[]= {"a","b","c","d","e","f","g","h","i","j"};
float old_celsius[] = {26,26,26,26,26,26,26,26,26};
float celsius[] = {26,26,26,26,26,26,26,26,26};
float old_rStatus[] = {1,1,1,1,1,1,1,1,1}; //room status all ON
float rStatus[] = {0,0,0,0,0,0,0,0,0}; //room status all OFF
float L_Temp[] = {26.7,26.7,26.7,26.7,26.7,26.7,26.7,26.7,26.7}; //max 9
byte address[9][8]; // sensor's max 9, each address is 8

unsigned long Timer_1[] = {0,0,0,0,0,0,0,0,0};
unsigned long Timer_2[] = {0,0,0,0,0,0,0,0,0};
byte isOFF[] = {0,0,0,0,0,0,0,0,0};
int autoOff_OnTimer = 20; //20min

String input_string="";
String last_datastr="";

//const byte interruptPin = 13;
const byte interruptPin = 5;
float accCountValue = 0.00;
volatile byte INTstateHistory = 0;

void accCount() {
  delayMicroseconds(4000);
  if(digitalRead(interruptPin) != LOW) return;

  detachInterrupt(interruptPin);
  accCountValue = accCountValue + 0.01;
  INTstateHistory = 1;
  userTempset = 1;
  old_celsius[0] += 0.5;

  //state = !state;
  Serial.print("  INT_Status[] -----> ");
  Serial.println(accCountValue);

}

void INTsetup() {
  pinMode(interruptPin, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(interruptPin), accCount, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPin), accCount, FALLING);
}

/*
 * setON_OFFstatus by the measured Temperature
 */
void setON_OFFstatus(byte Sensor){
   byte nSensor = Sensor;


   if(heating_system_status)
   {
     if((WiFi.macAddress() == yileeMacaddress)||(WiFi.macAddress() == testMacaddress)){
       Serial.print("nSensor: ");
       Serial.println(nSensor);
       Serial.println(WiFi.macAddress()); //5C:CF:7F:23:F1:36
       Serial.println(yileeMacaddress);  //2C:3A:E8:08:E3:3D

       //really going OFF condition...
       //if(((L_Temp[nSensor] <= celsius[nSensor]) && (isOFF[nSensor] == 0))&&(nSensor==2)) {
       if(((L_Temp[nSensor] <= celsius[nSensor]) && (isOFF[nSensor] == 0))) {
          rStatus[nSensor] = 0;       //OFF valve
          isOFF[nSensor] = 1;
          Timer_1[nSensor] = epochTime;   //point of turned OFF

          Serial.print("going OFF Formatted Time: ");
          Serial.println(formattedTime);

          Serial.print("going OFF Epoch Time: ");
          Serial.println(epochTime);
        }

        Serial.print("epochTime - Timer_1[nSensor-");
        Serial.print(nSensor);
        Serial.print("]: ");
        Serial.println((epochTime - Timer_1[nSensor]));
        Serial.print("autoOff_OnTimer * 60UL: ");
        Serial.println((autoOff_OnTimer * 60UL));

        debugV("epochTime - Timer_1[ %u ]= %u", nSensor, (epochTime - Timer_1[nSensor]));

        //going ON condition,, based on autoOff_OnTimer
        //if(((L_Temp[nSensor] > celsius[nSensor]) && ((epochTime - Timer_1[nSensor]) > (autoOff_OnTimer * 60UL)))&&(nSensor==2)) {
        if(((L_Temp[nSensor] > celsius[nSensor]) && ((epochTime - Timer_1[nSensor]) > (autoOff_OnTimer * 60UL)))) {
            rStatus[nSensor] = L_Temp[nSensor];	//ON valve
            isOFF[nSensor] = 0;
            Timer_2[nSensor] = epochTime;   //point of turned ON

       }
/*
       if(isOFF[Master_room] == 0){
           if(L_Temp[Father_room] > celsius[Father_room]){
             rStatus[Father_room] = L_Temp[Father_room];	//ON valve - Father room
             isOFF[Father_room] = 0;
           }

           if(L_Temp[Mother_room] > celsius[Mother_room]){
             rStatus[Mother_room] = L_Temp[Mother_room];	//ON valve - YS room
             isOFF[Mother_room] = 0;
           }

           if(L_Temp[YJ_room] > celsius[YJ_room]){
             rStatus[YJ_room] = L_Temp[YJ_room];	//ON valve - YJ room
             isOFF[YJ_room] = 0;
           }
       }
*/
     }
     else{
      //really going OFF condition...
      //if((L_Temp[nSensor] <= celsius[nSensor]) && ((millis() - Timer_2[nSensor]) > interOpenTimer) && (isOFF[nSensor] == 0)) {
      //if((L_Temp[nSensor] <= celsius[nSensor]) && ((now2 - Timer_2[nSensor]) > interOpenTimer) && (isOFF[nSensor] == 0)) {
      if((L_Temp[nSensor] <= celsius[nSensor]) && (isOFF[nSensor] == 0)) {
         rStatus[nSensor] = 0;       //OFF valve
         isOFF[nSensor] = 1;

         Serial.print("going OFF Formatted Time: ");
         Serial.println(formattedTime);

         Serial.print("going OFF Epoch Time: ");
         Serial.println(epochTime);

         Timer_1[nSensor] = epochTime;   //point of turned OFF
       }

       Serial.print("epochTime - Timer_1[nSensor-");
       Serial.print(nSensor);
       Serial.print("]: ");
       Serial.println((epochTime - Timer_1[nSensor]));
       Serial.print("autoOff_OnTimer * 60UL: ");
       Serial.println((autoOff_OnTimer * 60UL));

       //going ON condition,, based on autoOff_OnTimer
       //if((L_Temp[nSensor] > celsius[nSensor]) && ((millis() - Timer_1[nSensor]) > (autoOff_OnTimer * a_min))) {
       if((L_Temp[nSensor] > celsius[nSensor]) && ((epochTime - Timer_1[nSensor]) > (autoOff_OnTimer * 60UL))) {
           rStatus[nSensor] = L_Temp[nSensor];	//ON valve
           isOFF[nSensor] = 0;
/*
           Serial.print("going ON Formatted Time: ");
           Serial.println(formattedTime);

           Serial.print("going ON Epoch Time: ");
           Serial.println(epochTime);
*/
           Timer_2[nSensor] = epochTime;   //point of turned ON
      }
     }
   }
   else{
     rStatus[nSensor] = 0;
   }
} //void setON_OFFstatus(byte Sensor)

/*
 * Sensor address read
 */
void readOneWireAddr()
{
  byte nSensor = 0;
  byte i;
  byte addr[8];

  //Serial.println("readOneWireAddr");
  while (ds.search(addr)) {
    sName[nSensor] = "";
    Serial.println();
    Serial.print("ROM =");
    for ( i = 0; i < 8; i++) {
      sName[nSensor] += String(addr[i], HEX);
      address[nSensor][i] = addr[i];
      Serial.write(' ');
      Serial.print(address[nSensor][i], HEX);
    }
    Serial.println();
    Serial.print("sName :");
    Serial.print(sName[nSensor]);

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!:" + nSensor);
    }
    Serial.println();
    nSensor++;
    delay(250);
  }
  numSensor = nSensor;
  Serial.printf("number of Sensors are '%d'\n", numSensor);

}

/*
 * read out the measured Temperature data
 */
void readoutTemperature(byte Sensor)
{
  byte nSensor = Sensor;
  byte s, i;
  byte present = 0;
  byte type_s = 0; //Chip = DS18B20
  byte data[12];
  byte addr[8];

  //Serial.println("readoutTemperature");
  //while (ds.search(addr)) {
  //for ( s = 0; i < numSensor; i++) {
    //Serial.println();
    //Serial.print("ROM =");
    for ( i = 0; i < 8; i++) {
      addr[i] = address[nSensor][i];
      //Serial.write(' ');
      //Serial.print(addr[i], HEX);
    }
    //Serial.println();

    // the first ROM byte indicates which chip
    switch (addr[0]) {
          case 0x10:
          //Serial.println("  Chip = DS18S20");  // or old DS1820
          type_s = 1;
          break;
    case 0x28:
          //Serial.println("  Chip = DS18B20");
          type_s = 0;
          break;
    case 0x22:
          //Serial.println("  Chip = DS1822");
          type_s = 0;
          break;
    default:
          Serial.println("Device is not a DS18x20 family device.");
    }

    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);     // Read Scratchpad
    /*
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    */
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
        //Serial.print(data[i], HEX);
        //Serial.print(" ");
    }
    /*
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();
    */


    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        // default is 12 bit resolution, 750 ms conversion time
		// but set as 10bits
		raw = raw & ~3; // 10 bit res, 187.5 ms
    }
    celsius[nSensor] = (float)raw / 16.0;
    //fahrenheit[nSensor] = celsius[nSensor] * 1.8 + 32.0;
    Serial.println("--- readoutTemperature");
    Serial.print("  Temps = ");
    Serial.print(celsius[nSensor]);
    Serial.println(" 'C");
/*
  if(celsius[nSensor] == 85){
    Serial.println("Temperature is abnormal -- set to manual");
    celsius[nSensor] = L_Temp[nSensor]-1;
  }
*/
	if(L_Temp[nSensor]){
		Serial.print("  L_Temp[");
		Serial.print(nSensor);
		Serial.print("] ====> ");
		Serial.println(L_Temp[nSensor]);
	}
/*
	if(L_Temp[nSensor] <= celsius[nSensor]){
		rStatus[nSensor] = 0;
	}else{
		rStatus[nSensor] = L_Temp[nSensor];
		if(L_Temp[nSensor]){
			Serial.print("  rStatus[] -----> ");
			Serial.println(rStatus[nSensor]);
		}
	}
*/
  setON_OFFstatus(nSensor);

  //} //numSensor
} //readTemperature


/*
 * ask Sensor to measure Temperature
 */
void measureTemperature(byte Sensor)
{
  byte nSensor = Sensor;
  byte s, i;
  byte type_s;
  byte addr[8];

  //Serial.println("measureTemperature");
  //while (ds.search(addr)) {
  //for ( s = 0; i < numSensor; i++) {
    //Serial.println();
    //Serial.print("ROM =");
    for ( i = 0; i < 8; i++) {
      addr[i] = address[nSensor][i];
      //Serial.write(' ');
      //Serial.print(addr[i], HEX);
    }
    //Serial.println();

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);  // start conversion, with parasite power on at the end

    //delay(1000);     	// maybe 750ms is enough, maybe not
    delay(200);     	// maybe 187.5ms is enough, maybe not
    //delay(20);     	// test purpose
  //} //numSensor
} //measureTemperature

/*
 * Temperature measurement
 */
String readFromOneWire()
{
  String payload="";

    byte nSensor = 0;
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];

    while (ds.search(addr)) {
    sName[nSensor] = "";
    //Serial.print("ROM =");
    for ( i = 0; i < 8; i++) {
        //Serial.write(' ');
        //Serial.print(addr[i], HEX);
		sName[nSensor] += String(addr[i], HEX);
    }
    //Serial.print(sName[nSensor]);

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
		payload="";
        return payload;
    }
    Serial.println();

    //readoutTemperature(nSensor);

    // the first ROM byte indicates which chip
    switch (addr[0]) {
          case 0x10:
          //Serial.println("  Chip = DS18S20");  // or old DS1820
          type_s = 1;
          break;
    case 0x28:
          //Serial.println("  Chip = DS18B20");
          type_s = 0;
          break;
    case 0x22:
          //Serial.println("  Chip = DS1822");
          type_s = 0;
          break;
    default:
          Serial.println("Device is not a DS18x20 family device.");
		  payload="";
          return payload;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);  // start conversion, with parasite power on at the end

    //delay(1000);     	// maybe 750ms is enough, maybe not
    delay(200);     	// maybe 187.5ms is enough, maybe not
    //delay(20);     	// test purpose


    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);     // Read Scratchpad
    /*
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    */
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
        //Serial.print(data[i], HEX);
        //Serial.print(" ");
    }
    /*
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();
    */


    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        // default is 12 bit resolution, 750 ms conversion time
		// but set as 10bits
		raw = raw & ~3; // 10 bit res, 187.5 ms
    }
    celsius[nSensor] = (float)raw / 16.0;
    //fahrenheit[nSensor] = celsius[nSensor] * 1.8 + 32.0;
    Serial.println("--- measuredTemperature");
    Serial.print("  Temps = ");
    Serial.print(celsius[nSensor]);
    Serial.print(" 'C");


	if(L_Temp[nSensor]){
		Serial.print("  L_Temp[");
		Serial.print(nSensor);
		Serial.print("] ====> ");
		Serial.print(L_Temp[nSensor]);
	}
/*
	if(L_Temp[nSensor] <= celsius[nSensor]){
		rStatus[nSensor] = 0;
	}else{
		rStatus[nSensor] = L_Temp[nSensor];
		if(L_Temp[nSensor]){
			Serial.print("  rStatus[] -----> ");
			Serial.println(rStatus[nSensor]);
		}
	}
*/
    setON_OFFstatus(nSensor);

    nSensor += 1;
    payload = "OK";

  }
    Serial.println();
    Serial.println("No more addresses.");
    ds.reset_search();
    delay(1000);

	numSensor = nSensor;
    return payload;
}


boolean input_get(String& data)
{
  boolean gotData = false;

  // If data from test API e.g `http://<IP-ADDRESS>/input?string=CT1:3935,CT2:325,T1:12.5,T2:16.9,T3:11.2,T4:34.7`
  if(input_string.length() > 0) {
    data = input_string;
    input_string = "";
    gotData = true;
  }
  // If data received on serial
  else if (Serial.available()) {
    // Could check for string integrity here
    data = Serial.readStringUntil('\n');
    gotData = true;
  }

  if(gotData)
  {
    // Get rid of any whitespace, newlines etc
    data.trim();

    if(data.length() > 0) {
      Serial.printf("Got '%s'\n", data.c_str());
      last_datastr = data;
    } else {
      gotData = false;
    }
  }

  return gotData;
}
