#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <Servo.h>
#include "ESP32_MailClient.h"

#define SERVO_PIN 26 // ESP32 pin GPIO26 connected to servo motor
#define DHTPIN 32     // Digital pin connected to the DHT sensor
#define water_sensor  19
#define sound  21
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
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "smartcradle5@gmail.com"
#define AUTHOR_PASSWORD "qsgrmmijxjcujobk"
/* Recipient's email*/
#define RECIPIENT_EMAIL "humayunsiraj0@gmail.com"
#define emailSenderAccount    "smartcradle5@gmail.com"    // Sender email address
#define emailSenderPassword  "qsgrmmijxjcujobk"       // Sender email password
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "CRADLE ALERT"   // Email subject
String inputMessage ="humayunsiraj0@gmail.com";   // Recipient email alert
String enableEmailChecked = "checked";
String inputMessage2 = "true";
// Flag variable to keep track if email notification was sent or not
bool emailSent = false;
/* Declare the global used SMTPSession object for SMTP transport */
SMTPData smtpData;

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
  } else {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

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

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String page = "<!DOCTYPE html><html>";
    page += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    page += "<link rel=\"icon\" href=\"data:,\">";
    page += "<style>html { font-family: Arial, sans-serif; text-align: center; background-color: #FFCCCC;}";
    page += "h1 { color: white; }";
    page += ".status-block { width: 300px; height: 100px; margin: 20px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 100px; }";
    page += ".status-block.green { background-color: #4CAF50; color: white; }";
    page += ".status-block.red { background-color: #FF0000; color: white; }";
    page += ".small-block { width: 300px; height: 50px; margin: 10px auto; border-radius: 8px; font-size: 24px; font-weight: bold; line-height: 50px; }";
    page += ".small-block.green { background-color: #4CAF50; color: white; }";
    page += ".small-block.red { background-color: #FF0000; color: white; }";
    page += ".toggle-button { display: inline-block; background-color: #008CBA; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 8px; }";
    page += ".toggle-button:hover { background-color: #005f7f; }";
    page += "</style></head>";
    page += "<body>";
    page += "<h1>IOT BASED SMART CRADLE</h1>";
    page += "<div class=\"status-block <?php echo $isWet ? 'red' : 'green'; ?>\"><?php echo $isWet ? 'URINE DETECTED' : 'NO URINE DETECTED'; ?></div>";
    page += "<div class=\"status-block <?php echo $isCry ? 'red' : 'green'; ?>\"><?php echo $isCry ? 'BABY CRYING' : 'NO CRY'; ?></div>";
    page += "<div class=\"small-block green\">TEMP: <?php echo dht.readTemperature(); ?> C</div>";
    page += "<div class=\"small-block green\">HUMIDITY: <?php echo dht.readHumidity(); ?> %</div>";
    page += "<button class=\"toggle-button\" onclick=\"toggleSwing()\">Toggle Cradle Swing</button>";
    page += "<script>";
    page += "function toggleSwing() {";
    page += "var xhttp = new XMLHttpRequest();";
    page += "xhttp.open('GET', '/toggleSwing', true);";
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

  // Start the server
  server.begin();
  dht.begin();
  String ip = "the webserver IP is " + String(WiFi.localIP().toString());
  Serial.print(ip);
  sendMessage(ip);
 
  if(sendEmailNotification(ip)) {
    emailSent = true;
  }
  else {
    Serial.println("Email failed to send");
  }    
}

void loop() {
  // Update sensor states
  wet = digitalRead(water_sensor);
  cry = digitalRead(sound);
  wet_ctime = millis();
  cry_ctime = millis();

  if (wet_ctime - wet_ptime > 4000 & wet_alert < 2 & wet == 0) {
    wet_ptime = wet_ctime;
    if (sendEmailNotification("WETNESS ON CRADLE")) {
      Serial.println("WETNESS ON CRADLE");
      emailSent = true;
    }
    else {
      Serial.println("Email failed to send");
    }
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
    if (samples > 1000 & cry_alert == 0) {
      if (sendEmailNotification("BABY IS CRYING")) {
        Serial.println("BABY is CRYING");
        emailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }
      cry_alert++;
      isCry = 1;
    }
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
  server.handleClient();
}

bool sendEmailNotification(String emailMessage) {
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  smtpData.setSender("Smart_Cradle_Alert_Mail", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);
  smtpData.setSendCallback(sendCallback);

  if (!MailClient.sendMail(smtpData))
  {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }

  smtpData.empty();
  return true;
}

void sendCallback(SendStatus msg)
{
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success())
  {
    Serial.println("----------------");
  }
}
