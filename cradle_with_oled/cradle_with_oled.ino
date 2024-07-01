#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include "ESP32_MailClient.h"

#define SERVO_PIN 26 // ESP32 pin GPIO26 connected to servo motor
#define DHTPIN 32     // Digital pin connected to the DHT sensor
#define water_sensor  19
#define sound  21
#define DHTTYPE DHT11   // DHT 11  
#define fan 23

DHT dht(DHTPIN, DHTTYPE);
Servo servo;

// Replace with your network credentials
const char* ssid = "hcn sar 03212323255";
const char* password = "humayunsj789";

// Set web server port number to 80
AsyncWebServer server(80);



unsigned long lastEvent = 0;
bool isWet = false;
bool isWet_prev = false;
bool isCry = false;
bool flag = false;
bool cry = false;
bool wet = false;

int cnt1;
int cry_alert;
int wet_alert;
int s_cnt; // Swing counting
unsigned long wet_ctime;
unsigned long wet_ptime;
unsigned long cry_ctime;
unsigned long cry_ptime;
unsigned long swing_ctime;
unsigned long swing_ptime;
unsigned long samples;

int swingState = 0; // Variable to keep track of cradle swing state
int fanState = 0; // Variable to keep track of fan state

String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3]);
}

void cradle_swing(int dir) {
  if (dir == 0) {
    for (int pos = 0; pos <= 180; pos += 1) {
      // in steps of 1 degree
      servo.write(pos);
      delay(15); // waits 15ms to reach the position
    }
  } else if (dir == 1) {
    // rotates from 180 degrees to 0 degrees
    for (int pos = 180; pos >= 0; pos -= 1) {
      servo.write(pos);
      delay(15); // waits 15ms to reach the position
    }
  }
}

