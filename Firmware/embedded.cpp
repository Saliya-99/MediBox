#include<PubSubClient.h>
#include<WiFi.h>
#include "DHTesp.h"
#include "RTClib.h"
#include <ESP32Servo.h>

const int servoPin = 18;

Servo servo;
RTC_DS1307 rtc;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

int numofDays = 0;
int  playAlarm_1  = 0;
int  playAlarm_2  = 0;
int  playAlarm_3  = 0;
int  scheduler  = 0;
int alarm_1_hour = 0;
int alarm_1_min = 0;
int alarm_2_hour = 0;
int alarm_2_min = 0;
int alarm_3_hour = 0;
int alarm_3_min = 0;
float LDR_Val = 0;    
int LDR_PIN =34; 
int servoAngle;
int angleOffset = 30;
float CF = 0.75;
int pos;
int buzzerDelay= 100;
int buzzerFrequency= 250;
int buzzerMode = 0;

int day;
int hour;
int minute;
char tempAr[6];
char humAr[6];
DHTesp dhtSensor;
const int DHT_PIN = 15;

void setup() {
  Serial.begin(115200);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  servo.attach(servoPin, 500, 2400);
  setupWifi();
  setupMqtt();
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop() {
  if (!mqttClient.connected()){
    connectToBroker();
  }
  mqttClient.loop();
  if (hour ==0 && minute == 0){
    numofDays -= 1;
  }
  servoTurn();
  readTime();
  readLDR();
  buzzerAlarms();
  displayAlertHumAndTemp();
  updateTempAndHum();
  mqttClient.publish("Temp", tempAr);
  mqttClient.publish("Hum", humAr);

  delay(1000);
  
}

void readTime(){
  DateTime now = rtc.now();
  day = now.day();
  hour = now.hour();
  minute = now.minute();
  Serial.println(hour);
  Serial.println(minute);
}

void buzzerAlarms(){
    if (scheduler == 1){
    if (playAlarm_1==1){
      if (hour == alarm_1_hour && minute == alarm_1_min){
        PlayBuzzer();
      }
      else{
          OffBuzzer();
      }
    }
    else{
      OffBuzzer();
    }

    if (playAlarm_2==1){
      if (hour == alarm_2_hour && minute == alarm_2_min){
        PlayBuzzer();
      }
      else{
          OffBuzzer();
      }
    }
    else{
      OffBuzzer();
    }

    if (playAlarm_3==1){
      if (hour == alarm_3_hour && minute == alarm_3_min){
        PlayBuzzer();
      }
      else{
          OffBuzzer();
      }
    }
    else{
      OffBuzzer();
    }
  }
}

void setupWifi(){
  WiFi.begin("Wokwi-GUEST","");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

}

void PlayBuzzer(){
  digitalWrite(13, HIGH);
  delay(100);
  tone(13, buzzerFrequency, buzzerDelay);
  digitalWrite(13, LOW);
  delay(100);
 
}
void OffBuzzer(){

  digitalWrite(13, LOW);
 
 
}
void setupMqtt(){
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(callback);
}
void connectToBroker(){
  while (!mqttClient.connected()){
    Serial.print("Attempting...");
    if (mqttClient.connect("ESP-32-36536")){
      Serial.println("Connected");
      mqttClient.subscribe("AlarmOnOff");
      mqttClient.subscribe("Alarm_1_OnOff");
      mqttClient.subscribe("Alarm_1_Time");
      mqttClient.subscribe("Alarm_2_OnOff");
      mqttClient.subscribe("Alarm_2_Time");
      mqttClient.subscribe("Alarm_3_OnOff");
      mqttClient.subscribe("Alarm_3_Time");
      mqttClient.subscribe("NumOfDaysToRepeat");
      mqttClient.subscribe("minimumAngle");
      mqttClient.subscribe("CF");
      mqttClient.subscribe("buzzerDelay");
      mqttClient.subscribe("buzzerfreq");
      mqttClient.subscribe("buzzerType");
      
    }
    else {
      Serial.println("Failed");
      Serial.println(mqttClient.state());
      delay(5);
    }
  }
}
void updateTempAndHum(){
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  String(data.temperature, 2).toCharArray(tempAr,6);
  String(data.humidity, 1).toCharArray(humAr,6);
  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
  Serial.println("---");
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message:Topic[");
  Serial.print(topic);
  Serial.println("]");
  char incdata[length];
  int incdataNum[length];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    incdata[i] = (char)payload[i];
    incdataNum[i] = (char)payload[i]-'0';
  }
  Serial.println();
  
  if (strcmp(topic, "AlarmOnOff") == 0){
     if (incdata[0] == '1'){
      scheduler = 1;
 
     }
     else if (incdata[0] == '0'){
      scheduler = 0;
     }
  }


  if (strcmp(topic, "Alarm_1_OnOff") == 0){
     if (incdata[0] == '1'){
      playAlarm_1 = 1;
 
     }
     else if (incdata[0] == '0'){
      playAlarm_1 = 0;
     }
  }

  if (strcmp(topic, "Alarm_2_OnOff") == 0){
     if (incdata[0] == '1'){
      playAlarm_2 = 1;
 
     }
     else if (incdata[0] == '0'){
      playAlarm_2 = 0;
     }
  }

  if (strcmp(topic, "Alarm_3_OnOff") == 0){
     if (incdata[0] == '1'){
      playAlarm_3 = 1;
 
     }
     else if (incdata[0] == '0'){
      playAlarm_3 = 0;
     }
  }

  if (strcmp(topic, "Alarm_1_Time") == 0){

    int timeValue = 0;
    for (int i = 0;i<length;i++){
        timeValue += (incdata[i]-'0')*pow(10,(length-1-i));
    }

    alarm_1_hour = timeValue/(1000*3600);
    alarm_1_min =  (((float)timeValue/(1000*3600))-alarm_1_hour)*60;
    Serial.println(alarm_1_hour);
    Serial.println(alarm_1_min);
    

  }


  if (strcmp(topic, "Alarm_2_Time") == 0){

    int timeValue = 0;
    for (int i = 0;i<length;i++){
        timeValue += (incdata[i]-'0')*pow(10,(length-1-i));
    }

    alarm_2_hour = timeValue/(1000*3600);
    alarm_2_min =  (((float)timeValue/(1000*3600))-alarm_2_hour)*60;
    Serial.println(alarm_2_hour);
    Serial.println(alarm_2_min);
    

  }



  if (strcmp(topic, "Alarm_3_Time") == 0){

    int timeValue = 0;
    for (int i = 0;i<length;i++){
        timeValue += (incdata[i]-'0')*pow(10,(length-1-i));
    }

    alarm_3_hour = timeValue/(1000*3600);
    alarm_3_min =  (((float)timeValue/(1000*3600))-alarm_3_hour)*60;
    Serial.println(alarm_3_hour);
    Serial.println(alarm_3_min);
    
  }

  if (strcmp(topic, "NumOfDaysToRepeat") == 0){

    numofDays = incdata[0] - '0';
    
  }

  if (strcmp(topic, "CF") == 0){

    CF = atof(&incdata[0]);

    
  }

  if (strcmp(topic, "minimumAngle") == 0){
 
    angleOffset = atof(&incdata[0]);

    
  }

  if (strcmp(topic, "buzzerType") == 0){

  buzzerMode = atof(&incdata[0]);

  
 }

  if (strcmp(topic, "buzzerDelay") == 0){

  buzzerDelay = atof(&incdata[0]);

 }

  if (strcmp(topic, "buzzerfreq") == 0){

  buzzerFrequency = atof(&incdata[0]);

  
 }


  

}

void displayAlertHumAndTemp(){
  int humidity = (humAr[0]-'0')*10+humAr[1]-'0';
  int temperature = (tempAr[0]-'0')*10+tempAr[1]-'0';
  if ( humidity>85 |humidity<35 | temperature>35 | temperature<5){
     mqttClient.publish("alert", "RISKY TH");
  }
}
void readLDR(){
  LDR_Val = (float)analogRead(LDR_PIN)/4063;
  char ldr[6];
  dtostrf(LDR_Val, 4, 3, ldr);
   mqttClient.publish("UVINDEX", ldr);
  Serial.println(LDR_Val);

}
void servoTurn(){
  servoAngle = angleOffset+(180-angleOffset)*LDR_Val*CF;
  Serial.print("Servo: ");
  Serial.println(servoAngle);
  servo.write(servoAngle);

  

}
