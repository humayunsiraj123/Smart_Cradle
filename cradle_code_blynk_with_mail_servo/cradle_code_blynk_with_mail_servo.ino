#define BLYNK_AUTH_TOKEN "o0flWq_beEOPgN0fzh1suYRHu-aRU5cr"
#define BLYNK_TEMPLATE_ID "TMPL2Nu_hdp90"
#define BLYNK_TEMPLATE_NAME "Iot smart cradle"
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <AsyncTCP.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include "ESP32_MailClient.h"
#include <ESP32Servo.h>




#define fan 18
#define sound 13
#define DHTTYPE DHT11  // DHT 11
#define DHTPIN 4       // Digital pin connected to the DHT sensor
#define SERVO_PIN 19   // ESP32 pin GPIO26 connected to servo motor
int ENA = 14;          //speed
int IN1 = 26;          //motor pin 1
int IN2 = 27;
const int frequency = 500;
const int pwm_channel = 0;
const int resolution = 8;

DHT dht(DHTPIN, DHTTYPE);

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "hcn sar 03212323255";
char pass[] = "humayunsj789";
Servo servo;
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "smartcradle5@gmail.com"
#define AUTHOR_PASSWORD "qsgrmmijxjcujobk"
/* Recipient's email*/
#define RECIPIENT_EMAIL "humayunsiraj0@gmail.com"    //enter email to send alert
#define emailSenderAccount "smartcradle5@gmail.com"  // Sender email address
#define emailSenderPassword "qsgrmmijxjcujobk"       // Sender email password
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "CRADLE ALERT"  // Email subject

String inputMessage = "humayunsiraj0@gmail.com";  // Recipient email alert
String enableEmailChecked = "checked";
String inputMessage2 = "true";
// Flag variable to keep track if email notification was sent or not
bool emailSent = false;
/* Declare the global used SMTPSession object for SMTP transport */

//SMTPData smtpData;

BlynkTimer timer;

bool fan_blynk;
bool fan_state;
bool active;
int speed;


unsigned long lastEvent = 0;
bool isCry = false;
bool flag = false;
bool cry = false;

int cry_alert;
int s_cnt;  //swing counting
unsigned long cry_ctime;
unsigned long cry_ptime;
unsigned long samples;
int statostart;
int relay1_state = 0;
unsigned long s1_cnt;
int samp_no;

SMTPData smtpData;
#define timeSeconds 10

// Set GPIOs for LED and PIR Motion Sensor
const int led = 2;
//const int motionSensor = 16;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long pt_speed;
// Checks if motion was detected, sets LED HIGH and starts a timer


void DHT11sensor();
//bool sendEmailNotification(String emailMessage);
//void sendCallback(SendStatus msg);
//fan control
BLYNK_WRITE(V2) {
  fan_blynk = param.asInt();
  Serial.print("fan BLYNK:");
  Serial.println(fan_blynk);
}

BLYNK_WRITE(V1) {
  speed = param.asInt();
  Serial.print("speed:");
  Serial.println(speed);
}



void setup() {

  Serial.begin(115200);

  pinMode(sound, INPUT);
  pinMode(fan, OUTPUT);

  digitalWrite(fan, 1);
  servo.attach(SERVO_PIN);  // attaches the servo on ESP32 pin
  servo.write(0);
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(100L, DHT11sensor);
  timer.setInterval(100L, fan_cntrl);
}


void loop()

{
  Blynk.run();
  timer.run();
  now = millis();

  cry = digitalRead(sound);
  cry_ctime = millis();
  if (millis() - pt_speed > 500) {
    pt_speed = millis();
    if (speed == 100) {
      if (s1_cnt % 2) {
        for (int i = 0; i < 180; i += 30) {
          servo.write(i);
          delay(15);
        }
        s1_cnt++;
      } else if (!(s1_cnt % 2)) {
        for (int i = 180; i > 0; i -= 30) {
          servo.write(i);
          delay(15);
        }
        s1_cnt++;
      }
    }

    else if (speed == 35) {
      if (s1_cnt % 2) {
        for (int i = 0; i < 180; i += 10) {
          servo.write(i);
          delay(30);
        }
        s1_cnt++;
      } else if (s1_cnt % 2 == 0) {
        for (int i = 180; i > 0; i -= 10) {
          servo.write(i);
          delay(30);
        }
        s1_cnt++;
      }
    }
    // digitalWrite(IN1, 1);
    // digitalWrite(IN2, 0);
    // ledcWrite(pwm_channel, 200);

    else if (speed == 70) {
      if (s1_cnt % 2) {
        for (int i = 0; i < 180; i += 20) {
          servo.write(i);
          delay(15);
        }
        s1_cnt++;
      } else if (s1_cnt % 2 == 0) {
        for (int i = 180; i > 0; i -= 20) {
          servo.write(i);
          delay(15);
        }
        s1_cnt++;
      }
    } else {
      servo.write(0);
    }
  }



  if (cry_ctime - cry_ptime > 10000) {
    cry_ptime = cry_ctime;
    Serial.print("NO OF SAMPLES ");
    Serial.println(samples);
    // if(sample>1000)
    // {}
    // else if (sample<)
    if (samples > 10000 & cry_alert == 0) {
      if (sendEmailNotification("BABY IS CRYING")) {
        Serial.println("BABY is CRYING");
        emailSent = true;
      } else {
        Serial.println("Email failed to send");
      }
      cry_alert++;
      isCry = 1;
    }
    if (isCry == 1 & s_cnt < 5) {
      ++s_cnt;
      Serial.println(s_cnt);
    } else if (samples < 1000 | s_cnt == 5) {
      s_cnt = 0;
      cry_alert = 0;
      isCry = 0;
    }
    samples = 0;
  }

  if (cry == 1) {
    samples++;
  }
}


float h;
float t;
void DHT11sensor() {
  /// h =h>100?0:++h; //dht.readHumidity();
  // t =t>100?0:++t;; //dht.readTemperature();
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V0, t);  // Send temperature to V2 on the Blynk app
}
void fan_cntrl() {
  digitalWrite(fan, !fan_blynk);
}

// void servo_cntrl() {
//   if (digitalRead(forward_btn) == LOW) {
//     delay(200);
//     if (toy_active) {
//       servo.write(90);
//       Serial.println("SERVO WRITE +++");
//     }
//     //Serial.print("forward = ");
//     //Serial.println();
//     //delay(50);
//   }

//   if (digitalRead(rev_btn) == LOW) {
//     delay(200);
//     if (toy_active) {
//       Serial.println("SERVO WRITE ---");
//       servo.write(-90);
//     }
//     //Serial.print("rev = ");
//     //Serial.println();
//     //delay(50);
//   }
// }

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

  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }

  smtpData.empty();
  return true;
}

void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}