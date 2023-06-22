#include<PubSubClient.h>
#include<WiFi.h>
#include "DHTesp.h"
#include "RTClib.h"
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27
#define LCD_COLUMNS 20
#define LCD_LINES   4

const int servoPin = 18;

Servo servo;
RTC_DS1307 rtc;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);
DHTesp dhtSensor;
//variables
//schedulaer

uint8_t numofDays = 0;
uint8_t  playAlarm_1  = 0;
uint8_t  playAlarm_2  = 0;
uint8_t  playAlarm_3  = 0;
uint8_t  scheduler  = 0;
uint8_t alarm_1_hour = 0;
uint8_t alarm_1_min = 0;
uint8_t alarm_2_hour = 0;
uint8_t alarm_2_min = 0;
uint8_t alarm_3_hour = 0;
uint8_t alarm_3_min = 0;
//min and max value of temperature and humidity
uint8_t minTemp;
uint8_t maxTemp;
uint8_t minHum;
uint8_t maxHum;
//Light intensity
float LDR_Val = 0;    
int LDR_PIN =34; 
//shade controller
uint8_t servoAngle;
uint8_t angleOffset = 30;
float CF = 0.75;
int pos;
//buzzer settings 
int buzzerDelay= 100;
int buzzerFrequency= 250;
uint8_t buzzerMode = 0;
//RTC module
uint8_t day;
uint8_t hour;
uint8_t minute;
char tempAr[9];
char humAr[6];
//dht22 sensor
const int DHT_PIN = 15;
//LCD display controll

uint8_t gotoAlarm = 0;
uint8_t gotomainMenu=0;



 
// void IRAM_ATTR toggleAlarm()
// {
//   stopAlarm = 1;
//   Serial.println("Interrupt Called");
// }

void setup() {
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  servo.attach(servoPin, 500, 2400);
  setupWifi();
  setupMqtt();
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  lcd.init();
  lcd.backlight();

  // pinMode(25, OUTPUT);
  // digitalWrite(25,LOW);
  // attachInterrupt(25, toggleAlarm, FALLING);
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
  mqttClient.publish("Tempt", tempAr);
  mqttClient.publish("Hum", humAr);
  lcd_display();
  Serial.print("Alarm_1: ");
  Serial.print((alarm_1_hour));
  Serial.print(":");
  Serial.println((alarm_1_min));
  Serial.print("Alarm_2: ");
  Serial.print(alarm_2_hour);
  Serial.print(":");
  Serial.println(alarm_2_min);
  Serial.print("Alarm_3: ");
  Serial.print(alarm_3_hour);
  Serial.print(":");
  Serial.println(alarm_3_min);

  delay(100);
  
}

void lcd_display(){



  if (digitalRead(12)==0){
    gotoAlarm = 1;
  }
  if (digitalRead(14)==0){
      gotomainMenu = 1;
      gotoAlarm =0;
  }
  // Serial.println(digitalRead(12));

  if (gotoAlarm==1){

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Alarm 1: ");

    lcd.setCursor(10, 0);
    lcd.print(alarm_1_hour);

    lcd.setCursor(12, 0);
    lcd.print(":");
    lcd.setCursor(13, 0);
    lcd.print(alarm_1_min);

    
    lcd.setCursor(0, 1);
    lcd.print("Alarm 2: ");

    lcd.setCursor(10, 1);
    lcd.print(alarm_2_hour);

    lcd.setCursor(12, 1);
    lcd.print(":");
    lcd.setCursor(13, 1);
    lcd.print(alarm_2_min);

    
    lcd.setCursor(0, 2);
    lcd.print("Alarm 3: ");

    lcd.setCursor(10, 2);
    lcd.print(alarm_3_hour);

    lcd.setCursor(12, 2);
    lcd.print(":");
    lcd.setCursor(13, 2);
    lcd.print(alarm_3_min);
    
  }
  
  else{

    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print("WELCOME");
    lcd.setCursor(7, 1);
    lcd.print(hour);
    lcd.setCursor(9, 1);
    lcd.print(":");
    lcd.setCursor(10, 1);
    lcd.print(minute);

  }



}

