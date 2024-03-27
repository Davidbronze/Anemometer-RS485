//
// ANEMOMETER AND WIND DIRECTION POINTER FOR AGRICULTURAL DRONE
// 20240212

#include <Arduino.h>
// github link: https://github.com/4-20ma/ModbusMaster
#include <ModbusMaster.h>
#include <SPI.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Preferences.h> // EEPROM library
#include <Arduino_JSON.h>


/* Modbus stuff */
#define MODBUS_DE_PIN 18 // connect DR, RE pin of MAX485 to gpio 4
#define MODBUS_RX_PIN 16 // Rx pin 18 of ESP32 connect to RO pin of MAX485
#define MODBUS_TX_PIN 17 // Tx pin 19 of ESP32 connect to DI pin of MAX485
#define MODBUS_SERIAL_BAUD 4800 // Baud rate for esp32 and max485 communication


ModbusMaster node1;
ModbusMaster node2;

AsyncWebServer server(80);

AsyncEventSource events("/events");

JSONVar readings;

Preferences prefs;

// Replace with your network credentials
const char* ssid = "XXXXXXXXXX-2024";
const char* password = "XXXXXXXXXXX";
//IPAddress localIp(192,168,4,1);

uint16_t inputSpeed[2]; // array to storage the holding registers of anemometer
uint16_t inputDirection[2]; // array to storage holding registers of wind direction

int intervalReading = 5000;
int lastTime = 0;

uint8_t result;
uint16_t data[6];
uint16_t data2[6];

String PLACEHOLDER = "";
String currentProgram ="";
String windLimit = "";
String humidityLimit = "";
String temperatureLimit = "";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en" >
<head>  
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
    
<script>
    window.addEventListener('load', getReadings);    
    
    const sel = document.querySelector.bind(document);
    const canvas = sel('canvas');
    const cv = canvas.getContext('2d');
    const cv2 = canvas.getContext('2d');
    const W = canvas.width;
    const H = canvas.height;
    const MIDX = Math.floor(W/2);
    const MIDY = Math.floor(H/2);   
    var temp;
    var hum;
    var speed;
    var direction;  

    meter(speed);
    compass(direction);
    requestAnimationFrame(updateMeter);

function compass(azim = 10, centerX = MIDX, centerY = (3.3*MIDY/2), radius = 50){
  var azimReading = 0;
    if(azim<90){
      azimReading = azim + 270;
      }
      else{
        azimReading = azim - 90;
      }
  arcCompass(centerX, centerY,'#00FF7F', 4, 0, 360, radius, 'butt');  
  compassLine(azimReading, '#FF0000', 4, centerX, centerY);
  arcCompass(centerX, centerY,'#00FF7F', 8, 0, 360, 4, 'butt');
  cv2.font = '2rem Roboto';
  cv2.fillStyle = '#ffffff';
  cv2.fillText(azim, centerX, centerY + 90);
  cv2.fillText('  °', centerX+20, centerY + 90);  
}

function meter(angle = 250, centerX = MIDX, centerY = MIDY, radius = 100)
{  
  var speedReading = 0;
    if (angle<20){
      speedReading = (angle*5.7)+180;
    }
    else if(angle>20 & angle<100){
      speedReading = (angle*0.7)+280;
    }
    else{speedReading = 340;}

  cv.fillStyle = '#444444';
  cv.fillRect(0, 0, W, H);

  //spokes(centerX, centerY, 5, 85, 5, 345, 185, 2, '#ffff00')
  spokes(centerX, centerY);
  arc(centerX, centerY, '#00FF7F', 20, 180, 340, radius, 'butt');
  finalLine(speedReading, '#ff0000', 8, centerX, centerY);
  arc(centerX, centerY, '#0000FF', 3, 180, 350, radius - 80, 'butt');
  arc(centerX, centerY, '#00ACC1', 3, 180, 350, radius - 85, 'butt');  
  //arc(centerX, centerY, '#ffff00', 5, 180, angle, radius);
  cv.font = '2rem Roboto';
  cv.fillStyle = '#ffffff';
  cv.fillText((Math.floor(angle)), centerX, centerY + 60);
  cv.font = '2rem Roboto';
  cv.fillStyle = '#ffffff';
  cv.fillText('km/h', centerX - 25, centerY + 90);
}


function compassLine(azim=270, color = '#1E90FF', width = 4, centerX = MIDX, centerY = (3.3*MIDY/2)){
  let p1 = polarToCartesian(5, azim+100);
  let p2 = polarToCartesian(40, azim);
  let p3 = polarToCartesian(5, azim-100);
  let p4 = polarToCartesian(-40, azim);

    cv2.beginPath();
    cv2.strokeStyle = color;
    cv2.lineWidth = width;
    cv2.lineCap = 'butt';
    cv2.moveTo(centerX + p1.x, centerY +  p1.y);
    cv2.lineTo(centerX + p2.x, centerY +  p2.y);
    cv2.stroke();
  cv2.beginPath();
  cv2.strokeStyle = color;
  cv2.lineWidth = width;
  cv2.lineCap = 'butt';
  cv2.moveTo(centerX + p3.x, centerY +  p3.y);
  cv2.lineTo(centerX + p2.x, centerY +  p2.y);
  cv2.stroke();
    cv2.beginPath();
    cv2.strokeStyle = '#DCDCDC';
    cv2.lineWidth = width;
    cv2.lineCap = 'butt';
    cv2.moveTo(centerX + p1.x, centerY +  p1.y);
    cv2.lineTo(centerX + p4.x, centerY +  p4.y);
    cv2.stroke();
  cv2.beginPath();
  cv2.strokeStyle = '#DCDCDC';
  cv2.lineWidth = width;
  cv2.lineCap = 'butt';
  cv2.moveTo(centerX + p3.x, centerY +  p3.y);
  cv2.lineTo(centerX + p4.x, centerY +  p4.y);
  cv2.stroke();
}

