//
// ANEMOMETER AND WIND DIRECTION POINTER FOR AGRICULTURAL DRONE
// 20240212

#include <Arduino.h>
// github link: https://github.com/4-20ma/ModbusMaster
#include <ModbusMaster.h> // só serve para dados de 32 bits
#include <ModbusRTUMaster.h>
#include <SPI.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Preferences.h> // EEPROM library


/* Modbus stuff */
#define MODBUS_DIR_PIN  4 // connect DR, RE pin of MAX485 to gpio 4
#define MODBUS_RX_PIN 18 // Rx pin 18 of ESP32 connect to RO pin of MAX485
#define MODBUS_TX_PIN 19 // Tx pin 19 of ESP32 connect to DI pin of MAX485
#define MODBUS_SERIAL_BAUD 4800 // Baud rate for esp32 and max485 communication


ModbusRTUMaster modbus(Serial2, MODBUS_DIR_PIN); // serial port, driver enable pin for rs-485 (optional)

uint16_t inputSpeed[2]; // array to storage the holding registers of anemometer
uint16_t inputDirection[2]; // array to storage holding registers of wind direction



// Pin 4 made high for Modbus transmision mode
void modbusPreTransmission()
{
  delay(500);
  digitalWrite(MODBUS_DIR_PIN, HIGH);
}
// Pin 4 made low for Modbus receive mode
void modbusPostTransmission()
{
  digitalWrite(MODBUS_DIR_PIN, LOW);
  delay(500);
}


void setup()
      {
        //  esp serial communication
        Serial.begin(115200);
        pinMode(MODBUS_DIR_PIN, OUTPUT);
        digitalWrite(MODBUS_DIR_PIN, LOW);

        //Serial2.begin(baud-rate, protocol, RX pin, TX pin);.
        Serial2.begin(MODBUS_SERIAL_BAUD, SERIAL_8E1, MODBUS_RX_PIN, MODBUS_TX_PIN);
        Serial2.setTimeout(2000);
        modbus.begin(4800);   

        Serial.println("end of setup");
        
        }

  void sendModBus (){
          modbusPreTransmission();    
        uint8_t modBusCommand[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};
        for (int i=0; i<=8; i++){
          Serial2.print(modBusCommand[i]);
        }
        modbusPostTransmission();
        Serial.println("command sent");
        }

  void receiveModBus(){
        if (Serial2.available()){
          char dataBit = Serial2.read();      
          uint8_t modBusResponse[] = {};
        }
      }

void loop()
{
    // slave id, starting data address,
    // unsigned 16 bit integer array to place input register values, number of input registers to read:
    modbus.readInputRegisters(1, 0, inputSpeed, 2); 
    modbus.readInputRegisters(2, 0, inputDirection, 2);

    Serial.print("Wind speed: ");
    Serial.println(inputSpeed[0]);
    Serial.println(*((float *)inputSpeed[0]));
    Serial.print("Wind direction: ");
    Serial.println(inputDirection[1]);
    Serial.println(*((float *)inputDirection[1]));                                                        


  }