void readTime(){
  DateTime now = rtc.now();
  day = now.day();
  hour = now.hour();
  minute = now.minute();
  Serial.print("Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
}

void buzzerAlarms(){


    if ((scheduler == 1) && (numofDays>0)){
      Serial.println("In scheduler");
      if (playAlarm_1==1){
        Serial.println("in al 1");
        if (((hour-alarm_1_hour)==0) && ((minute - alarm_1_min)==0)){
          Serial.println("in alarm_1");
          PlayBuzzer();
        }
      }


      if (playAlarm_2==1){
        if ((hour == alarm_2_hour) && (minute == alarm_2_min)){
          PlayBuzzer();
        }
      }

      if (playAlarm_3==1){
        if ((hour == alarm_3_hour) && (minute == alarm_3_min)){
          PlayBuzzer();
        }
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

  Serial.println("alarm:");
  if (buzzerMode ==0){  
    // digitalWrite(13, HIGH);
    // delay(100);

    tone(13, buzzerFrequency, buzzerDelay);
    delay(100);

    
    // digitalWrite(13, LOW);
    // delay(100);
     
      
  }
  else{
   
    tone(13, buzzerFrequency, buzzerDelay);
  }


 
}
void OffBuzzer(){

  digitalWrite(13, LOW);
 
 
}
void setupMqtt(){
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(callback);
}
void connectToBroker(){
  uint8_t count_con = 0;
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
      mqttClient.subscribe("minTemp");
      mqttClient.subscribe("maxTemp");
      mqttClient.subscribe("minHum");
      mqttClient.subscribe("maxHum");
      
    }
    else {
      count_con += 1;
      if (count_con==5){
      break;
      }
      Serial.println("Failed");
      Serial.println(mqttClient.state());
      delay(5);
    }
  }
}
void updateTempAndHum(){
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  String(data.temperature, 1).toCharArray(tempAr,9);
  String(data.humidity, 1).toCharArray(humAr,6);
  Serial.println("Temp: " + String(data.temperature, 1) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
  Serial.println("---");
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message:Topic[");
  Serial.print(topic);
  Serial.print("] ");
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
    alarm_1_min =  (timeValue-alarm_1_hour*3600*1000)/(60*1000);
    

  }


  if (strcmp(topic, "Alarm_2_Time") == 0){



    int timeValue = 0;
    for (int i = 0;i<length;i++){
        timeValue += (incdata[i]-'0')*pow(10,(length-1-i));
    }


    alarm_2_hour = timeValue/(1000*3600);
    alarm_2_min =  (timeValue-alarm_2_hour*3600*1000)/(60*1000);
  }



  if (strcmp(topic, "Alarm_3_Time") == 0){



    int timeValue = 0;
    for (int i = 0;i<length;i++){
        timeValue += (incdata[i]-'0')*pow(10,(length-1-i));
    }

    alarm_3_hour = timeValue/(1000*3600);
    alarm_3_min =  (timeValue-alarm_3_hour*3600*1000)/(60*1000);
    
  }

  if (strcmp(topic, "NumOfDaysToRepeat") == 0){

    numofDays = atof(&incdata[0]);
    
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

  if (strcmp(topic, "minTemp") == 0){

  minTemp = atof(&incdata[0]);

  
 }  
  if (strcmp(topic, "maxTemp") == 0){

  maxTemp = atof(&incdata[0]);

  
 } 
  if (strcmp(topic, "minHum") == 0){

  minHum = atof(&incdata[0]);

  
 }  
  if (strcmp(topic, "maxHum") == 0){

  maxHum = atof(&incdata[0]);

  
 }
  

}

void displayAlertHumAndTemp(){
  int humidity = (humAr[0]-'0')*10+humAr[1]-'0';
  int temperature = (tempAr[0]-'0')*10+tempAr[1]-'0';
  if ( humidity>maxHum |humidity<minHum | temperature>maxTemp | temperature<minTemp){
     mqttClient.publish("alert", "RISKY TH");
  }
}
void readLDR(){
  LDR_Val = 1-(float)analogRead(LDR_PIN)/4063;
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
