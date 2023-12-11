
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
// Set the LCD address to 0x27 for a 16 chars and 2 line display
//LiquidCrystal_I2C lcd(0x27, 20, 4);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Replace with your network credentials
const char* ssid = "CSIS.MH";
const char* password = "Omonjade2002";
String serverName = "http://192.168.8.100:5000/";  //"http://www.gpproject.live/";  //http://127.0.0.1:5000 for local dev or set 0.0.0.0 in flask env
const char* api_key = "xdol";
int cloud = 1;
int flag = 1;

int relay_pin[4] = { 32, 23, 33, 17 };
int current_pin[4] = { 35, 39, 34, 36 };
int green = 18;
int red = 16;
int lcdstate = 0;

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

int voltage;

//Voltage Libraries
#include "EmonLib.h"
int voltage_pin = 19;
#define VOLT_CAL 592
EnergyMonitor v;

//Send to Cloud details
unsigned long previous_time = 0;
const unsigned long interval_to_send = 5000;
String response;
String f_response;
String state;
bool fault = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int j = 0; j < 4; j++) {
    pinMode(current_pin[j], INPUT);
    pinMode(relay_pin[j], OUTPUT);
  }
  pinMode(voltage_pin, INPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);

  //SYSTEM INIT
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Jaidson Smart");
  lcd.setCursor(0, 1);
  lcd.print("Distrib... Board");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing");
  lcd.setCursor(0, 1);
  lcd.print("............");
  delay(2000);

  //WIFI INIT
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //SYSTEM INIT
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initialization");
  lcd.setCursor(0, 1);
  lcd.print("Successful");
  delay(3000);
  lcd.clear();
}

void loop() {

  //If no fault, turn on green light
  String f = send_to_cloud("get_fault"); //fault is a 0 by default
  fault = char(f[0]) - '0';
  if (fault) {
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
  } else {
    digitalWrite(red, LOW);
    digitalWrite(green, HIGH);
  }

  // put your main code here, to run repeatedly:
  state = send_to_cloud("get_state");
  state.replace(" ", "");
  state.replace(",", "");
  //byte s = (byte) strtol(state.c_str(), NULL, 2);
  changeState(state);


  voltage = constrain(voltage, 227, 230);
  
  current_1 = read_current(ACS1);
  power_1 = current_1 * voltage;
  current_2 = read_current(ACS2);
  power_2 = current_2 * voltage;
  current_3 = read_current(ACS3);
  power_3 = current_3 * voltage;
  current_4 = read_current(ACS4);
  power_4 = current_4 * voltage;

  Serial.println(voltage);
  //Print on lcd

if (lcdstate == 0){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C1: ");
  lcd.print(current_1);
  lcd.print(" P1: ");
  lcd.print(power_1);

  lcd.setCursor(0, 1);
  lcd.print("C2: ");
  lcd.print(current_2);
  lcd.print(" P2: ");
  lcd.print(power_2);
 lcdstate = 1;
} 
else {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C3: ");
  lcd.print(current_3);
  lcd.print(" P3: ");
  lcd.print(power_3);

  lcd.setCursor(0, 1);
  lcd.print("C4: ");
  lcd.print(current_4);
  lcd.print(" P4: ");
  lcd.print(power_4);
 lcdstate = 0;
}

  //Send Data to cloud every interval_to_send secs.
  if (cloud) {
    if (millis() - previous_time >= interval_to_send) {
      String detail = "update/key=" + String(api_key) 
                      + "/c1=" + String(current_1) + "/c2=" + String(current_2)
                      + "/c3=" + String(current_3) + "/c4=" + String(current_4) + "/v=" + String(voltage);
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

void changeState(String s) {
  digitalWrite(relay_pin[0], char(s[0]) - '0');
  digitalWrite(relay_pin[1], char(s[1]) - '0');
  digitalWrite(relay_pin[2], char(s[2]) - '0');
  digitalWrite(relay_pin[3], char(s[3]) - '0');
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

    // Send HTTP POST request
    int httpResponseCode = http.GET();
    delay(1100);  //This should give time for the server to process the request.

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code:");
      if (data == "get_state") {
        response = http.getString();
        return response;
      }
      if (data == "get_fault") {
        f_response = http.getString();
        return f_response;
      }
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      return String(int(0));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