void setup() {
  Serial.begin(115200);
  servo.attach(SERVO_PIN);  // attaches the servo on ESP32 pin
  servo.write(0);
  pinMode(water_sensor, INPUT);
  pinMode(sound, INPUT);
  pinMode(fan, OUTPUT); // Set fan pin as output
  digitalWrite(fan, LOW); // Initially turn off the fan

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

//page += "<style>html { font-family: Arial, sans-serif; text-align: center; background-color: #E0F7FA;}";
    
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String page = "<!DOCTYPE html><html>";
    page += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    page += "<link rel=\"icon\" href=\"data:,\">";
//    //page += "<style>html { font-family: Arial, sans-serif; text-align: center; background: linear-gradient(to bottom, #87CEFA, #F2F2F2);}";
//    page += "<style>html, body { height: 100%; margin: 0; font-family: Arial, sans-serif; text-align: center; background: linear-gradient(to bottom, #87CEFA, #F2F2F2); }";
//    page += "h1 { color: white; }";
//    page += ".status-block { width: 300px; height: 100px; margin: 20px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 100px; }";
//    page += ".status-block.green { background-color: #4CAF50; color: white; }";
//    page += ".status-block.red { background-color: #FF0000; color: white; }";
//    page += ".small-block { width: 300px; height: 50px; margin: 10px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 50px; }";
//    page += ".small-block.yellow { background-color: #FFFF99; color: black; }";
//    page += ".small-block.green { background-color: #4CAF50; color: white; }"; // Changed to green
//    page += ".small-block.blue { background-color: #87CEFA; color: white; }";  // Changed to light blue
//    page += ".toggle-button { display: inline-block; background-color: #008CBA; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 8px; }";
//    page += ".toggle-button:hover { background-color: #005f7f; }";
//    page += "</style></head>";
page += "<style>";
page += "html, body { height: 100%; margin: 0; font-family: Arial, sans-serif; text-align: center; background: linear-gradient(to bottom, #87CEFA, #F2F2F2); }";
page += "h1 { color: white; }";
page += ".status-block { width: 300px; height: 100px; margin: 20px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 100px; }";
page += ".status-block.green { background-color: #4CAF50; color: white; }";
page += ".status-block.red { background-color: #FF0000; color: white; }";
page += ".small-block { width: 300px; height: 50px; margin: 10px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 50px; }";
page += ".small-block.yellow { background-color: #FFFF99; color: black; }";
page += ".small-block.green { background-color: #4CAF50; color: white; }";
page += ".small-block.blue { background-color: #87CEFA; color: white; }";
page += ".toggle-button { display: block; width: 200px; margin: 10px auto; background-color: #008CBA; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; font-size: 16px; cursor: pointer; border-radius: 8px; }";
page += ".toggle-button:hover { background-color: #005f7f; }";
page += "</style></head>";
    page += "<body>";
    page += "<h1>SMART BABY CRADLE</h1>";
    page += "<div class=\"status-block " + String(isWet ? "red" : "green") + "\">" + String(isWet ? "URINE DETECTED" : "NO URINE DETECTED") + "</div>";
    page += "<div class=\"status-block " + String(isCry ? "red" : "green") + "\">" + String(isCry ? "BABY CRYING" : "NO CRY") + "</div>";
    page += "<div class=\"small-block green\">TEMP: " + String(dht.readTemperature()) + " C</div>";
    page += "<div class=\"small-block blue\">HUMIDITY: " + String(dht.readHumidity()) + " %</div>";
    page += "<button class=\"toggle-button\" onclick=\"toggleSwing()\">Toggle Cradle Swing</button>";
    page += "<button class=\"toggle-button\" onclick=\"toggleFan()\">Toggle Fan</button>";
    page += "<script>";
    page += "function toggleSwing() {";
    page += "var xhttp = new XMLHttpRequest();";
    page += "xhttp.open('GET', '/toggleSwing', true);";
    page += "xhttp.send();";
    page += "}";
    page += "function toggleFan() {";
    page += "var xhttp = new XMLHttpRequest();";
    page += "xhttp.open('GET', '/toggleFan', true);";
    page += "xhttp.send();";
    page += "}";
    page += "setInterval(function() { location.reload(); }, 1000);";
    page += "</script>";
    page += "</body></html>";
    request->send(200, "text/html", page);
  });

  // Route to handle toggleSwing request
  server.on("/toggleSwing", HTTP_GET, [](AsyncWebServerRequest *request){
    // Toggle the cradle swing state
    swingState = !swingState;
    // Perform cradle swing based on the new state
    //cradle_swing(swingState);
    request->send(200, "text/plain", "Cradle swing toggled");
  });

  // Route to handle toggleFan request
  server.on("/toggleFan", HTTP_GET, [](AsyncWebServerRequest *request){
    // Toggle the fan state
    fanState = !fanState;
    // Turn fan on or off based on the new state
    digitalWrite(fan, fanState ? HIGH : LOW);
    request->send(200, "text/plain", "Fan toggled");
  });

  // Start the server
  server.begin();
  dht.begin();
  String ip = "the webserver IP is " + String(WiFi.localIP().toString());
  Serial.print(ip);
  
}

void loop() {
  // Update sensor states
  wet = digitalRead(water_sensor);
  cry = digitalRead(sound);
  wet_ctime = millis();
  cry_ctime = millis();

  if (wet_ctime - wet_ptime > 4000 & wet_alert < 2 & wet == 0) {
    wet_ptime = wet_ctime;
   
    wet_alert++;
    isWet = 1;
  }
  else if (wet == 1) {
    wet_ptime = wet_ctime;
    isWet = 0;
    wet_alert = 0;
  }

  if (cry_ctime - cry_ptime > 1000) {
    cry_ptime = cry_ctime;
    Serial.print("NO OF SAMPLES ");
    Serial.println(samples);
   
      cry_alert++;
      isCry = 1;
    
    if (isCry == 1 & s_cnt < 10) {
      ++s_cnt;
      Serial.println(s_cnt);
    }
    else if (samples < 1000 |s_cnt==10) {
      s_cnt=0;
      cry_alert = 0;
      isCry = 0;
    }
    samples = 0;
  }

  if (cry == 0) {
    samples++;
  }
  
  
  if(swingState ==1 && millis()-swing_ptime>1000){
    ++cnt1;
    cradle_swing(cnt1%2);
    Serial.println("swinging");
    swing_ptime = millis();
  }

  // Handle incoming client requests
 // server.handleClient();
}
