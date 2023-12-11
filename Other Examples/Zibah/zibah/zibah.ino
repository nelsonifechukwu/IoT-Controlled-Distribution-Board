
//Libraries for ESP32
#include <Wire.h>
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

//Used for LCD Screen
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Replace with your network credentials
const char* ssid = "PIEEIPPIE";
const char* password = "Xjok23dl";
String serverName = "https://zibahapp.onrender.com/";  //"http://www.gpproject.live/";  //http://127.0.0.1:5000 for local dev or set 0.0.0.0 in flask env
const char* api_key = "xdol";
int cloud = 0;
int flag = 1;

int bulb_relay = 17;
int relay_pin[4] = { 23, 33, 32, 18 };
int current_pin[4] = { 34, 35, 36, 39};

#include "ACS712.h"
ACS712 ACS1(current_pin[0], 3.3, 4095, 66);
ACS712 ACS2(current_pin[1], 3.3, 4095, 66);
ACS712 ACS3(current_pin[2], 3.3, 4095, 66);
ACS712 ACS4(current_pin[3], 3.3, 4095, 66);

int current_1; 
int current_2;
int current_3;
int current_4;

int power_1;
int power_2;
int power_3;
int power_4;
int voltage = 0;

//Voltage Libraries
#include "EmonLib.h"
int voltage_pin = 12;
#define VOLT_CAL 592
EnergyMonitor v;

//Send to Cloud details
unsigned long previous_time = 0;
const unsigned long interval_to_send = 5000;
String response;
String state;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(voltage_pin, INPUT);
  pinMode(bulb_relay, OUTPUT);
  for (int j = 0; j < 4; j++) {
  pinMode(current_pin[j], INPUT);
  pinMode(relay_pin[j], OUTPUT);
  }

  //SYSTEM INIT
  lcd.begin();
  lcd.backlight();
  long t = millis();
  while (millis() - t > 5000) {
    lcd.setCursor(0, 0);
    lcd.print("Welcome");
    lcd.setCursor(0, 1);
    lcd.print("Zibah Distribution");
    lcd.setCursor(0, 2);
    lcd.print("Board System");
    lcd.setCursor(0, 3);
    lcd.print("Initializing...");
    delay(3000);
    lcd.clear();
  }

  //WIFI INIT
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting");
  int i = 1;
  while (WiFi.status() != WL_CONNECTED && i > 0) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(i - 1, 1);
    lcd.print(".");
    i++;
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

    //SYSTEM INIT
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Initialization");
  lcd.setCursor(0, 2);
  lcd.print("Successful");
  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  state = send_to_cloud("get_state");
  digitalWrite(relay_pin[0], char(state[0]) - '0');
  digitalWrite(relay_pin[1], char(state[3]) - '0');
  digitalWrite(relay_pin[2], char(state[6]) - '0');
  digitalWrite(relay_pin[3], char(state[9]) - '0');

  current_1 = read_current(ACS1);
  power_1 = current_1 * voltage;
  current_2 = read_current(ACS2);
  power_2 = current_2 * voltage;
  current_3 = read_current(ACS3);
  power_3 = current_3 * voltage;
  current_4 = read_current(ACS4);
  power_4 = current_4 * voltage;
  voltage = read_voltage();
  voltage = constrain(voltage, 227, 230);

  //Print on lcd
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("c1: ");
  lcd.print(current_1);
  lcd.print(" P1: ");
  lcd.print(power_1);
  lcd.print(" V: ");
  lcd.print(voltage);

  lcd.setCursor(0, 1);
  lcd.print("c2: ");
  lcd.print(current_2 );
  lcd.print(" P2: ");
  lcd.print(power_2);

  lcd.setCursor(0, 2);
  lcd.print("c3: ");
  lcd.print(current_3);
  lcd.print(" P3: ");
  lcd.print(power_3);

  lcd.setCursor(0, 3);
  lcd.print("c4: ");
  lcd.print(current_4);
  lcd.print(" P4: ");
  lcd.print(power_4);

  //Send Data to cloud every interval_to_send secs.
  if (cloud) {
    if (millis() - previous_time >= interval_to_send) {
      String detail = "update/key=" + String(api_key) + "/voltage=" + String(voltage)
                      + "/c1=" + String(current_1) + "/c2=" + String(current_2)
                      + "/c3=" + String(current_3) + "/c4=" + String(current_4);
      send_to_cloud(detail);
      previous_time = millis();
    }
  }
}

float read_current(ACS712 name) {
  float average = 0;
  for (int i = 0; i < 100; i++) {
    average += name.mA_AC();
  }
  float curr = (average / 100.0);
  curr = curr * 1e-3;  //convert from mA to A
  return curr;
}

float read_voltage() {
  v.calcVI(20, 2000);
  float volt = v.Vrms;
  delay(10);
  return volt;
}

String send_to_cloud(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    //http://127.0.0.1:5000/update/key=xdol/temp=12/vll=30/vln=3/vne=113/current=13
    // Prepare your HTTP POST request data
    String httpRequestData = data;
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    http.begin((serverName + httpRequestData).c_str());
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Send HTTP GET request
    int httpResponseCode = http.GET();
    delay(1100);  //This should give time for the server to process the request.

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code:");
      if (data == "get_state") {
        response = http.getString();
        return response;
      }
    } else { 
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      return String(0);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
