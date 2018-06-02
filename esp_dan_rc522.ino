#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 8
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];

String ssid ="HCB100";
String password="PingPong0716";
String data;
String server = "192.168.8.102"; // www.example.com
String uri = "/project/save.php";// our example is /esppost.php
String str_uid;

void setup() {
  Serial1.begin(115200);
  Serial.begin(115200);
  Serial.println("SETUP SERIAL");
  //--
  Serial.println("Setup RC522");
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  //----
  Serial.println();
  Serial.println("SETUP WIFI");
  reset();
  connectWifi();
}

void reset() {
  Serial.println("ON RESET");
  Serial1.println("AT+RST");
  delay(1000);
  if(Serial1.find("OK") ) Serial.println("Module Reset");
}


void connectWifi() {
  Serial.println("ON CONNECT");
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  Serial1.println(cmd);
  while(!Serial1.available()){
    Serial.println("waiting response to AP");
    delay(1000);
  }
  //delay(3000);
  if(Serial1.find("OK")) {
    Serial.println("Connected!");
  }else {
    connectWifi();
    Serial.println("Cannot connect to wifi"); 
   }
}



void loop () {
  str_uid = "";
  Serial.println("ON LOOP");
  delay(5000);
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();

    
  }else 
    Serial.println(F("Card read previously."));


  if(str_uid.equals("")){
    
  }else{
    //----
    data = "uid=" + str_uid ;
    Serial.println(data);
    httppost();
    delay(1000);
    //----
  }
  
    
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void httppost () {
  Serial1.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
  //--
  while(!Serial1.available()){
    Serial.println("Waiting TCP ready...");
  }
  //--
  if( Serial1.find("OK")) {
    Serial.println("TCP connection ready");
  } 
  delay(1000);
  String postRequest =
  "POST " + uri + " HTTP/1.0\r\n" +
  "Host: " + server + "\r\n" +
  "Accept: *" + "/" + "*\r\n" +
  "Content-Length: " + data.length() + "\r\n" +
  "Content-Type: application/x-www-form-urlencoded\r\n" +
  "\r\n" + data;
  String sendCmd = "AT+CIPSEND=" + (String) postRequest.length();//determine the number of caracters to be sent.
  //Serial1.print(sendCmd);
  Serial1.println(sendCmd );
  //delay(500);
  //--
  while(!Serial1.available()){
    Serial.println("Waiting for cnnection to server is established...");
  }
  //--
  if(Serial1.find(">")) {
    Serial.println("Sending.."); 
    Serial1.print(postRequest);
    if( Serial1.find("SEND OK")) { 
      Serial.println("Packet sent");
      while(!Serial1.available()){
        Serial.println("Loading web server response");
      }
      while (Serial1.available()) {
        String tmpResp = Serial1.readString();
        Serial.println(tmpResp);
      }
      //Serial1.println("AT+CIPCLOSE");
    }
  }
  Serial1.println("AT+CIPCLOSE");
  delay(1000);
  Serial.flush();
  Serial1.flush();
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
    str_uid += (String)buffer[i];
  }
}
