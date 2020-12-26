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

#include <Arduino.h>
#include <ESP8266WebServer.h>         // Config portal
#include <FS.h>                       // SPIFFS file-system: store web server html, CSS etc.

#include "io2better.h"
#include "web_server.h"
#include "config.h"
#include "wifi.h"
#include "mqtt.h"
#include "input.h"
//#include "ESP-main-2.h"
#include "ota.h"

ESP8266WebServer server(80);          //Create class for Web server

// Get running firmware version from build tag environment variable
#define TEXTIFY(A) #A
#define ESCAPEQUOTE(A) TEXTIFY(A)
String currentfirmware = ESCAPEQUOTE(BUILD_TAG);
//String currentfirmware = "release.0.0.6-master-2017-12-28-389";
String buildVersion = "";

void BuildVersion(){
  String message = currentfirmware;
	byte hyphenPosition;
	byte i;

	for(i=0; i<5; i++){
		hyphenPosition = message.indexOf('-');
		if(hyphenPosition != -1)
		{
			//Serial.println(message.substring(0,hyphenPosition));
			message = message.substring(hyphenPosition+1,message.length());
		}
		else
		{
			if(message.length()>0)
			Serial.println(message);
		}
	}
	buildVersion = message;
  Serial.println(message);
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  //if(path.endsWith("/")) path += "simple_home.html";
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    Serial.println("handleFileRead-true");
    return true;
  }
  Serial.println("handleFileRead-false");
  return false;
}



// -------------------------------------------------------------------
// Helper function to decode the URL values
// -------------------------------------------------------------------
void decodeURI(String& val)
{
    val.replace("%21", "!");
//    val.replace("%22", '"');
    val.replace("%23", "#");
    val.replace("%24", "$");
    val.replace("%26", "&");
    val.replace("%27", "'");
    val.replace("%28", "(");
    val.replace("%29", ")");
    val.replace("%2A", "*");
    val.replace("%2B", "+");
    val.replace("%2C", ",");
    val.replace("%2D", "-");
    val.replace("%2E", ".");
    val.replace("%2F", "/");
    val.replace("%3A", ":");
    val.replace("%3B", ";");
    val.replace("%3C", "<");
    val.replace("%3D", "=");
    val.replace("%3E", ">");
    val.replace("%3F", "?");
    val.replace("%40", "@");
    val.replace("%5B", "[");
    val.replace("%5C", "'\'");
    val.replace("%5D", "]");
    val.replace("%5E", "^");
    val.replace("%5F", "_");
    val.replace("%60", "`");
    val.replace("%7B", "{");
    val.replace("%7C", "|");
    val.replace("%7D", "}");
    val.replace("%7E", "~");
    val.replace('+', ' ');

    // Decode the % char last as there is always the posibility that the decoded
    // % char coould be followed by one of the other replaces
    val.replace("%25", "%");
}

// -------------------------------------------------------------------
// Load Home page
// url: /
// -------------------------------------------------------------------
void handleHome() {
  SPIFFS.begin(); // mount the fs
  Serial.println("simple_home");
  File f = SPIFFS.open("/simple_home.html", "r");
/*
  if (wifi_mode = WIFI_MODE_AP_ONLY) {
    f.close();
    Serial.println("handleHome");
    File f = SPIFFS.open("/home.html", "r");
    //File f = SPIFFS.open("/simple_home.html", "r");
  }
*/
  Serial.println("f.readString()");
  if (f) {
    String s = f.readString();
    //String s = "f.readString()";
    server.send(200, "text/html", s);
    Serial.println("server.send(200, text/html, s)");
    f.close();
    //Serial.println(s);
  } else {
    server.send(200, "text/plain","/home.html not found, have you flashed the SPIFFS?");
  }
  delay(100);
/*
  Serial.print("WiFi Scan: ");
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" networks found");
  st = "";
  rssi = "";
  for (int i = 0; i < n; ++i){
    st += "\""+WiFi.SSID(i)+"\"";
    rssi += "\""+String(WiFi.RSSI(i))+"\"";

  Serial.print(WiFi.SSID(i));
  Serial.println(" : ");
  Serial.println(String(WiFi.RSSI(i)));

    if (i<n-1) st += ",";
    if (i<n-1) rssi += ",";
  }
*/
}

// -------------------------------------------------------------------
// Wifi scan
// url: /scan
// -------------------------------------------------------------------
void handleScan() {
  wifi_scan();

  delay(500);
  //server.send(200, "text/plain","[" +st+ "],[" +rssi+"]");
  server.send(200, "text/plain", "scaned");

/*
  String s = "{";
  s += "\"networks\":["+st+"],";
  s += "\"rssi\":["+rssi+"],";

  Serial.println(st);
  //Serial.println(rssi);

  s += "\"free_heap\":\""+String(ESP.getFreeHeap())+"\",";
  s += "\"version\":\""+currentfirmware+"\"";
  s += "}";
  server.send(200, "text/html", s);
*/
}

