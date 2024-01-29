#include <Arduino.h>
// github link: https://github.com/4-20ma/ModbusMaster
#include <ModbusMaster.h>

/* Modbus stuff */
#define MODBUS_DIR_PIN  4 // connect DR, RE pin of MAX485 to gpio 4
#define MODBUS_RX_PIN 18 // Rx pin  
#define MODBUS_TX_PIN 19 // Tx pin 
#define MODBUS_SERIAL_BAUD 9600 // Baud rate for esp32 and max485 communication

// voltage, current and frequency data register of DDM18SD
uint16_t data_register[1] = {0x0000};

//Initialize the ModbusMaster object as node1
ModbusMaster node1;
ModbusMaster node2;

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
  Serial2.setTimeout(200);
  //modbus slave ID 14
  node1.begin(1, Serial2);
  node2.begin(2, Serial2);

//  callbacks allow us to configure the RS485 transceiver correctly
   node1.preTransmission(modbusPreTransmission);
   node1.postTransmission(modbusPostTransmission);
   node2.preTransmission(modbusPreTransmission);
   node2.postTransmission(modbusPostTransmission);
  
  }

void loop()
{
    uint8_t result;
    uint16_t data[2];
    int i;    
    float reading;

    uint8_t result2;
    uint16_t data2[2];
    int j;
    float reading2;

      
      //Modbus function 0x03 Read Holding Registers according to energy meter datasheet
      result = node1.readHoldingRegisters(0x0000, 0x0001);
        if (result == node1.ku8MBSuccess) {
          Serial.println("Success, Received data: ");
          
          //Retrieve the data from getResponseBuffer(uint8_t u8Index) function.
          //that is return 16-bit data. our energy meter return 32-bit data everytime.
          //that's why, we take the value to a array called data
           data[0]=node1.getResponseBuffer(0x00);
           data[1]=node1.getResponseBuffer(0x01);
           
           //read voltage          
            Serial.print("Wind speed: ");
            // we just convert the uint16_t type data array to float type using type casting
            reading = *((float *)data);
            Serial.print(reading/10);
            Serial.println(" m/s");
           
           
        } else {
          Serial.print("Failed, Response Code: ");
          Serial.print(result, HEX);
          Serial.println("");
          delay(5000); 
        }

    result2 = node2.readHoldingRegisters(3, 1);
        if (result2 == node2.ku8MBSuccess) {
          Serial.println("Success, Received data: ");
          
          //Retrieve the data from getResponseBuffer(uint8_t u8Index) function.
          //that is return 16-bit data. our energy meter return 32-bit data everytime.
          //that's why, we take the value to a array called data
           data2[0]=node2.getResponseBuffer(0x00);
           data2[1]=node2.getResponseBuffer(0x01);
           
           //read voltage          
            Serial.print("Wind direction: ");
            // we just convert the uint16_t type data array to float type using type casting
            reading2 = *((float *)data2);
            Serial.print(reading2);
            Serial.println(" grades");
           
           
        } else {
          Serial.print("Failed, Response Code: ");
          Serial.print(result, HEX);
          Serial.println("");
          delay(5000); 
        } 
    
    delay(1000);
  }