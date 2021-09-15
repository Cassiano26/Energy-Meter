#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <PZEM004Tv30.h>
#include "time.h"
#include <Preferences.h>

Preferences preferences;

char auth[] = "**********************";
char ssid[] = "*********";
char pass[] = "********";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -14400;  // GM offset in seconds. this value can be a positive or negative number, depending on your location.
const int   daylightOffset_sec = 3600;

// Global variables 

int currentHour;
int currentMinute;
int currentWeekDay;

bool wrongTime;

float tax1;
float tax2;
float tax3;
float noTax;
float pzem1LastEnergy;
float pzem2LastEnergy;
float pzem3LastEnergy;

float voltage_phase01;
float current_phase01;
float power_phase01;
float pf_phase01;
float frequency_phase01; 

float voltage_phase02;
float current_phase02;
float power_phase02;
float pf_phase02;
float frequency_phase02;

float voltage_phase03;
float current_phase03;
float power_phase03;
float pf_phase03;
float frequency_phase03;

float energy_phase01;
float energy_phase02;
float energy_phase03;

float totalEnergy;

bool errorPzem1;
bool errorPzem2;
bool errorPzem3;


WidgetLED ledPzem1(V19);
WidgetLED ledPzem2(V20);
WidgetLED ledPzem3(V21);

// Declaring PZEM - 004 v3 sensors. 

PZEM004Tv30 pzem1(&Serial2,16,17,0x01);
PZEM004Tv30 pzem2(&Serial2,16,17,0x02);
PZEM004Tv30 pzem3(&Serial2,16,17,0x03);

// Blynk app Timer.

BlynkTimer timer;

// Functions

void saveLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    wrongTime = true;
    return;
  }

  wrongTime = false;
   
  char hour[3];
  strftime(hour,3, "%H", &timeinfo);
  currentHour = String(hour).toInt(); 
  Serial.println(currentHour);
  
  char minutes[3]; 
  strftime(minutes,3, "%M", &timeinfo);
  currentMinute = String(minutes).toInt();
  Serial.println(currentMinute);

  char weekDay[2];
  strftime(weekDay, 2, "%u", &timeinfo);
  currentWeekDay = String(weekDay).toInt();
  Serial.println(currentWeekDay);
  
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to AP successfully!");
  
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.println("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.begin(ssid, pass);
  
}

void readPzems(){
  
  voltage_phase01 = pzem1.voltage();
  current_phase01 = pzem1.current();
  power_phase01 = pzem1.power(); 
  energy_phase01 = pzem1.energy();
  pf_phase01 = pzem1.pf();
  frequency_phase01 = pzem1.frequency();

  voltage_phase02 = pzem2.voltage();
  current_phase02 = pzem2.current();
  power_phase02 = pzem2.power(); 
  energy_phase02 = pzem2.energy();
  pf_phase02 = pzem2.pf();
  frequency_phase02 = pzem2.frequency();

  voltage_phase03 = pzem3.voltage();
  current_phase03 = pzem3.current();
  power_phase03 = pzem3.power(); 
  energy_phase03 = pzem3.energy();
  pf_phase03 = pzem3.pf();
  frequency_phase03 = pzem3.frequency();
  
  checkPzemError();
  
}

void getData(){
 
  tax1 = preferences.getFloat("tax1", 0);
  tax2 = preferences.getFloat("tax2", 0);
  tax3 = preferences.getFloat("tax3", 0);
  noTax = preferences.getFloat("noTax", 0);
  pzem1LastEnergy = preferences.getFloat("pzem1LastEnergy", 0);
  pzem2LastEnergy = preferences.getFloat("pzem2LastEnergy", 0);
  pzem3LastEnergy = preferences.getFloat("pzem3LastEnergy", 0);

  delay(500);
  
}

void checkPzemResetEnergy() {
  if (pzem1LastEnergy > energy_phase01) {
    pzem1LastEnergy = 0.0;
  } 
  
  if(pzem2LastEnergy > energy_phase02){
    pzem2LastEnergy = 0.0;
  }

  if(pzem3LastEnergy > energy_phase03) {
    pzem3LastEnergy = 0.0;
  }

}

void checkPzemError() {

  if(isnan(energy_phase01)) {
    errorPzem1 = true;
  } else {
    errorPzem1 = false;
  }

  if(isnan(energy_phase02)) {
    errorPzem2 = true;
  } else {
    errorPzem2 = false;
  }

  if(isnan(energy_phase03)) {
    errorPzem3 = true;
  } else {
    errorPzem3 = false;
  }

}