// -------------------------------------------------------------------
// Handle turning Access point off
// url: /apoff
// -------------------------------------------------------------------
void handleAPOff() {
  server.send(200, "text/html", "Turning AP Off");
  Serial.println("Turning AP Off");
  WiFi.disconnect();
  delay(1000);
  ESP.reset();
  //delay(2000);
  //WiFi.mode(WIFI_STA);
}

// -------------------------------------------------------------------
// Save selected network to EEPROM and attempt connection
// url: /savenetwork
// -------------------------------------------------------------------
void handleSaveNetwork() {
  String s;
  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");

  decodeURI(qsid);
  decodeURI(qpass);

  if (qsid != 0)
  {
    config_save_wifi(qsid, qpass);

    server.send(200, "text/plain", "saved");
    delay(2000);

    //wifi_restart();
    WiFi.disconnect();
    //delay(1000);
    //ESP.reset();
    do_reboot_exe();

  } else {
    server.send(400, "text/plain", "No SSID");
  }
}

// -------------------------------------------------------------------
// Save ESP-main-2
// url: /saveemoncms
// -------------------------------------------------------------------
void handleSaveEmoncms()
{
  config_save_emoncms(server.arg("server"),
                      server.arg("node"),
                      server.arg("apikey"),
                      server.arg("fingerprint"));

  // BUG: Potential buffer overflow issue the values emoncms_xxx come from user
  //      input so could overflow the buffer no matter the length
  char tmpStr[200];
  sprintf(tmpStr,"Saved: %s %s %s %s",emoncms_server.c_str(),emoncms_node.c_str(),emoncms_apikey.c_str(),emoncms_fingerprint.c_str());
  Serial.println(tmpStr);
  server.send(200, "text/html", tmpStr);
}

// -------------------------------------------------------------------
// Save MQTT Config
// url: /savemqtt
// -------------------------------------------------------------------
void handleSaveMqtt() {
  config_save_mqtt(server.arg("server"),
                   server.arg("topic"),
                   server.arg("prefix"),
                   server.arg("user"),
                   server.arg("pass"));

  char tmpStr[200];
  // BUG: Potential buffer overflow issue the values mqtt_xxx come from user
  //      input so could overflow the buffer no matter the length
  sprintf(tmpStr,"Saved: %s %s %s %s %s",mqtt_server.c_str(),mqtt_topic.c_str(),mqtt_feed_prefix.c_str(),mqtt_user.c_str(),mqtt_pass.c_str());
  Serial.println(tmpStr);
  server.send(200, "text/html", tmpStr);

  // If connected disconnect MQTT to trigger re-connect with new details
  mqtt_restart();
}

// -------------------------------------------------------------------
// Save the web site user/pass
// url: /saveadmin
// -------------------------------------------------------------------
void handleSaveAdmin() {
  String quser = server.arg("user");
  String qpass = server.arg("pass");

  decodeURI(quser);
  decodeURI(qpass);

  config_save_admin(quser, qpass);

  server.send(200, "text/html", "saved");
}
// -------------------------------------------------------------------
// Save the web site user/pass
// url: /savecommand
// -------------------------------------------------------------------
void handleSaveCommand() {
  String qexec = server.arg("exec");

  decodeURI(qexec);
  Serial.println("handleSaveCommand: "+ qexec);
  server.send(200, "text/html", "executed");

  exec_save_command(qexec);
}


// -------------------------------------------------------------------
// Last values on atmega serial
// url: /lastvalues
// -------------------------------------------------------------------
void handleLastValues() {
  server.send(200, "text/html", last_datastr);
}

// -------------------------------------------------------------------
// Returns status json
// url: /status
// -------------------------------------------------------------------
void handleStatus() {
Serial.println("handleStatus-in");

  String s = "{";
  if (wifi_mode==WIFI_MODE_STA) {
    s += "\"mode\":\"STA\",";
  } else if (wifi_mode==WIFI_MODE_AP_STA_RETRY || wifi_mode==WIFI_MODE_AP_ONLY) {
    s += "\"mode\":\"AP\",";
  } else if (wifi_mode==WIFI_MODE_AP_AND_STA) {
    s += "\"mode\":\"STA+AP\",";
  }

  s += "\"networks\":["+st+"],";
  s += "\"rssi\":["+rssi+"],";

  Serial.println(st);
  //Serial.println(rssi);

  s += "\"ssid\":\""+esid+"\",";
  //s += "\"pass\":\""+epass+"\",";
  s += "\"srssi\":\""+String(WiFi.RSSI())+"\",";
  s += "\"ipaddress\":\""+ipaddress+"\",";
  //s += "\"emoncms_server\":\""+emoncms_server+"\",";
  //s += "\"emoncms_node\":\""+emoncms_node+"\",";
  //s += "\"emoncms_apikey\":\""+emoncms_apikey+"\",";
  //s += "\"emoncms_fingerprint\":\""+emoncms_fingerprint+"\",";
  //s += "\"emoncms_connected\":\""+String(emoncms_connected)+"\",";
  //s += "\"packets_sent\":\""+String(packets_sent)+"\",";
  //s += "\"packets_success\":\""+String(packets_success)+"\",";

  s += "\"mqtt_server\":\""+mqtt_server+"\",";
  //s += "\"mqtt_topic\":\""+mqtt_topic+"\",";
  //s += "\"mqtt_feed_prefix\":\""+mqtt_feed_prefix+"\",";
  s += "\"mqtt_user\":\""+mqtt_user+"\",";
  s += "\"mqtt_pass\":\""+mqtt_pass+"\",";
  s += "\"mqtt_connected\":\""+String(mqtt_connected())+"\",";

  s += "\"www_username\":\""+www_username+"\",";
  s += "\"www_password\":\""+www_password+"\",";

  s += "\"free_heap\":\""+String(ESP.getFreeHeap())+"\",";
  s += "\"version\":\""+currentfirmware+"\"";

  s += "}";
  server.send(200, "text/html", s);
  //server.send(200, "application/json", s);
}

