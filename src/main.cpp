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
#define MODBUS_DE_PIN 18 // connect DR, RE pin of MAX485 to gpio 4
#define MODBUS_RX_PIN 16 // Rx pin 18 of ESP32 connect to RO pin of MAX485
#define MODBUS_TX_PIN 17 // Tx pin 19 of ESP32 connect to DI pin of MAX485
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

int intervalReading = 2000;
int lastReading = 0;


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
         const sel = document.querySelector.bind(document);

const canvas = sel('canvas');
const c = canvas.getContext('2d');

const W = canvas.width;
const H = canvas.height;
const MIDX = Math.floor(W/2);
const MIDY = Math.floor(H/3);
const MINSPEED = 180;
const MAXSPEED = 340;
let prev = null;
let speed = 250;

requestAnimationFrame(updateMeter);

function updateMeter(curr)
{
  
  meter(speed);

  requestAnimationFrame(updateMeter);
}

//meter(280, 100, 100);

function meter(angle = 280, centerX = MIDX, centerY = MIDY, radius = 100)
{
  c.fillStyle = '#444444';
  c.fillRect(0, 0, W, H);

  spokes(centerX, centerY, 5, 85, 5, 345, 185, 2, '#ffff00')
  spokes(centerX, centerY);
  finalLine(angle, '#ff0000', 4, centerX, centerY);
  arc(centerX, centerY, '#ff00ff', 3, 180, 350, radius - 80, 'butt');
  arc(centerX, centerY, '#00ACC1', 3, 180, 350, radius - 85, 'butt');
  arc(centerX, centerY, '#ff00ff', 8, 180, 340, radius);
  arc(centerX, centerY, '#ffff00', 5, 180, angle, radius);
  c.font = '1.6rem Roboto';
  c.fillStyle = '#ffffff';
  c.fillText(Math.floor(angle), centerX - 20, centerY + 30);
}

function d(x){ return (Math.PI / 180) * x; }

function finalLine(angle = 340, color = '#ff0000', width = 4, centerX = MIDX, centerY = MIDY)
{
  let p1 = polarToCartesian(15, angle);
  let p2 = polarToCartesian(100, angle);
  c.beginPath();
  c.strokeStyle = color;
  c.lineWidth = width;
  c.moveTo(centerX + p1.x, centerY +  p1.y);
  c.lineTo(centerX + p2.x, centerY +  p2.y);
  c.stroke();
}

function arc(centerX = MIDX, centerY = MIDY, arcColor = '#ff00ff', arcWidth = 8, startAngle = 180, endAngle = 340, radius = MIDX - 100, arcCap = 'round')
{
  c.beginPath();
  c.strokeStyle = arcColor;
  c.lineWidth = arcWidth;
  c.lineCap = arcCap;
  c.arc(centerX, centerY, radius, d(startAngle), d(endAngle), false);
  c.stroke();
}

function polarToCartesian(radius, angle)
{
  return {x: radius * Math.cos(d(angle)), y: radius * Math.sin(d(angle))};
}

function spokes(centerX = MIDX, centerY = MIDY, stepAngle = 20, radius = 80, spokeLength = 10, last = 350, startAngle = 180, spokeWidth = 4, spokeColor = '#00ffff')
{
  c.beginPath();
  c.strokeStyle = spokeColor;
  c.lineWidth = spokeWidth;
  c.lineCap = 'butt';

  for(let currAngle = startAngle; currAngle < last; currAngle += stepAngle)
  {
    let p1 = polarToCartesian(radius, currAngle);
    c.moveTo(centerX + p1.x, centerY + p1.y);
    let p2 = polarToCartesian(radius + spokeLength, currAngle);
    c.lineTo(centerX + p2.x, centerY + p2.y);
  }
  c.stroke();
}
              
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
       
        String clockButton = "<button id=\"button_clock\" onclick=adjustClock()>Atualizar opções</button>";      
                
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
        node1.postTransmission(modbusPostTransmission);
        node2.preTransmission(modbusPreTransmission);
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
       
        result = node1.readHoldingRegisters(0x0000, 2);
        if (result==node1.ku8MBSuccess){
          Serial.print("Wind Speed=  ");
          for (int i = 0; i < 2; i ++){
            data[i] = node1.getResponseBuffer(i);
            Serial.print(data[i]);
          }
          Serial.println();
        }
        

        result = node2.readHoldingRegisters(0x0000, 2);
        if (result==node2.ku8MBSuccess){
          Serial.print("Wind Direction=  ");
          for (int i = 0; i < 2; i ++){
            data2[i] = node2.getResponseBuffer(i);
            Serial.println(data2[i]);
          }
          Serial.println();
        }        
      
  }