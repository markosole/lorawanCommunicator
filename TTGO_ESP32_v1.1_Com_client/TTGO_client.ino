/////////////////////////////////////////////////////////////////
// .d8888. d88888b d8b   db d8888b. d88888b d8888b. 
// 88'  YP 88'     888o  88 88  `8D 88'     88  `8D 
// `8bo.   88ooooo 88V8o 88 88   88 88ooooo 88oobY' 
//   `Y8b. 88~~~~~ 88 V8o88 88   88 88~~~~~ 88`8b   
// db   8D 88.     88  V888 88  .8D 88.     88 `88. 
// `8888Y' Y88888P VP   V8P Y8888D' Y88888P 88   YD 

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
uint32_t chipId = 0;


TinyGPSPlus gps;
int transmissionState = ERR_NONE;
volatile bool transmittedFlag = false;
volatile bool enableInterrupt = true;
unsigned long gpsLastMillis;
StaticJsonDocument<512> doc;

void setFlag(void)
{
  if (!enableInterrupt) {
    return;
  }
  transmittedFlag = true;
}

void setup()
{
  initBoard();
  
  // Get ESP32 chip ID
  for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}

	Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
	Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  Serial.print("Chip ID: "); Serial.println(chipId);


  // When the power is turned on, a delay is required.
  delay(1500);

  Serial.print(F("[SX1278] Initializing ... "));
 

  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("Lora started: OK!");

  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
   
  
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(64 , 0 , "LoraWan");
  display.drawString(64 , 20 , "Sender");
  display.drawString(64 , 40 , "Standby");
  display.display();
}


void loop()
{
    delay(1000);
    fetchingGPS();

    enableInterrupt = true;

}

void fetchingGPS() {
  doc.clear();
  doc["appid"] = "LoRa_GPS";
  doc["system"]["Name"] = chipId;
  doc["system"]["RSSI"] = rssi;
  doc["system"]["SNR"] = packSize; //"5678";
  doc["system"]["Status"] = "ok";
  doc["system"]["Battery"]["Connection"] = PMU.isBatteryConnect();
  doc["system"]["Battery"]["Voltage"] = String((PMU.getBattVoltage() / 1000), 2);
  doc["location"]["lat"] = gps.location.lat();
  doc["location"]["lng"] = gps.location.lng();
  doc["satellites"] = gps.satellites.value();
  doc["altitude"] =  String((gps.altitude.feet() / 3.2808), 2);
  char gpsDateTime[20];
  sprintf(gpsDateTime, "%04d-%02d-%02d %02d:%02d:%02d",
          gps.date.year(),
          gps.date.month(),
          gps.date.day(),
          gps.time.hour(),
          gps.time.minute(),
          gps.time.second());
  doc["gpsDateTime"] = gpsDateTime;

  // ***** Display data
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  
  display.drawString(64, 0, "Client");
  display.drawString(64, 20, String(chipId));
  display.drawString(64, 40, String(gps.satellites.value()));
  
  display.display();
  // ***** Display data

  String output;
  serializeJson(doc, output);
  Serial.println(output);
  // transmissionState = radio.startTransmit(output);
  
  // Start transmission
  LoRa.beginPacket();
  LoRa.print(output);
  LoRa.endPacket();
  
  smartDelay(1000);
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}
