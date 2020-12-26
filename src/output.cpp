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
#include "input.h"
#include "output.h"
//#include "gpio_MCP23S17.h"   // import library

//#include "Adafruit_MCP23017.h"

//#define R1_BUILTIN 4

//const uint8_t sclk = 14;
//const uint8_t mosi =13; //Master Output Slave Input ESP8266=Master,MCP23S08=slave

const uint8_t MCP_CS = 15;
gpio_MCP23S17 mcp(MCP_CS,0x20);//instance

//Adafruit_MCP23017 mcp;

Ticker ticker;
/*
unsigned long Timer_1[] = {0,0,0,0,0,0,0,0};
unsigned long Timer_2[] = {0,0,0,0,0,0,0,0};
byte isOFF[] = {0,0,0,0,0,0,0,0};
*/

/*
void wireSetup() //I2C setup
{
  byte i;
  mcp.begin();      // use default address 0
  Serial.println("Start I2C wireSetup: ");

  for ( i = 0; i < numberofOUT_gpio ; i++) {
    mcp.pinMode(i, OUTPUT);
    mcp.digitalWrite(i, LOW);
    delay(100);
  }
  delay(300);
  for ( i = 0; i < numberofOUT_gpio ; i++) {
    mcp.digitalWrite(i, HIGH);
    delay(100);
  }

  Serial.println("End I2C wireSetup: ");

  for ( i = 8; i < (numberofIN_gpio + 8) ; i++) {
    mcp.pinMode(i, INPUT);
  }
}
*/

/*
 * SPI relayControl
 */
void valve_relayControl() {
  byte i;

	if (heating_system_status)
  {
		for ( i = 0; i < (numSensor-1) ; i++) {
		  //rStatus[i] == 0 ? mcp.digitalWrite(i,HIGH) : mcp.digitalWrite(i,LOW);

      //Serial.print("valve_relayControl rStatus-");
		  //Serial.print(i);
		  //Serial.print(" : ");
		  //Serial.println(rStatus[i]);

		  //rStatus[i] > 0 ? mcp.gpioDigitalWrite(i+8,LOW) : mcp.gpioDigitalWrite(i+8,HIGH);
		  if (rStatus[i] > 0){
			//mcp.gpioPinMode(OUTPUT);
      //mcp.gpioDigitalWrite(i+8,LOW);
  		delay(10);
			mcp.gpioDigitalWrite(i+8,LOW);
			//Serial.println("mcp.gpioDigitalWrite(i+8,LOW)");
		  } else {
			//mcp.gpioPinMode(OUTPUT);
      //mcp.gpioDigitalWrite(i+8,HIGH);
  		delay(10);
			mcp.gpioDigitalWrite(i+8,HIGH);
			//Serial.println("mcp.gpioDigitalWrite(i+8,HIGH)");
		  }
		}
    //to control RELAY for temp_input
    if (rStatus[numSensor-1] > 0){
      delay(10);
      mcp.gpioDigitalWrite(0,LOW);
    } else {
      delay(10);
      mcp.gpioDigitalWrite(0,HIGH);
    }
	}
	else
  {
    LED_setup(0.2);
    for ( i = 0; i < (numSensor-1) ; i++) {
      //delay(300);
      mcp.gpioDigitalWrite(i+8,HIGH);
    }
    LED_clear();
	}

/*
	for ( i = 0; i < (numSensor) ; i++) {
    //if((L_Temp[i] <= celsius[i]) && ((millis() - Timer_2[i]) > 60000UL)) { // 1min
		if((L_Temp[i] <= celsius[i]) && ((millis() - Timer_2[i]) > 60000UL) && (isOFF[i] == 0)) { // 1min
      //if(isOFF[i] == 0)
      {
        mcp.digitalWrite(i,HIGH); //if current celsius Greater than setting --> off
        Timer_1[i] = millis();
        isOFF[i] = 1;
        Serial.print(" isOFF: ");
        Serial.print(i);
        Serial.println(isOFF[i]);
      }
		}



    //if((L_Temp[i] > celsius[i]) || ((millis() - Timer_1[i]) > 60000UL)) {
    if(L_Temp[i] > celsius[i]) {
      Serial.println();
      Serial.print("L_Temp-");
      Serial.print(i);
      Serial.print(" : ");
      Serial.print(L_Temp[i]);
      Serial.print("    celsius-");
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(celsius[i]);
      Serial.println();

      mcp.digitalWrite(i,LOW);
      isOFF[i] = 0;
		}
    else if((millis() - Timer_1[i]) > 180000UL) { //3min
      Serial.println();
      Serial.print(i);
      Serial.print(" : millis-");
      Serial.print(millis());
      Serial.print("  -   vControlTimer-");
      Serial.print(Timer_1[i]);
      Serial.print("  =  ");
      Serial.println((millis() - Timer_1[i]));

      Timer_1[i] = millis();
      Timer_2[i] = millis();
      mcp.digitalWrite(i,LOW);
      isOFF[i] = 0;

    }
	} // for
*/
}


/*
void wireLoop()
{
  int state = mcp.digitalRead(0);  // get the current state of GPIO1 pin
      Serial.println(" digitalRead(0): " + state);

  mcp.digitalWrite(0, !state);     // set pin to the opposite state
  mcp.digitalWrite(7, state);     // set pin to the opposite state
  delay(9000);

}
*/

/*
 * relayControl
 */
/*
void relayControl() {
    byte i;
	if(L_Temp[0] <= celsius[0]){
		digitalWrite(R1_BUILTIN,LOW);
	} else {
		digitalWrite(R1_BUILTIN,HIGH);
	}
	for ( i = 1; i < (numSensor-1) ; i++) {
		if(L_Temp[i] <= celsius[i]){
			mcp.gpioDigitalWrite(i+3,LOW); //mcp.GPIO 0 ~ 3 for input sense
		} else {
			mcp.gpioDigitalWrite(i+3,HIGH);
		}
	}
}
*/

void mcp_GPIO_setup()
{
	byte i;
	Serial.println("Attempting SPI mcp.begin()...");
	mcp.begin();//x.begin(1) will override automatic SPI initialization
	// Set all pins to be outputs (by default they are all inputs)
	mcp.gpioPinMode(OUTPUT);
	// Change all pins at once, 16-bit value
	mcp.gpioPort(0xFFFF);

  //for (int i=0;i<16;i++){
	for (int i=8;i<16;i++){
		mcp.gpioDigitalWrite(i,LOW);
		delay(300);
	}
	//for (int i=0;i<16;i++){
	for (int i=15;i>7;i--){
		mcp.gpioDigitalWrite(i,HIGH);
		delay(300);
	}
}
void mcp_GPIO_test()
{
	byte i;
	Serial.println("Attempting SPI mcp_GPIO_test...");

  //for (int i=0;i<16;i++){
	for (int i=8;i<16;i++){
		mcp.gpioDigitalWrite(i,LOW);
		delay(50);
	}
	//for (int i=0;i<16;i++){
	for (int i=15;i>7;i--){
		mcp.gpioDigitalWrite(i,HIGH);
		delay(50);
	}
}

void tick(){
  //toggle state
  int state = digitalRead(ESP_LED);  // get the current state of GPIO1 pin
  digitalWrite(ESP_LED, !state);     // set pin to the opposite state
}

void LED_setup(float t) {
  //set led pin as output
  pinMode(ESP_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(t, tick);
 }

void LED_clear() {
 	ticker.detach();
	//keep LED off
	digitalWrite(ESP_LED, HIGH);
}