// -------------------------------------------------------------------
// Reset config and reboot
// url: /reset
// -------------------------------------------------------------------
void handleRst() {
  config_reset();
  server.send(200, "text/html", "1");
  wifi_disconnect();
  delay(1000);
  ESP.reset();
}

// -------------------------------------------------------------------
// Restart (Reboot)
// url: /restart
// -------------------------------------------------------------------
void handleRestart() {
  server.send(200, "text/html", "1");
  delay(1000);
  wifi_disconnect();
  ESP.restart();
}

// -------------------------------------------------------------------
// Handle test input API
// url /input
// e.g http://192.168.0.75/input?string=CT1:3935,CT2:325,T1:12.5,T2:16.9,T3:11.2,T4:34.7
// -------------------------------------------------------------------
void handleInput(){
  input_string = server.arg("string");
  server.send(200, "text/html", input_string);
  Serial.println(input_string);
}

// -------------------------------------------------------------------
// Check for updates and display current version
// url: /firmware
// -------------------------------------------------------------------
String handleUpdateCheck() {
  Serial.println("Running: " + currentfirmware);
  // Get latest firmware version number
  String latestfirmware = ota_get_latest_version();
  Serial.println("Latest: " + latestfirmware);
  // Update web interface with firmware version(s)
  String s = "{";
  s += "\"current\":\""+currentfirmware+"\",";
  s += "\"latest\":\""+latestfirmware+"\"";
  s += "}";
  server.send(200, "text/html", s);
  return (latestfirmware);
}


// -------------------------------------------------------------------
// Update firmware
// url: /update
// -------------------------------------------------------------------
void handleUpdate() {
  Serial.println("UPDATING...");
  delay(500);

  t_httpUpdate_return ret = ota_http_update();

  int retCode = 400;
  String str="error";
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      str = Serial.printf("Update failed error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      str = "No update, running latest firmware";
      break;
    case HTTP_UPDATE_OK:
      retCode = 200;
      str = "Update done!";
      break;
  }
  server.send(retCode, "text/plain", str);
  Serial.println(str);
}

void web_server_setup()
{
Serial.println("web_server_setup");
  // Start server & server root html /
  server.on("/", [](){
    if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()) && wifi_mode == WIFI_MODE_STA)
      return server.requestAuthentication();
    handleHome();
  });

  // Handle HTTP web interface button presses
  //server.on("/generate_204", handleHome);  //Android captive portal. Maybe not needed. Might be handled by notFound
  //server.on("/fwlink", handleHome);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound

  server.on("/status", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
Serial.println("handleStatus-out1");
  handleStatus();
  });
  server.on("/savenetwork", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleSaveNetwork();
  });
  server.on("/saveemoncms", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleSaveEmoncms();
  });
  server.on("/savemqtt", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleSaveMqtt();
  });
  server.on("/saveadmin", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleSaveAdmin();
  });

  server.on("/savecommand", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleSaveCommand();
  });

  server.on("/scan", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
Serial.println("handleScan,,,");
  handleScan();
  });

  server.on("/apoff", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleAPOff();
  });
  server.on("/firmware", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleUpdateCheck();
  });

  server.on("/update", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleUpdate();
  });

  server.on("/status", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
Serial.println("handleStatus-out2");
  handleStatus();
  });
  server.on("/lastvalues", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleLastValues();
  });
  server.on("/reset", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleRst();
  });
  server.on("/restart", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleRestart();
  });

  server.on("/input", [](){
  if(www_username!="" && !server.authenticate(www_username.c_str(), www_password.c_str()))
    return server.requestAuthentication();
  handleInput();
  });


  server.onNotFound([](){
Serial.println(server.uri());
  if(!handleFileRead(server.uri()))
    server.send(404, "text/plain", "NotFound");
  });

Serial.println("server.begin()");
  server.begin();
}

void web_server_loop()
{
  server.handleClient();          // Web server
}
