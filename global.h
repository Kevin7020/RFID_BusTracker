#ifndef GLOBAL_H
#define GLOBAL_H

ESP8266WebServer server(80);									// The Webserver
//HTTPClient httpclient;                      // The Webclient from the ESP8266HTTPClient.h library(Used for the clients)
boolean firstStart = true;										// On firststart = true, NTP will try to get a valid time
int AdminTimeOutCounter = 0;									// Counter for Disabling the AdminMode
strDateTime DateTime;										     	// Global DateTime structure, will be refreshed every Second
WiFiUDP UDPNTPClient;									    		// NTP Client
unsigned long UnixTimestamp = 0;							// GLOBALTIME  ( Will be set by NTP)
boolean Refresh = false;                      // For Main Loop, to refresh things like GPIO / WS2812
int cNTP_Update = 0;											    // Counter for Updating the time via NTP
Ticker tkSecond;										      		// Second - Timer for Updating Datetime Structure
Ticker tm;										              	// Scheduler - Timer for Updating clients status Structure
boolean AdminEnabled = true;	               	// Enable Admin Mode for a given Time
byte Minute_Old = 100;				                // Helpvariable for checking, when a new Minute comes up (for Auto Turn On / Off)

struct clients {                              // Structure of variables to be sent back to the HTML
  String name;
  String status;
};

clients Parada1;
clients Parada2;
clients Parada3;

struct strConfig {
  String ssid;
  String password;
  byte  IP[4];
  byte  Netmask[4];
  byte  Gateway[4];
  boolean dhcp;
  String ntpServerName;
  long Update_Time_Via_NTP_Every;
  long timezone;
  boolean daylight;
  String DeviceName;
  boolean AutoTurnOff;
  boolean AutoTurnOn;
  byte TurnOffHour;
  byte TurnOffMinute;
  byte TurnOnHour;
  byte TurnOnMinute;
  byte LED_R;
  byte LED_G;
  byte LED_B;
}   config;


/*
**
** CONFIGURATION HANDLING
**
*/
void ConfigureWifi()
{
  WiFi.begin (config.ssid.c_str(), config.password.c_str());
  if (!config.dhcp)
  {
    WiFi.config(IPAddress(config.IP[0], config.IP[1], config.IP[2], config.IP[3] ),  IPAddress(config.Gateway[0], config.Gateway[1], config.Gateway[2], config.Gateway[3] ) , IPAddress(config.Netmask[0], config.Netmask[1], config.Netmask[2], config.Netmask[3] ));
  }
}

/*
void WifiStatus() {
if (WiFi.status() == WL_CONNECTED) {
Serial.println(F("Hotspot Connected"));
} else {
Serial.println(F("Hotspot not Connected"));
}
}
*/

void WriteConfig()
{

  Serial.println(F("Writing Config"));
  EEPROM.write(0, 'C');
  EEPROM.write(1, 'F');
  EEPROM.write(2, 'G');

  EEPROM.write(16, config.dhcp);
  EEPROM.write(17, config.daylight);

  EEPROMWritelong(18, config.Update_Time_Via_NTP_Every); // 4 Byte

  EEPROMWritelong(22, config.timezone); // 4 Byte


  EEPROM.write(26, config.LED_R);
  EEPROM.write(27, config.LED_G);
  EEPROM.write(28, config.LED_B);

  EEPROM.write(32, config.IP[0]);
  EEPROM.write(33, config.IP[1]);
  EEPROM.write(34, config.IP[2]);
  EEPROM.write(35, config.IP[3]);

  EEPROM.write(36, config.Netmask[0]);
  EEPROM.write(37, config.Netmask[1]);
  EEPROM.write(38, config.Netmask[2]);
  EEPROM.write(39, config.Netmask[3]);

  EEPROM.write(40, config.Gateway[0]);
  EEPROM.write(41, config.Gateway[1]);
  EEPROM.write(42, config.Gateway[2]);
  EEPROM.write(43, config.Gateway[3]);


  WriteStringToEEPROM(64, config.ssid);
  WriteStringToEEPROM(96, config.password);
  WriteStringToEEPROM(128, config.ntpServerName);

  EEPROM.write(300, config.AutoTurnOn);
  EEPROM.write(301, config.AutoTurnOff);
  EEPROM.write(302, config.TurnOnHour);
  EEPROM.write(303, config.TurnOnMinute);
  EEPROM.write(304, config.TurnOffHour);
  EEPROM.write(305, config.TurnOffMinute);
  WriteStringToEEPROM(306, config.DeviceName);



  EEPROM.commit();
}
boolean ReadConfig() {

  Serial.println(F("Reading Configuration"));
  if (EEPROM.read(0) == 'C' && EEPROM.read(1) == 'F'  && EEPROM.read(2) == 'G' )
  {
    Serial.println(F("Configurarion Found!"));
    config.dhcp = 	EEPROM.read(16);

    config.daylight = EEPROM.read(17);

    config.Update_Time_Via_NTP_Every = EEPROMReadlong(18); // 4 Byte

    config.timezone = EEPROMReadlong(22); // 4 Byte

    config.LED_R = EEPROM.read(26);
    config.LED_G = EEPROM.read(27);
    config.LED_B = EEPROM.read(28);

    config.IP[0] = EEPROM.read(32);
    config.IP[1] = EEPROM.read(33);
    config.IP[2] = EEPROM.read(34);
    config.IP[3] = EEPROM.read(35);
    config.Netmask[0] = EEPROM.read(36);
    config.Netmask[1] = EEPROM.read(37);
    config.Netmask[2] = EEPROM.read(38);
    config.Netmask[3] = EEPROM.read(39);
    config.Gateway[0] = EEPROM.read(40);
    config.Gateway[1] = EEPROM.read(41);
    config.Gateway[2] = EEPROM.read(42);
    config.Gateway[3] = EEPROM.read(43);
    config.ssid = ReadStringFromEEPROM(64);
    config.password = ReadStringFromEEPROM(96);
    config.ntpServerName = ReadStringFromEEPROM(128);


    config.AutoTurnOn = EEPROM.read(300);
    config.AutoTurnOff = EEPROM.read(301);
    config.TurnOnHour = EEPROM.read(302);
    config.TurnOnMinute = EEPROM.read(303);
    config.TurnOffHour = EEPROM.read(304);
    config.TurnOffMinute = EEPROM.read(305);
    config.DeviceName = ReadStringFromEEPROM(306);
    return true;

  }
  else
  {
    Serial.println(F("Configurarion NOT FOUND!!!!"));
    return false;
  }
}

