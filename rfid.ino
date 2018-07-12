/*
 * Server Branch
 */

#include <ESP8266WiFi.h>                           //Used to connect to other Hotspots
//#include <ESP8266HTTPClient.h>                     //Client used on the Clients Branch to make POST requests
#include <ESP8266WebServer.h>                      //Used to Serve a Web server on the network
#include <Ticker.h>                                //Used to call a given function with a certain period
#include <EEPROM.h>                                //Used to store configs on non-volatile memory
#include <WiFiUdp.h>                               //Used for UDP communication between ESP8266 and an external client(In our case a NTP server)
#include "helpers.h"                               //internal header
#include "global.h"                                //internal header
#include <SPI.h>                                   //Used to stabish a SPI connection with an RFID reader
#include "MFRC522.h"                               //Used to interface with the RFID reader


/*
* Include the HTML, STYLE and Script "Pages"
*/
//#include "Page_Root.h"                              //Unused Page
#include "Backend_Rfid.h"
#include "Page_Admin.h"
#include "Page_Script.js.h"
#include "Page_Style.css.h"
#include "Page_NTPsettings.h"
#include "Page_Information.h"
#include "Page_rfid.h"
#include "Page_General.h"
#include "PAGE_NetworkConfiguration.h"
//#include "example.h"                               //Example removed from root


#define ACCESS_POINT_NAME  "ESP"                   //SSID name of the network created by the ADMIN mode
#define ACCESS_POINT_PASSWORD  "12345678"          //PASSWORD of the network created by the ADMIN mode
#define AdminTimeOut 180                           //Defines the Time in Seconds, when the Admin-Mode will be diabled
#define LED_BUILTIN 2                              //Defines the Pin number of the integrated led on the ESP8266

void setup ( void ) {
  pinMode(LED_BUILTIN, OUTPUT);                    //SET the Pin number of the integrated led on the ESP8266 as OUTPUT
  EEPROM.begin(512);                               //Start the EEPROM object, to read or write data
  Serial.begin(115200);
  delay(500);
  SPI.begin();                                     //Initialize SPI bus
  mfrc522.PCD_Init();                              //Initialize MFRC522
  Serial.println(F("Starting ES8266"));
  if (!ReadConfig())
  {
    // Requieres: WriteConfig, EEPROM, ReadConfig
    // Efect: If ReadConfig states that theres no config saved,
    //        then it writes a default config.
    // Modifies: config structure, and EEPROM data (declared on global.h)
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


  if (AdminEnabled){
    // Requieres: AdminEnabled, WiFi(object)
    // Efect: Checks if the ADMIN mode is enabled,
    //        if so starts the ADMIN wifi network and turns on the bultin led.
    // Modifies: LED_BUILTIN status and the Wifi(object)
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP( ACCESS_POINT_NAME , ACCESS_POINT_PASSWORD);
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    WiFi.mode(WIFI_STA);
  }

  ConfigureWifi();                                 //Tries to connect to the network saved on the settings

  /*  //Removed pages from the root of the server "/"
      server.on ( "/", processExample  );
      server.on ( "/admin/filldynamicdata", filldynamicdata );
      server.on ( "/example.html", []() { server.send ( 200, "text/html", PAGE_EXAMPLE );  } );
  */
  /*
  *   server.on ( "URL",   []() {                  //Do something when a client request 'URL'
  *   Serial.println(F("URL"));                    //Print the URL requested on the Serial
  *   server.send ( 200, "text/html", "" );        //Retrun 200(OK) ,the formating of our reply , the reply its self (in this case nothing)
  *   }  );                                        //More can be fount at: https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html, https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples
  */
  server.on ( "/favicon.ico",   []() {
    Serial.println(F("favicon.ico"));
    server.send ( 200, "text/html", "" );
  }  );
  server.on ( "/", send_RFID_PageOrTake_Post );
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

  //On the following ones the reply is sent from the function its self,
  // with server.send ( 200, "text/plain", values);.
  server.on ( "/admin/values", send_network_configuration_values_html );
  server.on ( "/admin/connectionstate", send_connection_state_values_html );
  server.on ( "/admin/infovalues", send_information_values_html );
  server.on ( "/admin/ntpvalues", send_NTP_configuration_values_html );
  server.on ( "/admin/generalvalues", send_general_configuration_values_html);
  server.on ( "/admin/devicename",     send_devicename_value_html);
  server.on ( "/rfidvalues", send_rfid_values_html);

  server.onNotFound ( []() {
    Serial.println(F("Page Not Found"));
    server.send ( 400, "text/html", "Welp that can't be found here, try it later" );
  }  );

  server.begin();                                  //Start the WebServer!
  Serial.println( "HTTP server started" );
  tkSecond.attach(1, Second_Tick);                 //Timer for Updating Datetime Structure
  UDPNTPClient.begin(2390);                        // Port for NTP receiver (UDP object)
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
  UpdateDynamicData();
  /*
       Extra code goes here
  */

  /*if (Refresh){
    Refresh = false;
    Serial.printf("FreeMem:%d %d:%d:%d %d.%d.%d \n",ESP.getFreeHeap() , DateTime.hour,DateTime.minute, DateTime.second, DateTime.year, DateTime.month, DateTime.day);
  }*/

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
