#ifndef Backend_Rfid_H
#define Backend_Rfid_H

#define RST_PIN  0  // RST-PIN für RC522 - RFID - SPI - Modul GPIO5
#define SS_PIN  15  // SDA-PIN für RC522 - RFID - SPI - Modul GPIO4
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
//bool tagRead = false;
bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;

/* wiring the MFRC522 to ESP8266 (ESP-12)
  RST     = GPIO5
  SDA(SS) = GPIO4
  MOSI    = GPIO13
  MISO    = GPIO12
  SCK     = GPIO14
  GND     = GND
  3.3V    = 3.3V
*/

bool Tag_checker (void) {
  if (( mfrc522.uid.uidByte[0] == 0x70 &&
        mfrc522.uid.uidByte[1] == 0x71 &&
        mfrc522.uid.uidByte[2] == 0xe1 &&
        mfrc522.uid.uidByte[3] == 0xa4)
      ||
      ( mfrc522.uid.uidByte[0] == 0x71 &&
        mfrc522.uid.uidByte[1] == 0xB0 &&
        mfrc522.uid.uidByte[2] == 0xB0 &&
        mfrc522.uid.uidByte[3] == 0x2E)
      ||
      ( mfrc522.uid.uidByte[0] == 0xC5 &&
        mfrc522.uid.uidByte[1] == 0x57 &&
        mfrc522.uid.uidByte[2] == 0xF6 &&
        mfrc522.uid.uidByte[3] == 0x2D)) {

    return true;

  }
  return false;
}

void Tag_reader(void) {
  /*
  * Taken from https://github.com/miguelbalboa/rfid/wiki/Useful-code-snippets
  * Credits: https://github.com/metamorphious
  */
  rfid_tag_present_prev = rfid_tag_present;

  _rfid_error_counter += 1;
  if(_rfid_error_counter > 2){
    _tag_found = false;
  }

  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfrc522.PCD_WriteRegister(mfrc522.TxModeReg, 0x00);
  mfrc522.PCD_WriteRegister(mfrc522.RxModeReg, 0x00);
  // Reset ModWidthReg
  mfrc522.PCD_WriteRegister(mfrc522.ModWidthReg, 0x26);

  MFRC522::StatusCode result = mfrc522.PICC_RequestA(bufferATQA, &bufferSize);

  if(result == mfrc522.STATUS_OK){
    if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
      return;
    }
    _rfid_error_counter = 0;
    _tag_found = true;
  }

  rfid_tag_present = _tag_found;

  // rising edge
  if ((rfid_tag_present && !rfid_tag_present_prev) && Tag_checker){
    Serial.println(F("RFID Card found"));
    parada.status = (F("Arrived"));
    WebClient("http://192.168.0.28/", config.DeviceName, parada.status);
  }

  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev){
    Serial.println(F("RFID Card Lost"));
    parada.status = (F("Departed"));
    WebClient("http://192.168.0.28/", config.DeviceName, parada.status);
  }
}

/*
void Tag_reader(void) {
  if ( (! mfrc522.PICC_ReadCardSerial()) && tagRead) {
    Serial.println(F("RFID Card Lost"));
    parada.status = (F("Departed"));
    WebClient("http://192.168.0.28/", config.DeviceName, parada.status);
    tagRead = false;
    return;
  }
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // If the card is present and the Card is redable
  if (Tag_checker) {
    tagRead = true;
    Serial.println(F("RFID Card found"));
    parada.status = (F("Arrived"));
    WebClient("http://192.168.0.28/", config.DeviceName, parada.status);
    delay(2500); //Avoid Overflow of strings on the times variable
  }

      //Show some details of the PICC (that is: the tag/card)
       //Serial.print(F("Card UID:"));
       //dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
       //Serial.println();

}
*/
#endif
