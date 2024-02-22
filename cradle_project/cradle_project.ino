#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <UrlEncode.h>

#define DHTPIN 22     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

// Replace with your network credentials
const char* ssid = "hcn sar 03212323255";
const char* password = "humayunsj789";
String phoneNumber = "+923162725716";
String apiKey = "7320191";
// Set web server port number to 80
AsyncWebServer server(80);

int water_sensor = 27;
bool isWet = false;
bool isWet_prev = false;

int pir_sensor = 26;
bool isAwake = false;
bool buzzerOn = false;
bool isCry =false;
bool flag = false;
int buzzer = 32;
int sound = 18;
int cry_alert;
void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

int u_alert;

void setup() {
  Serial.begin(115200);

  pinMode(water_sensor, INPUT);
  pinMode(pir_sensor, INPUT);
  pinMode(buzzer, OUTPUT);
pinMode(sound,INPUT);
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

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String page = "<!DOCTYPE html><html>";
    page += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    page += "<link rel=\"icon\" href=\"data:,\">";
    page += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
    page += ".block { width: 300px; height: 100px; margin: 20px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 100px; }";
    page += ".block.green { background-color: #4CAF50; color: white; }";
    page += ".block.red { background-color: #FF0000; color: white; }";
    page += ".block.blue {background-color: #0000FF;color: white;}";
    page += ".block.white {background-color: #FFFFFF; color: black;}";
    page += ".small-block { width: 300px; height: 50px; margin: 10px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 50px; }";
    page += ".small-block.green { background-color: #4CAF50; color: white; }";
    page += ".small-block.red { background-color: #FF0000; color: white; }";
    page += ".small-block.blue {background-color: #0000FF;color: white;}";
    page += ".small-block.white {background-color: #FFFFFF; color: black;}";
    page += "</style></head>";
    page += "<body>";
    page += "<h1>IOT BASED SMART CRADLE</h1>";
    page += "<div class=\"block ";
    page += (isWet ? "blue\">URINE DETECTED" : "white\">NO URINE DETECTED");
    page += "</div>";
    page += "<div class=\"block ";
    page += (isAwake ? "red\">BABY AWAKE" : "green\">BABY SLEEPING");
    page += "</div>";
    page += "<div class=\"block ";
    page += (isCry ? "red\">BABY CRYING" : "green\">NO CRY");
    page += "</div>";  
    page += "<div class=\"small-block ";
    page += ("green\">TEMP: " + String(random(1,100)) + " C");
    page += "</div>";
    page += "<div class=\"small-block ";
    page += ("blue\">HUMIDITY: " + String(random(1,100)) + " %");
    page += "</div>";
    //page += "<p>BUZZER ALERT ";
   // page += ((buzzerOn) ? "on" : "off");
   // page += "</p>";
   // page += "<p><a href=\"/toggle\"><button class=\"button\">";
   // page += ((buzzerOn)? "TURN OFF BUZZER" : "TURN ON BUZZER");
   // page += "</button></a></p>";
    page += "<script>setInterval(function() { location.reload(); }, 1000);</script>"; // Refresh page every 2 seconds
    page += "</body></html>";
    request->send(200, "text/html", page);
    digitalWrite(buzzer,buzzerOn);
      
  });

  // Route to handle the buzzer toggle
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
    buzzerOn = !buzzerOn;
    //flag=!flag;
    //flag=0;
    Serial.print("flag");
    Serial.println(flag);
    
    //flag =!flag;
     //flag=(!isAwake)?0:flag; 
    //digitalWrite(2, flag);
    request->send(200, "text/plain", buzzerOn ? "Buzzer ON" : "Buzzer OFF");
  });

  // Start the server
  server.begin();
  dht.begin();
  String ip ="http://"+String(WiFi.localIP());
  sendMessage(ip);
}

void loop() {
  // Update sensor states
  isWet = !digitalRead(water_sensor);
  isAwake = digitalRead(pir_sensor);
  isCry = digitalRead(sound);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  if(isAwake && !flag)
  {buzzerOn =1;
  flag=1;}
  // digitalWrite(2,buzzerOn);
   // }
  else if(!isAwake && flag)
  {
  flag=0;
  // digitalWrite(2,buzzerOn);
   // }
  }

  if(isWet!=isWet_prev & u_alert==0){
    u_alert=1;
    isWet_prev=isWet;
    sendMessage("WETNESS ON CRADLE");
    }

if(isCry & cry_alert==0){
    u_alert=1;
    isWet_prev=isWet;
    sendMessage("WETNESS ON CRADLE");
    }
    
  delay(1000);

  // Handle incoming client requests
  // Note: The server will handle incoming requests asynchronously
//  server.handleClient();
}
