/*
 * Clients Branch
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include "helpers.h"
#include "global.h"
#include <SPI.h>
//#include <Wire.h>
#include "MFRC522.h"

/*
  Include the HTML, STYLE and Script "Pages"
*/
#include "Backend_Rfid.h"
#include "Page_Admin.h"
#include "Page_Script.js.h"
#include "Page_Style.css.h"
#include "Page_NTPsettings.h"
#include "Page_Information.h"
#include "Page_rfid.h"
#include "Page_General.h"
#include "PAGE_NetworkConfiguration.h"



#define ACCESS_POINT_NAME  "ESP"
#define ACCESS_POINT_PASSWORD  "12345678"
#define AdminTimeOut 180  // Defines the Time in Seconds, when the Admin-Mode will be diabled
#define LED_BUILTIN 2
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 4, /* data=*/ 5);   // pin remapping with ESP8266 HW I2C
/*
* I2C Connections to the screen
* GND  GND
* 3V3 VCC
* D1  SDA #1
* D2  SCL #2
*/
void setup ( void ) {
  pinMode(LED_BUILTIN, OUTPUT);
  EEPROM.begin(512);
  Serial.begin(115200);
  delay(500);
  SPI.begin();           // Init SPI bus
  /*
  *u8g2.begin();          // Init I2C bus
  *u8g2.clearBuffer();					// clear the internal memory
  *u8g2.setFont(u8g2_font_helvB12_tf);	// choose a suitable font https://github.com/olikraus/u8g2/wiki/fntlistall
  *u8g2.drawStr(0,20,"Hello World!");	// write something to the internal memory
  *u8g2.sendBuffer();					// transfer internal memory to the display
  */
  mfrc522.PCD_Init();    // Init MFRC522
  Serial.println(F("Starting ES8266"));
  if (!ReadConfig())
  {
    // DEFAULT CONFIG
    config.ssid = "SSID to connect to";
    config.password = "PASSWORD to connect to the given SSID";
    config.dhcp = true;
    config.IP[0] = 192; config.IP[1] = 168; config.IP[2] = 1; config.IP[3] = 100;
    config.Netmask[0] = 255; config.Netmask[1] = 255; config.Netmask[2] = 255; config.Netmask[3] = 0;
    config.Gateway[0] = 192; config.Gateway[1] = 168; config.Gateway[2] = 1; config.Gateway[3] = 1;
    config.ntpServerName = "1.cr.pool.ntp.org"; //Costa Rica NTP(time) server
    config.Update_Time_Via_NTP_Every =  0;
    config.timezone = -60; //Costa Rica Timezone
    config.daylight = false;
    config.DeviceName = "Not Named";
    config.AutoTurnOff = false;
    config.AutoTurnOn = false;
    config.TurnOffHour = 0;
    config.TurnOffMinute = 0;
    config.TurnOnHour = 0;
    config.TurnOnMinute = 0;
    WriteConfig();
    Serial.println(F("General config applied"));
  }


  if (AdminEnabled)
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP( ACCESS_POINT_NAME , ACCESS_POINT_PASSWORD);
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }

  ConfigureWifi();

  server.on ( "/favicon.ico",   []() {
    Serial.println(F("favicon.ico"));
    server.send ( 200, "text/html", "" );
  }  );

  server.on ( "/", []() {
    Serial.println(F("root.html"));
    server.send ( 200, "text/html", PAGE_RFID );
  }  );
  server.on ( "/config.html", send_network_configuration_html );
  server.on ( "/info.html", []() {
    Serial.println(F("info.html"));
    server.send ( 200, "text/html", PAGE_Information );
  }  );
  server.on ( "/admin.html", []() {
    Serial.println(F("admin.html"));
    server.send ( 200, "text/html", PAGE_AdminMainPage );
  }  );
  server.on ( "/ntp.html", send_NTP_configuration_html  );
  server.on ( "/general.html", send_general_html  );
  server.on ( "/style.css", []() { //This guy is taking resources by being loaded over an over.
    Serial.println(F("style.css"));
    server.send ( 200, "text/css", PAGE_Style_css );
  } );
  server.on ( "/microajax.js", []() { //This guy is taking resources by being loaded over an over.
    Serial.println(F("microajax.js"));
    server.send ( 200, "text/plain", PAGE_microajax_js );
  } );

  server.on ( "/admin/values", send_network_configuration_values_html );
  server.on ( "/admin/connectionstate", send_connection_state_values_html );
  server.on ( "/admin/infovalues", send_information_values_html );
  server.on ( "/admin/ntpvalues", send_NTP_configuration_values_html );
  server.on ( "/admin/generalvalues", send_general_configuration_values_html);
  server.on ( "/admin/devicename",     send_devicename_value_html);
  server.on ( "/admin/rfidvalues", send_rfid_values_html);


  server.onNotFound ( []() {
    Serial.println(F("Page Not Found"));
    server.send ( 400, "text/html", "Welp that can't be found here, try it later" );
  }  );
  server.begin();
  Serial.println( "HTTP server started" );
  tkSecond.attach(1, Second_Tick);
  UDPNTPClient.begin(2390);  // Port for NTP receive
}

void loop ( void ) {
  if (AdminEnabled)
  {
    if (AdminTimeOutCounter > AdminTimeOut)
    {
      AdminEnabled = false;
      Serial.println(F("Admin Mode disabled!"));
      WiFi.mode(WIFI_STA);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  if (config.Update_Time_Via_NTP_Every  > 0 )
  {
    if (cNTP_Update > 5 && firstStart)
    {
      NTPRefresh();
      cNTP_Update = 0;
      firstStart = false;
    }
    else if ( cNTP_Update > (config.Update_Time_Via_NTP_Every * 60) )
    {

      NTPRefresh();
      cNTP_Update = 0;
    }
  }

  if (DateTime.minute != Minute_Old)
  {
    Minute_Old = DateTime.minute;
    if (config.AutoTurnOn)
    {
      if (DateTime.hour == config.TurnOnHour && DateTime.minute == config.TurnOnMinute)
      {
        Serial.println(F("SwitchON"));
      }
    }


    Minute_Old = DateTime.minute;
    if (config.AutoTurnOff)
    {
      if (DateTime.hour == config.TurnOffHour && DateTime.minute == config.TurnOffMinute)
      {
        Serial.println(F("SwitchOff"));
      }
    }
  }
  server.handleClient();
  Tag_reader();

  /*
       Extra code goes here
  */

/*  if (Refresh)
  {
    Refresh = false;
    Serial.println(F("Refreshing..."));
    Serial.printf("FreeMem:%d %d:%d:%d %d.%d.%d \n",ESP.getFreeHeap() , DateTime.hour,DateTime.minute, DateTime.second, DateTime.year, DateTime.month, DateTime.day);
  }
*/
}

/*
  ESP_WebConfig

  Copyright (c) 2015 John Lassen. All rights reserved.
  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Latest version: 1.1.3  - 2015-07-20
  Changed the loading of the Javascript and CCS Files, so that they will successively loaded and that only one request goes to the ESP.

  -----------------------------------------------------------------------------------------------
  History

  Version: 1.1.2  - 2015-07-17
  Added URLDECODE for some input-fields (SSID, PASSWORD...)

  Version  1.1.1 - 2015-07-12
  First initial version to the public

*/