function finalLine(angle = 340, color = '#ff0000', width = 4, centerX = MIDX, centerY = MIDY)
{
  let p1 = polarToCartesian(15, angle);
  let p2 = polarToCartesian(100, angle);
  cv.beginPath();
  cv.strokeStyle = color;
  cv.lineWidth = width;
  cv.moveTo(centerX + p1.x, centerY +  p1.y);
  cv.lineTo(centerX + p2.x, centerY +  p2.y);
  cv.stroke();
}

function arc(centerX = MIDX, centerY = MIDY, arcColor = '#ff00ff', arcWidth = 8, startAngle = 300, endAngle = 340, radius = MIDX - 100, arcCap = 'round')
{
  const gradient = cv.createLinearGradient(0,100,300,200);
  gradient.addColorStop("0.5", "#00FF7F");
  gradient.addColorStop("0.8", "yellow");
  gradient.addColorStop("1", "red");
  cv.beginPath();
  cv.strokeStyle = gradient;
  cv.lineWidth = arcWidth;
  cv.lineCap = arcCap;
  cv.arc(centerX, centerY, radius, d(startAngle), d(endAngle), false);
  cv.stroke();
}

function arcCompass(centerX = MIDX, centerY = MIDY, arcColor = '#ff00ff', arcWidth = 8, startAngle = 300, endAngle = 340, radius = MIDX - 100, arcCap = 'round')
{  
  cv.beginPath();
  cv.strokeStyle = arcColor;
  cv.lineWidth = arcWidth;
  cv.lineCap = arcCap;
  cv.arc(centerX, centerY, radius, d(startAngle), d(endAngle), false);
  cv.stroke();
}

function polarToCartesian(radius, angle)
{
  return {x: radius * Math.cos(d(angle)), y: radius * Math.sin(d(angle))};
}

function d(x){ return (Math.PI / 180) * x; }

function spokes(centerX = MIDX, centerY = MIDY, stepAngle = 20, radius = 80, spokeLength = 10, last = 350, startAngle = 180, spokeWidth = 4, spokeColor = '#00ffff')
{
  cv.beginPath();
  cv.strokeStyle = spokeColor;
  cv.lineWidth = spokeWidth;
  cv.lineCap = 'butt';

  for(let currAngle = startAngle; currAngle < last; currAngle += stepAngle)
  {
    let p1 = polarToCartesian(radius, currAngle);
    cv.moveTo(centerX + p1.x, centerY + p1.y);
    let p2 = polarToCartesian(radius + spokeLength, currAngle);
    cv.lineTo(centerX + p2.x, centerY + p2.y);
  }
  cv.stroke();
}

function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
       temp = myObj.temperature;
       hum = myObj.humidity;
       speed = myObj.winSpeed;
       direction = myObj.winDirection;      
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
       temp = myObj.temperature;
       hum = myObj.humidity;
       speed = myObj.winSpeed;
       direction = myObj.winDirection;
  }, false);} 
              
    </script>
       
  </body>
</html>
)rawliteral";


String getSensorReadings(){
  readings["winSpeed"] = String(9);  // substituindo data[0] por número para teste
  readings["winDirection"] = String(45); // substituindo data2[1] por número para teste
  String jsonString = JSON.stringify(readings);
  return jsonString;
  Serial.println(jsonString);
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
                request->send(200, "text/html", index_html); //get readings
                  });

        server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
            String json = getSensorReadings();
            request->send(200, "application/json", json); //get readings
            json = String();
          });

          events.onConnect([](AsyncEventSourceClient *client){
            if(client->lastId()){
              Serial.printf("Client reconnected! Last message ID is: %u\n", client->lastId());
            }
            client->send("hello!", NULL, millis(), 1000);
          });

          server.addHandler(&events);        

          // Start server
          server.begin();

        Serial.println("end of setup");
        
        } 


void loop()
      {             
        result = node1.readHoldingRegisters(0x0000, 2);
        if (result==node1.ku8MBSuccess){
          Serial.print("Wind Speed=  ");
          for (int i = 0; i < 2; i ++){
            data[i] = node1.getResponseBuffer(i);
            Serial.print(data[i]);
          }
        }
        

        result = node2.readHoldingRegisters(0x0000, 2);
        if (result==node2.ku8MBSuccess){
          Serial.print("Wind Direction=  ");
          for (int i = 0; i < 2; i ++){
            data2[i] = node2.getResponseBuffer(i);
            Serial.println(data2[i]);
          }
        }

        if ((millis() - lastTime) > intervalReading){
          events.send("ping", NULL, millis());
          events.send(getSensorReadings().c_str(), "new_readings", millis());
          lastTime = millis();
        }

      }