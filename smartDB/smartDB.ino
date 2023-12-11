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

#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Replace with your network credentials
const char* ssid = "PIEEIPPIE";
const char* password = "Xjok23dl";
String serverName = "https://dipoapp.onrender.com/";
const char* api_key = "xdol";

//SHIFT REGISTER
int dataPin = 5;
int latchPin = 18;
int clockPin = 19;

//Sensor Variables
float current_1;
float current_2;
float current_3;
float current_4;
float current_5;
float current_6;
float current_7;
float current_8;
float voltage;

//PIN DEFINITIONS
int current_pin[7] = { 35, 36, 39, 34, 32, 33, 17 };

#include "ACS712.h"
ACS712 ACS1(current_pin[0], 3.3, 4095, 66);
ACS712 ACS2(current_pin[1], 3.3, 4095, 66);
ACS712 ACS3(current_pin[2], 3.3, 4095, 66);
ACS712 ACS4(current_pin[3], 3.3, 4095, 66);
ACS712 ACS5(current_pin[4], 3.3, 4095, 66);
ACS712 ACS6(current_pin[5], 3.3, 4095, 66);
ACS712 ACS7(current_pin[6], 3.3, 4095, 66);

//Voltage Libraries
#include "EmonLib.h"
int voltage_pin = 16;
#define VOLT_CAL 592
EnergyMonitor v;

//Send to Cloud details
unsigned long previous_time = 0;
const unsigned long interval_to_send = 5000;
String state;
String response;

void setup() {

  // PIN DEFINITIONS
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(voltage_pin, INPUT);

  for (int j = 0; j < 7; j++) {
    pinMode(current_pin[j], INPUT);
  }
  Serial.begin(115200);
  //v.voltage(voltage_pin, VOLT_CAL, 1.7);
  lcd.begin();
  lcd.backlight();


    lcd.setCursor(0, 0);
    lcd.print("Welcome");
    lcd.setCursor(0, 1);
    lcd.print("Smart Distribution");
    lcd.setCursor(0, 2);
    lcd.print("Board System");
    lcd.setCursor(0, 3);
    lcd.print("Initializing...");
    delay(3000);
    lcd.clear();

  //WIFI CONNECTION
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
  lcd.setCursor(0, 1);
  lcd.print("Initialization");
  lcd.setCursor(0, 2);
  lcd.print("Successful");
  delay(3000);
}

void loop() {

  // Read States
  state = send_to_cloud("get_state");
  state.replace(" ", "");
  state.replace(",", "");
  byte s = (byte) strtol(state.c_str(), NULL, 2);
  changeState(s);


  //Read Current & Voltage Values
  current_1 = read_current(ACS1);
  current_2 = read_current(ACS2);
  current_3 = read_current(ACS3);
  current_4 = read_current(ACS4);
  current_5 = read_current(ACS5);
  current_6 = read_current(ACS6);
  current_7 = read_current(ACS7);
  current_8 = 0;
  voltage = read_voltage();
  voltage = constrain(4, 227, 230);

  printdetails();

  //Menu: Do you want normal readings only or continuous reading with cloud integration?
  int cloud = 1;

  //Send Data to cloud every interval_to_send secs.
  if (cloud) {
    if (millis() - previous_time >= interval_to_send) {
      String request = "update/key=" + String(api_key) + "/voltage=" + String(int(voltage))
                      + "/c1=" + String(int(current_1)) + "/c2=" + String(int(current_2))
                      + "/c3=" + String(int(current_3)) + "/c4=" + String(int(current_4))
                      + "/c5=" + String(int(current_5)) + "/c6=" + String(int(current_6))
                      + "/c7=" + String(int(current_7)) + "/c8=" + String(int(current_8));
      send_to_cloud(request);
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

void changeState(byte states) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, states);
  delay(100);
  digitalWrite(latchPin, HIGH);
}

void printdetails(){

    // PRINT INFO ON LCD
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(voltage);
  lcd.print("V");

  lcd.setCursor(11, 0);
  lcd.print("C1:");
  lcd.print(current_1);
  lcd.print("A");

  lcd.setCursor(0, 1);
  lcd.print("C2:");
  lcd.print(current_2);
  lcd.print("A");

  lcd.setCursor(11, 1);
  lcd.print("C3:");
  lcd.print(current_3);
  lcd.print("A");

  lcd.setCursor(0, 2);
  lcd.print("C4:");
  lcd.print(current_4);
  lcd.print("A");

  lcd.setCursor(11, 2);
  lcd.print("C5:");
  lcd.print(current_5);
  lcd.print("A");

  lcd.setCursor(0, 3);
  lcd.print("C6:");
  lcd.print(current_6);
  lcd.print("A");

  lcd.setCursor(11, 3);
  lcd.print("C7:");
  lcd.print(current_7);
  lcd.print("A");
}

String send_to_cloud(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

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
      Serial.println(httpResponseCode);
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