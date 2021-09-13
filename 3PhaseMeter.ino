#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <PZEM004Tv30.h>
#include "time.h"
#include <Preferences.h>

Preferences preferences;

char auth[] = "********************************";
char ssid[] = "************";
char pass[] = "*******";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -14400;  // GM offset in seconds. this value can be a positive or negative number, depending on your location.
const int   daylightOffset_sec = 3600;

// Global variables 

int currentHour;
int currentMinute;
int currentWeekDay;
bool wrongTime = true;

float tax1;
float tax2;
float tax3;
float noTax;
float pzem1LastEnergyReading;
float pzem2LastEnergyReading;
float pzem3LastEnergyReading;
// float lastEnergyValue;

float voltage_phase01=0;
float current_phase01=0;
float power_phase01=0;
float pf_phase01=0;

float voltage_phase02=0;
float current_phase02=0;
float power_phase02=0;
float pf_phase02=0;

float voltage_phase03=0;
float current_phase03=0;
float power_phase03=0;
float pf_phase03=0;

float energy_phase01;
float energy_phase02;
float energy_phase03;

// Declaring PZEM - 004 v3 sensors. 

PZEM004Tv30 pzem0(&Serial2,16,17,0x01);
PZEM004Tv30 pzem1(&Serial2,16,17,0x02);
PZEM004Tv30 pzem2(&Serial2,16,17,0x03);

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
  float voltage = pzem0.voltage();
  if (!isnan(voltage)) {
    Serial.print("Voltage: "); Serial.print(voltage); Serial.println("V");
    voltage_phase01 = voltage;
  } else {
    voltage_phase01 = 0.0;
    Serial.println("Error reading voltage");
  }
  float current = pzem0.current();
  if (!isnan(current)) {
    current_phase01 = current;
  } else {
    current_phase01 = 0.0;
  }
  float power = pzem0.power();
  if (!isnan(power)) {
    power_phase01 = power;
  } else {
    power_phase01 = 0.0;
  }
  float energy = pzem0.energy();
  if (!isnan(energy)) {
    energy_phase01 = energy;
  }
  float pf = pzem0.pf();
  if (!isnan(pf)) {
    pf_phase01 = pf;
  } else {
    pf_phase01 = 0.0;
  }
  
  voltage = pzem1.voltage();
  if (!isnan(voltage)) {
    voltage_phase02 = voltage;
  } else {
    voltage_phase02 = 0.0;
  }
  current = pzem1.current();
  if (!isnan(current)) {
    current_phase02 = current;
  } else {
    current_phase02 = 0.0;
  }
  power = pzem1.power();
  if (!isnan(power)) {
    power_phase02 = power;
  } else {
    power_phase02 = 0.0;
  }
  energy = pzem1.energy();
  if (!isnan(energy)) {
    energy_phase02 = energy;
  }
  pf = pzem1.pf();
  if (!isnan(pf)) {
    pf_phase02 = pf;
  } else {
    pf_phase02 = 0.0;
  }

  voltage = pzem2.voltage();
  if (!isnan(voltage)) {
    voltage_phase03 = voltage;
  } else {
    voltage_phase03 = 0.0;
  }
  current = pzem2.current();
  if (!isnan(current)) {
    current_phase03 = current;
  } else {
    current_phase03 = 0.0;
  }
  power = pzem2.power();
  if (!isnan(power)) {
    power_phase03 = power;
  } else {
    power_phase03 = 0.0;
  }
  energy = pzem2.energy();
  if (!isnan(energy)) {
    energy_phase03 = energy;
  }
  pf = pzem2.pf();
  if (!isnan(pf)) {
    pf_phase03 = pf;
  } else {
    pf_phase03 = 0.0;
  }
  
}

void getData(){
 
  tax1 = preferences.getFloat("tax1", 0);
  tax2 = preferences.getFloat("tax2", 0);
  tax3 = preferences.getFloat("tax3", 0);
  noTax = preferences.getFloat("noTax", 0);
  pzem1LastEnergyReading = preferences.getFloat("pzem1LastEnergyReading", 0);
  pzem2LastEnergyReading = preferences.getFloat("pzem2LastEnergyReading", 0);
  pzem3LastEnergyReading = preferences.getFloat("pzem3LastEnergyReading", 0);

  //lastEnergyValue = preferences.getFloat("lastEnergyValue", 0);

  delay(1000);
  
}

void checkPzemResetEnergy() {
  if (pzem1LastEnergyReading > energy_phase01) {
    preferences.putFloat("pzem1LastEnergyReading", 0.0);
  } 
  
  if(pzem2LastEnergyReading > energy_phase02){
    preferences.putFloat("pzem2LastEnergyReading", 0.0);
  }

  if(pzem3LastEnergyReading > energy_phase03) {
    preferences.putFloat("pzem3LastEnergyReading", 0.0);
  }
}

void taxDivider(){

  checkPzemResetEnergy();
  
  // tax
  //     Saturday - 6. or Sunday - 7.
  //       tax1 06:00 to 21:30
  //       tax3 21:30 to 06:00
  //     Other days - 1,2,3,4,5.
  //       tax1 06:00 to 17:30 and 20:30 to 21:30
  //       tax2 17:30 to 20:30
  //       tax3 21:30 to 06:00

  
  float totalEnergyPzem1 = energy_phase01 - pzem1LastEnergyReading;
  float totalEnergyPzem2 = energy_phase02 - pzem2LastEnergyReading;
  float totalEnergyPzem3 = energy_phase03 - pzem3LastEnergyReading;

  float totalEnergy = totalEnergyPzem1 + totalEnergyPzem2 + totalEnergyPzem3;

  if(wrongTime){
    noTax = noTax + totalEnergy;
    preferences.putFloat("noTax", noTax);
  } else {
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

  

  preferences.putFloat("lastEnergyValue", totalEnergy);

  delay(1000);
  
}

void myTimerEvent(){

  saveLocalTime(); 
  getData();
  readPzems();
  taxDivider();
  
  // Sending data to blynk app.
  
  Blynk.virtualWrite(V0, voltage_phase01);
  Blynk.virtualWrite(V1, current_phase01);            
  Blynk.virtualWrite(V2, power_phase01);
  Blynk.virtualWrite(V3, pf_phase01);

  Blynk.virtualWrite(V4, voltage_phase02);
  Blynk.virtualWrite(V5, current_phase02);            
  Blynk.virtualWrite(V6, power_phase02);
  Blynk.virtualWrite(V7, pf_phase02);
  
  Blynk.virtualWrite(V8, voltage_phase03);
  Blynk.virtualWrite(V9, current_phase03);            
  Blynk.virtualWrite(V10, power_phase03);
  Blynk.virtualWrite(V11, pf_phase03);
  
  Blynk.virtualWrite(V12, tax1);
  Blynk.virtualWrite(V13, tax2);
  Blynk.virtualWrite(V14, tax3);
  Blynk.virtualWrite(V15, noTax);
  
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
  
}

// Loop. This area has always must be clean.

void loop() {
  Blynk.run();
  timer.run();
  
}
