#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <Servo.h>

#define SERVO_PIN 26 // ESP32 pin GPIO26 connected to servo motor
#define DHTPIN 32     // Digital pin connected to the DHT sensor
#define water_sensor  19
#define sound  18
#define DHTTYPE DHT11   // DHT 11  

DHT dht(DHTPIN, DHTTYPE);
Servo servo;

// Replace with your network credentials
const char* ssid = "hcn sar 03212323255";
const char* password = "humayunsj789";
String phoneNumber = "+923162725716";
String apiKey = "7320191";
// Set web server port number to 80
AsyncWebServer server(80);


unsigned long lastEvent = 0;
bool isWet = false;
bool isWet_prev = false;
bool isCry = false;
bool flag = false;
bool cry = false;
bool wet = false;

int cry_alert;
int wet_alert;
int s_cnt;//swing counting
unsigned long wet_ctime;
unsigned long wet_ptime;
unsigned long cry_ctime;
unsigned long cry_ptime;
unsigned long samples;
void sendMessage(String message) {

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200) {
    Serial.print("Message sent successfully");
  }
  else {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

int u_alert;
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3])  ;
}
void cradle_swing(int dir) {
  if (dir == 0) {
    for (int pos = 0; pos <= 180; pos += 1) {
      // in steps of 1 degree
      servo.write(pos);
      //delay(5); // waits 15ms to reach the position
    }
  }
  else if (dir == 1) {
    // rotates from 180 degrees to 0 degrees
    for (int pos = 180; pos >= 0; pos -= 1) {
      servo.write(pos);
      // delay(5); // waits 15ms to reach the position
    }
  }
}

void setup() {
  Serial.begin(115200);
  servo.attach(SERVO_PIN);  // attaches the servo on ESP32 pin
  pinMode(water_sensor, INPUT);
  pinMode(sound, INPUT);
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
  //#80E27E for light green and #33DFFF for light blue.
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String page = "<!DOCTYPE html><html>";
    page += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    page += "<link rel=\"icon\" href=\"data:,\">";
    page += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; background-color: #FFCCCC;}";
    page += "h1 { color: white; }";
    page += ".block { width: 300px; height: 100px; margin: 20px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 100px; }";
    page += ".block.green { background-color: #4CAF50; color: white; }";
    page += ".block.lgreen { background-color: #80E27E; color: white; }";
    page += ".block.red { background-color: #FF0000; color: white; }";
    page += ".block.blue {background-color: #0000FF;color: white;}";
    page += ".block.lblue {background-color: #33DFFF;color: white;}";
    page += ".block.white {background-color: #FFFFFF; color: black;}";
    page += ".small-block { width: 300px; height: 50px; margin: 10px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 50px; }";
    page += ".small-block.green { background-color: #4CAF50; color: white; }";
    page += ".small-block.lgreen { background-color: #80E27E; color: white; }";
    page += ".small-block.red { background-color: #FF0000; color: white; }";
    page += ".small-block.blue {background-color: #0000FF;color: white;}";
    page += ".small-block.lblue {background-color: #33DFFF;color: white;}";
    page += ".small-block.white {background-color: #FFFFFF; color: black;}";
    page += "</style></head>";
    page += "<body>";
    page += "<h1>IOT BASED SMART CRADLE</h1>";
    page += "<div class=\"block ";
    page += (isWet ? "blue\">URINE DETECTED" : "white\">NO URINE DETECTED");
    page += "</div>";
    page += "<div class=\"block ";
    page += (isCry ? "red\">BABY CRYING" : "white\">NO CRY");
    page += "</div>";
    page += "<div class=\"small-block ";
    page += ("lgreen\">TEMP: " + String(dht.readTemperature()) + " C");
    page += "</div>";
    page += "<div class=\"small-block ";
    page += ("lblue\">HUMIDITY: " + String(dht.readHumidity()) + " %");
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


  });



  // Start the server
  server.begin();
  dht.begin();
  String ip = "the webserver IP is " + String(WiFi.localIP().toString());
  Serial.print(ip);
  //sendMessage(WiFi.localIP().toString());
  sendMessage(ip);
}

void loop() {
  // Update sensor states
  wet = digitalRead(water_sensor);
  cry = digitalRead(sound);
  wet_ctime = millis();
  cry_ctime = millis();

  if (wet_ctime - wet_ptime > 4000 & wet_alert < 2 & wet == 0) {
    wet_ptime = wet_ctime;
    sendMessage("WETNESS ON CRADLE");
    wet_alert++;
    isWet = 1;
  }
  else if (wet == 1) {
    wet_ptime = wet_ctime;
    isWet = 0;
    wet_alert=0;
  }

  if (cry_ctime - cry_ptime > 1000) {
    cry_ptime = cry_ctime;
    Serial.print("NO OF SAMPLES ");
    Serial.println(samples);
    if (samples > 1000 & cry_alert ==0) {
      sendMessage("BABY IS CRYING");
      cry_alert++;
      isCry = 1;
    }
    if(isCry==1 & s_cnt<10){
     cradle_swing(s_cnt%2);
     s_cnt = s_cnt>10?0:++s_cnt;
      cry_alert =s_cnt>10?0:cry_alert;
      }
    else if (samples < 100 && s_cnt==0) {

      cry_alert = 0;
      isCry = 0;
    }
    samples = 0;
  }

  if (cry == 0) {
    samples++;
  }



  //  delay(3000);

  // Handle incoming client requests
  // Note: The server will handle incoming requests asynchronously
  //  server.handleClient();
}
