
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>


const int PIN_CURRENT = A0;
const int PIN_VOLTAGE = A1;
const int PIN_TEMP    = A2;
const int PIN_RELAY   = 2;
const int PIN_SD_CS   = 10;


const float ADC_RES = 1023.0;
const float VREF    = 5.0;


const float R1 = 30000.0;
const float R2 = 10000.0;


const float ACS712_OFFSET = 2.5;
const float ACS712_SENS   = 0.185;


const float V_MAX       = 8.4;
const float V_MIN       = 4.0;
const float TEMP_MAX    = 45.0;
const float CURRENT_MAX = 4.5;


const int RELAY_ON  = HIGH;
const int RELAY_OFF = LOW;


const unsigned long LOG_INTERVAL_MS = 5000;


RTC_DS3231 rtc;
File logFile;


bool relayState = true;
bool faultActive = false;
String faultReason = "";
unsigned long lastLog = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("BMS Starting"));

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, RELAY_ON);


   
   
  // RTC Initialization
  if (!rtc.begin()) {
    Serial.println(F("RTC not found!"));
    while (1);
  }
  rtc.adjust(DateTime(2026, 6, 3, 10, 06, 0));

  
  Serial.println(F("RTC initialized successfully"));

  if (!SD.begin(PIN_SD_CS)) {
    Serial.println(F("ERROR: SD card initialization failed!"));
  } else {
    Serial.println(F("SD Card OK"));

    if (!SD.exists("bms_log.csv")) {
      logFile = SD.open("bms_log.csv", FILE_WRITE);

      if (logFile) {
        logFile.println(F("Date,Time,Voltage(V),Temperature(C),Current(A),Status,Fault"));
        logFile.close();
        Serial.println(F("CSV file created."));
      }
    }
  }

  Serial.println(F("BMS Ready. Monitoring started."));
  Serial.println(F("--------------------------------------------"));
  Serial.println(F("Date       Time      V(V)  T(C)  I(A)  Status"));
  Serial.println(F("--------------------------------------------"));
}

float readVoltage() {
  int raw = analogRead(PIN_VOLTAGE);

  float v_adc = (raw / ADC_RES) * VREF;

  float v_bat = v_adc * ((R1 + R2) / R2);

  return v_bat;
}

float readTemperature() {
  int raw = analogRead(PIN_TEMP);

  float v_adc = (raw / ADC_RES) * VREF;

  float tempC = v_adc / 0.01;

  return tempC;
}

float readCurrent() {
  long sum = 0;

  for (int i = 0; i < 50; i++) {
    sum += analogRead(PIN_CURRENT);
    delay(2);
  }

  float raw = sum / 50.0;

  float v_adc = (raw / ADC_RES) * VREF;

  float current = (v_adc - ACS712_OFFSET) / ACS712_SENS;

  return abs(current);
}

void checkFaults(float voltage, float tempC, float current) {

  faultActive = false;
  faultReason = "NORMAL";

  if (voltage > V_MAX) {
    faultActive = true;
    faultReason = "OVERVOLTAGE";
  }
  else if (voltage < V_MIN) {
    faultActive = true;
    faultReason = "UNDERVOLTAGE";
  }
  else if (tempC > TEMP_MAX) {
    faultActive = true;
    faultReason = "OVERTEMP";
  }
  else if (current > CURRENT_MAX) {
    faultActive = true;
    faultReason = "OVERCURRENT";
  }

  if (faultActive) {
    digitalWrite(PIN_RELAY, RELAY_OFF);
    relayState = false;
  }
  else {
     digitalWrite(PIN_RELAY, RELAY_ON);
    relayState = true;
  }
}

void logToSD(DateTime now, float voltage, float tempC,
             float current, String status) {

  logFile = SD.open("bms_log.csv", FILE_WRITE);

  if (logFile) {

    logFile.print(now.year());
    logFile.print("-");

    if (now.month() < 10) logFile.print("0");
    logFile.print(now.month());
    logFile.print("-");

    if (now.day() < 10) logFile.print("0");
    logFile.print(now.day());
    logFile.print(",");

    if (now.hour() < 10) logFile.print("0");
    logFile.print(now.hour());
    logFile.print(":");

    if (now.minute() < 10) logFile.print("0");
    logFile.print(now.minute());
    logFile.print(":");

    if (now.second() < 10) logFile.print("0");
    logFile.print(now.second());
    logFile.print(",");

    logFile.print(voltage, 2);
    logFile.print(",");

    logFile.print(tempC, 1);
    logFile.print(",");

    logFile.print(current, 2);
    logFile.print(",");

    logFile.print(relayState ? "RELAY_ON" : "RELAY_OFF");
    logFile.print(",");

    logFile.println(status);

    logFile.close();
  }
}

void printSerial(DateTime now, float voltage,
                 float tempC, float current) {

  if (now.day() < 10) Serial.print("0");
  Serial.print(now.day());
  Serial.print("/");

  if (now.month() < 10) Serial.print("0");
  Serial.print(now.month());
  Serial.print("/");

  Serial.print(now.year());
  Serial.print("  ");

  if (now.hour() < 10) Serial.print("0");
  Serial.print(now.hour());
  Serial.print(":");

  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");

  if (now.second() < 10) Serial.print("0");
  Serial.print(now.second());
  Serial.print("  ");

  Serial.print(voltage, 2);
  Serial.print("V  ");

  Serial.print(tempC, 1);
  Serial.print("C  ");

  Serial.print(current, 2);
  Serial.print("A  ");

  if (faultActive) {
    Serial.print("*** FAULT: ");
    Serial.print(faultReason);
    Serial.println(" - RELAY OFF ***");
  } else {
    Serial.println("OK - RELAY ON");
  }
}

void loop() {

  unsigned long now_ms = millis();

  float voltage = readVoltage();
  float tempC   = readTemperature();
  float current = readCurrent();

  checkFaults(voltage, tempC, current);

  if (now_ms - lastLog >= LOG_INTERVAL_MS) {

    lastLog = now_ms;

    DateTime rtcNow = rtc.now();

    printSerial(rtcNow, voltage, tempC, current);

    logToSD(rtcNow, voltage, tempC, current, faultReason);
  }

  delay(100);
}

