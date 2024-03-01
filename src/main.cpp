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

Preferences prefs;

// Replace with your network credentials
const char* ssid = "XXXXXXXXXX-2024";
const char* password = "XXXXXXXXXXX";
//IPAddress localIp(192,168,4,1);

uint16_t inputSpeed[2]; // array to storage the holding registers of anemometer
uint16_t inputDirection[2]; // array to storage holding registers of wind direction

int timeOutHere = 200;

String windLimit = "";
String humidityLimit = "";
String temperatureLimit = "";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" type="text/css" href="estilo.css">
    <link href="https://fonts.googleapis.com/css?family=Prompt&display=swap" rel="stylesheet">
    <link rel="shortcut icon" href="imagens/agro3.ico" type="image/vnd.microsoft.icon">    
    <title>AgroexPerto</title>
    <style type="text/css">

</style>
</head>
  <body>
    <!-- partial:index.partial.html -->
<canvas width="400" height="600"></canvas>
<!-- partial -->
    <div class="div1"><br>
    %PLACEHOLDER%
    <script>
          function ...
              
    </script>
    <br><br></div>    
  </body>
</html>
)rawliteral";


String processor(const String& var){
  if(var == "PLACEHOLDER"){
    String currentProgram ="";    
    
    currentProgram = "<br><table><tr><td>Temperatura</td><td>"+temperatureLimit+" ºC</td></tr>"+
                                  "<tr><td>Fotoperíodo</td><td>"+windLimit+" horas</td></tr>"+
                                  "<tr><td>Número de imersões diárias</td><td>"+humidityLimit+"</td></tr>";

    String imersionHours = "<br><table><tr>  /tr></table><br><br>";
       
        String clockButton = "<button id=\"button_clock\" onclick=adjustClock()>Atualizar relógio</button>";      
                
        currentProgram = currentProgram + imersionHours;
        
        Serial.println(currentProgram);

    return currentProgram;
  }
  return String();
}

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

        WiFi.softAP(ssid, password);
        Serial.println(WiFi.softAPIP());

        //Serial2.begin(baud-rate, protocol, RX pin, TX pin);
        Serial2.begin(MODBUS_SERIAL_BAUD, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
        node1.begin(1, Serial2);
        node2.begin(2, Serial2);  

        //calbacks de RS485
        node1.preTransmission(modbusPreTransmission);
        node1.postTransmission(modbusPreTransmission);
        node2.preTransmission(modbusPostTransmission);
        node2.postTransmission(modbusPostTransmission);
        

        //callbacks de webserver
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){        
                request->send_P(200, "text/html", index_html, processor); //processor
                  });
        server.on("/adjustClock", HTTP_GET, [](AsyncWebServerRequest *request){
            if (request->hasParam("year")) {
                  windLimit = request->getParam("wind_speed_limt")->value();
                  humidityLimit = request->getParam("UR_limt")->value();
                  temperatureLimit = request->getParam("UR_limt")->value();

                  int first = windLimit.toInt();
                  int second = humidityLimit.toInt();
                  int third = temperatureLimit.toInt();
                  
                  Serial.print(first);
                  Serial.print(" : ");
                  Serial.println(third);                  

            request->send_P(200, "text/html", index_html, processor); //processor
          }});



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