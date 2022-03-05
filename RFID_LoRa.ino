#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "HX711.h"

#define LORA_SCK     5    
#define LORA_MISO    19   
#define LORA_MOSI    27 
#define LORA_SS      18  
#define LORA_RST     14   
#define LORA_DI0     26  
#define LORA_BAND    915E6

#include <SPI.h>
#include <MFRC522.h>

#define RFID_SDA 5 
#define RFID_SCK 18 
#define RFID_MOSI 23
#define RFID_MISO 19
#define RFID_RST 22

//Pinos da HX711
#define DOUT  15
#define CLK  4
#define calibration_factor -9050.0 //Fator de Calibração da HX711

MFRC522 mfrc522(RFID_SDA, RFID_RST);  // Create MFRC522 instance
HX711 scale;

int current_spi = -1; // -1 - NOT STARTED   0 - RFID   1 - LORA
float peso1;
char *Bovinos[]= {"Bovino1", "Bovino2"};

void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
}

void loop() {
  bool card_present = RFID_check();
  if (card_present)
   LORA_send();
}

void spi_select(int which) {
     if (which == current_spi) return;
     SPI.end();
     
     switch(which) {
        case 0:
          SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI);
          mfrc522.PCD_Init();   
        break;
        case 1:
          SPI.begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_SS);
          LoRa.setPins(LORA_SS,LORA_RST,LORA_DI0);
        break;
     }

     current_spi = which;
}

int RFID_check() {
  spi_select(0);
  // Look for new cards
 if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()){ //VERIFICA SE O CARTÃO PRESENTE NO LEITOR É DIFERENTE DO ÚLTIMO CARTÃO LIDO. CASO NÃO SEJA, FAZ
    return false;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
    /***INICIO BLOCO DE CÓDIGO RESPONSÁVEL POR GERAR A TAG RFID LIDA***/
  String strID = ""; 
  for (byte i = 0; i < 4; i++) {
    strID +=
    (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(mfrc522.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  strID.toUpperCase();
/***FIM DO BLOCO DE CÓDIGO RESPONSÁVEL POR GERAR A TAG RFID LIDA***/

if (strID.indexOf("96:9F:01:94") >= 0) { //SE O ENDEREÇO DA TAG LIDA FOR IGUAL AO ENDEREÇO INFORMADO, FAZ
  peso1=scale.get_units(20);
  }
  return true;
}

void LORA_send() {
  spi_select(1);
  
  Serial.println("LoRa Sender Test"); 
  
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("init ok");
 
  delay(1500);

  // send packet
  LoRa.beginPacket();
  LoRa.print(Bovinos[1]);
  LoRa.print(peso1);
  LoRa.endPacket();

  Serial.println(peso1);
  delay(2000);                       // wait for a second
 }
  