void taxDivider(){

  calculateAndStorege();

  // tax
  //     Saturday - 6. or Sunday - 7.
  //       tax1 06:00 to 21:30
  //       tax3 21:30 to 06:00
  //     Other days - 1,2,3,4,5.
  //       tax1 06:00 to 17:30 and 20:30 to 21:30
  //       tax2 17:30 to 20:30
  //       tax3 21:30 to 06:00  

  if (currentWeekDay == 6 || currentWeekDay == 7 ) {
    if(currentHour >= 6 && currentHour < 21) {
      tax1 = tax1 + totalEnergy;
      preferences.putFloat("tax1", tax1);
    } else if(currentHour == 21 && currentMinute < 30) {
      tax1 = tax1 + totalEnergy;
      preferences.putFloat("tax1", tax1);
    } else {
      tax3 = tax3 + totalEnergy;
      preferences.putFloat("tax3", tax3);  
      }
  } else {
    if (currentHour >= 6 && currentHour < 17 ){
      tax1 = tax1 + totalEnergy;
      preferences.putFloat("tax1", tax1);
    } else if(currentHour == 17 && currentMinute < 30) {
      tax1 = tax1 + totalEnergy;
      preferences.putFloat("tax1", tax1);
    } else if(currentHour == 17 && currentMinute >= 30) {
      tax2 = tax2 + totalEnergy;
      preferences.putFloat("tax2", tax2);
    } else if(currentHour == 18 || currentHour == 19) {
      tax2 = tax2 + totalEnergy;
      preferences.putFloat("tax2", tax2);
    } else if(currentHour == 20 && currentMinute < 30) {
      tax2 = tax2 + totalEnergy;
      preferences.putFloat("tax2", tax2);
    } else if(currentHour == 20 && currentMinute >= 30) {
      tax1 = tax1 + totalEnergy;
      preferences.putFloat("tax1", tax1);
    } else if(currentHour == 21 && currentMinute < 30) {
      tax1 = tax1 + totalEnergy;
      preferences.putFloat("tax1", tax1);
    } else {
      tax3 = tax3 + totalEnergy;
      preferences.putFloat("tax3", tax3);
    }  
    
  }

}

void calculateAndStorege() {

  checkPzemResetEnergy();

  float totalEnergyPzem1 = energy_phase01 - pzem1LastEnergy;
  float totalEnergyPzem2 = energy_phase02 - pzem2LastEnergy;
  float totalEnergyPzem3 = energy_phase03 - pzem3LastEnergy;

  totalEnergy = totalEnergyPzem1 + totalEnergyPzem2 + totalEnergyPzem3;

  preferences.putFloat("pzem1LastEnergy", energy_phase01);
  preferences.putFloat("pzem2LastEnergy", energy_phase02);
  preferences.putFloat("pzem3LastEnergy", energy_phase03);

  delay(1000);

}

void myTimerEvent(){

  getData();
  readPzems();
  saveLocalTime();

  if(errorPzem1 || errorPzem2 || errorPzem3) {
   
    if(errorPzem1){
      Blynk.setProperty(V19, "color", "#D3435C");
    } else {
      Blynk.setProperty(V19, "color", "#5CD343");
    }

    if(errorPzem2) {
      Blynk.setProperty(V20, "color", "#D3435C");
    } else {
      Blynk.setProperty(V20, "color", "#5CD343");
    }

    if(errorPzem3) {
      Blynk.setProperty(V21, "color", "#D3435C");
    } else {
      Blynk.setProperty(V21, "color", "#5CD343");
    }

  } else if(wrongTime) {

    calculateAndStorege();
    noTax = noTax + totalEnergy;
    preferences.putFloat("noTax", noTax);

    delay(1000);

  } else {

    taxDivider();
  
    // Sending data to blynk app.
  
    Blynk.virtualWrite(V0, voltage_phase01);
    Blynk.virtualWrite(V1, current_phase01);            
    Blynk.virtualWrite(V2, power_phase01);
    Blynk.virtualWrite(V3, pf_phase01);
    Blynk.virtualWrite(V4, frequency_phase01);

    Blynk.virtualWrite(V5, voltage_phase02);
    Blynk.virtualWrite(V6, current_phase02);            
    Blynk.virtualWrite(V7, power_phase02);
    Blynk.virtualWrite(V8, pf_phase02);
    Blynk.virtualWrite(V9, frequency_phase02);
  
    Blynk.virtualWrite(V10, voltage_phase03);
    Blynk.virtualWrite(V11, current_phase03);            
    Blynk.virtualWrite(V12, power_phase03);
    Blynk.virtualWrite(V13, pf_phase03);
    Blynk.virtualWrite(V14, frequency_phase03);
  
    Blynk.virtualWrite(V15, tax1);
    Blynk.virtualWrite(V16, tax2);
    Blynk.virtualWrite(V17, tax3);
    Blynk.virtualWrite(V18, noTax);

    // red: #D3435C
    // green: #5CD343

    Blynk.setProperty(V19, "color", "#5CD343");
    Blynk.setProperty(V20, "color", "#5CD343");
    Blynk.setProperty(V21, "color", "#5CD343");
  
  }
   
}

// Setup

void setup() {
  
  Serial.begin(115200);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  preferences.begin("accumulated", false);

  WiFi.disconnect(true);

  delay(1000);

  WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(10000L, myTimerEvent);

  ledPzem1.on();
  ledPzem2.on();
  ledPzem3.on();
  
}

// Loop. This area has always must be clean.

void loop() {

  Blynk.run();
  timer.run();
  
}
