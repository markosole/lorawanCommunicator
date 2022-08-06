/////////////////////////////////////////////////////////////////
// d8888b. d88888b  .o88b. d88888b d888888b db    db d88888b d8888b. 
// 88  `8D 88'     d8P  Y8 88'       `88'   88    88 88'     88  `8D 
// 88oobY' 88ooooo 8P      88ooooo    88    Y8    8P 88ooooo 88oobY' 
// 88`8b   88~~~~~ 8b      88~~~~~    88    `8b  d8' 88~~~~~ 88`8b   
// 88 `88. 88.     Y8b  d8 88.       .88.    `8bd8'  88.     88 `88. 
// 88   YD Y88888P  `Y88P' Y88888P Y888888P    YP    Y88888P 88   YD 

// TTGO ESP32 Lora module V1.1 with SX1278 chip, frequency 433 MHz. NOTE: This won't work on model with SX126x models

#include <TinyGPS++.h>                       
#include <ArduinoJson.h>
#include <LoRa.h>
#include "boards.h"
#define   ERR_NONE   0
#include <SPI.h>
#include "SSD1306.h" 
#include "images.h"

// Define settings for Lora module
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    433E6 //915E6

// Define display
SSD1306 display(0x3c, 21, 22);

String rssi = "RSSI --";
String packSize = "--";
String packet ;

// added for Lora when data is received
void loraData(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  //display.drawString(64 , 20 , "Received "+ packSize + " bytes");
  display.drawStringMaxWidth(64 , 40 , 128, packet);
  display.drawString(64, 0, rssi); 
  display.display();
  // Print Data on Serial port
  // ToDo: generate JSON data
  Serial.println(packet);
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  loraData();
}


void setup()
{

  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH);   // while OLED is running, must set GPIO16 in high、

  delay(1500);

  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Receiver initializing...");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  
  //if (!LoRa.begin(915E6)) {
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.receive();
  Serial.println("Initialized: ok");
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
   
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(64 , 0 , "GPS Server");
  display.drawString(64 , 20 , "Receiver");
  display.drawString(64 , 40 , "Ready!");
  display.display();
  
  delay(1500);
    
}


void loop()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Serial.println("Lora data received!");
    cbk(packetSize);  
  }
  delay(10);
}

