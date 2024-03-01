//
// ANEMOMETER AND WIND DIRECTION POINTER FOR AGRICULTURAL DRONE
// 20240212

#include <Arduino.h>
// github link: https://github.com/4-20ma/ModbusMaster
#include <ModbusMaster.h>
#include <ModbusRTUMaster.h>
#include <SPI.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Preferences.h> // EEPROM library



/* Modbus stuff */
#define MODBUS_DE_PIN  4 // connect DR, RE pin of MAX485 to gpio 4
#define MODBUS_RX_PIN 18 // Rx pin 18 of ESP32 connect to RO pin of MAX485
#define MODBUS_TX_PIN 19 // Tx pin 19 of ESP32 connect to DI pin of MAX485
#define MODBUS_SERIAL_BAUD 4800 // Baud rate for esp32 and max485 communication


ModbusMaster node1;
ModbusMaster node2;

AsyncWebServer server(80);

uint16_t inputSpeed[2]; // array to storage the holding registers of anemometer
uint16_t inputDirection[2]; // array to storage holding registers of wind direction

int timeOutHere = 200;


// Pin 4 made high for Modbus transmision mode
void modbusPreTransmission()
      {
        digitalWrite(MODBUS_DE_PIN, HIGH);
      }
  // Pin 4 made low for Modbus receive mode
void modbusPostTransmission()
      {
        digitalWrite(MODBUS_DE_PIN, LOW);
      }


void setup()
      {
        //  esp serial communication
        Serial.begin(115200);
        pinMode(MODBUS_DE_PIN, OUTPUT);
        digitalWrite(MODBUS_DE_PIN, LOW);

        //Serial2.begin(baud-rate, protocol, RX pin, TX pin);
        Serial2.begin(MODBUS_SERIAL_BAUD, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
        node1.begin(1, Serial2);
        node2.begin(2, Serial2);  

        //calbacks
        node1.preTransmission(modbusPreTransmission);
        node1.postTransmission(modbusPreTransmission);
        node2.preTransmission(modbusPostTransmission);
        node2.postTransmission(modbusPostTransmission);

        Serial.println("end of setup");
        
        } 


void loop()
{
        uint8_t result;
        uint16_t data[6];
        uint16_t data2[6];

        result = node1.readHoldingRegisters(0x000, 2);
        if (result==node1.ku8MBSuccess){
          Serial.print("Wind Speed=  ");
          for (int i = 0; i < 2; i ++){
            data[i] = node1.getResponseBuffer(i);
            Serial.print(data[i]);
          }
          Serial.println();
        }

        delay(timeOutHere);

        result = node2.readHoldingRegisters(0x000, 2);
        if (result==node2.ku8MBSuccess){
          Serial.print("Wind Direction=  ");
          for (int i = 0; i < 2; i ++){
            data2[i] = node2.getResponseBuffer(i);
            Serial.println(data2[i]);
          }
          Serial.println();
        }
      
  }