/*
**
**  NTP
**
*/

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
void NTPRefresh() {
  if (WiFi.status() == WL_CONNECTED)
  {
    IPAddress timeServerIP;
    WiFi.hostByName(config.ntpServerName.c_str(), timeServerIP);
    //sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    Serial.println(F("sending NTP packet..."));
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    UDPNTPClient.beginPacket(timeServerIP, 123);
    UDPNTPClient.write(packetBuffer, NTP_PACKET_SIZE);
    UDPNTPClient.endPacket();
    delay(1000);

    int cb = UDPNTPClient.parsePacket();
    if (!cb) {
      Serial.println(F("NTP no packet yet"));
    }
    else
    {
      Serial.print(F("NTP packet received, length="));
      Serial.println(cb);
      UDPNTPClient.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
      UnixTimestamp = epoch;
    }
  }
}

void Second_Tick()
{
  strDateTime tempDateTime;
  AdminTimeOutCounter++;
  cNTP_Update++;
  UnixTimestamp++;
  ConvertUnixTimeStamp(UnixTimestamp +  (config.timezone *  360) , &tempDateTime);
  if (config.daylight) // Sommerzeit beachten
  if (summertime(tempDateTime.year, tempDateTime.month, tempDateTime.day, tempDateTime.hour, 0))
  {
    ConvertUnixTimeStamp(UnixTimestamp +  (config.timezone *  360) + 3600, &DateTime);
  }
  else
  {
    DateTime = tempDateTime;
  }
  else
  {
    DateTime = tempDateTime;
  }
  Refresh = true;
}

int count = 0;
void UpdateTime(int toModify) {
  String min_five(F("ETA 5 min"));
  String min_four(F("ETA 4 min"));
  String min_three(F("ETA 3 min"));
  String min_two(F("ETA 2 min"));
  String min_one(F("ETA 1 min"));
  String minl(F("ETA < 1 min"));
  ++count;
  if (toModify == 0) {
    switch (count) {
      case 1:
      Parada2.status = min_five;
      break;
      case 2:
      Parada2.status = min_four;
      break;
      case 3:
      Parada2.status = min_three;
      break;
      case 4:
      Parada2.status = min_two;
      break;
      case 5:
      Parada2.status = min_one;
      break;
    }
  } else if (toModify == 1) {
    switch (count) {
      case 1:
      Parada3.status = min_five;
      break;
      case 2:
      Parada3.status = min_four;
      break;
      case 3:
      Parada3.status = min_three;
      break;
      case 4:
      Parada3.status = min_two;
      break;
      case 5:
      Parada3.status = min_one;
      break;
    }
  } else if (toModify == 2) {
    switch (count) {
      case 1:
      Parada1.status = min_five;
      break;
      case 2:
      Parada1.status = min_four;
      break;
      case 3:
      Parada1.status = min_three;
      break;
      case 4:
      Parada1.status = min_two;
      break;
      case 5:
      Parada1.status = min_one;
      break;
    }
  }
  if (count == 5) {
    tm.detach();
    if (!tm.active()) {
      switch (toModify) {
        case 0:
        Parada2.status = minl;
        break;
        case 1:
        Parada3.status = minl;
        break;
        case 2:
        Parada1.status = minl;
        break;
      }
      Serial.print(F("Removed ticker N:"));
      Serial.print(toModify);
      Serial.print(F("."));
      Serial.println(count);
      count = 0;
    } else {
      Serial.println(F("Err removing ticker!"));
      count = 0;
    }
    return;
  }
}

/*
*ID2 Parada1 15 --- Parada1 10 --- Parada1  5
*
*
*ID0 Parada2  5 --- Parada2 15 --- Parada2 10
*
*
*ID1 Parada3 10 --- Parada3  5 --- Parada3 15
*/
void UpdateDynamicData(void){
  String min_fifteen(F("ETA 15 min"));
  String min_ten(F("ETA 10 min"));
  String min_five(F("ETA 5 min"));
  String Departed(F("Departed"));
  String Arrived(F("Arrived"));
  if (
    ( (Parada1.status == Arrived)||
    (Parada2.status == Arrived)||
    (Parada3.status == Arrived) )&&
    tm.active() ) {
      tm.detach();
    }
    if (Parada1.status == Departed ){
      Parada1.status = min_fifteen;
      Parada3.status = min_ten;
      Parada2.status = min_five;
      tm.attach_ms(60000, UpdateTime, 0);
    }
    if (Parada2.status == Departed ){
      Parada2.status = min_fifteen;
      Parada1.status = min_ten;
      Parada3.status = min_five;
      tm.attach_ms(60000, UpdateTime, 1);
    }
    if (Parada3.status == Departed ){
      Parada3.status = min_fifteen;
      Parada2.status = min_ten;
      Parada1.status = min_five;
      tm.attach_ms(60000, UpdateTime, 2);
    }
  }
  #endif
