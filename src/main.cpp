#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Servo.h>
#include <RTClib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define ENCODER_CLK 2
#define ENCODER_DT  3
#define button1 13
#define button2 12

enum class SystemState{
  Blinds,
  Open,
  Close
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo servo;
RTC_DS1307 clock;
SystemState systemState = SystemState::Blinds;
int timeValue = 0;
bool action = false;
int servoPos = 90;
int oldValue1 = LOW;
int oldValue2 = LOW;
int period = 50;
unsigned long time_now = 0;
struct TimeOfDay {
  uint8_t hour;   
  uint8_t minute; 
};
bool isTriggered = false;
int lastClk = HIGH;
TimeOfDay openTime = {11, 50};
TimeOfDay closeTime = {20, 30};
TimeOfDay tempTime = {0,0};

int lastCheckedValue = -1;
String formatTime(TimeOfDay time) {
  String value = "";

  if (time.hour < 10) {
    value += "0";
  }
  value += String(time.hour);

  value += ":";

  if (time.minute < 10) {
    value += "0";
  }
  value += String(time.minute);

  return value;
}

TimeOfDay convertValueToTime(int value){
  value %= 144;
  uint8_t hours = value / 6;
  uint8_t minutes = (value % 6) * 10;
  return TimeOfDay{hours, minutes};
}

int convertTimeToValue(TimeOfDay time){
  int value = time.minute / 10 + time.hour * 6;
  return value;
}

bool isFirstTimeCloser(TimeOfDay timeA, TimeOfDay timeB) {
  DateTime now = clock.now();
  int nowValue = (now.hour() * 6) + now.minute() / 10;
  
  int diffA = (convertTimeToValue(timeA) - nowValue + 144) % 144;
  int diffB = (convertTimeToValue(timeB) - nowValue + 144) % 144;

  return (diffA == 0 ? 144 : diffA) < (diffB == 0 ? 144 : diffB);
}


void refreshDisplay(){
    display.clearDisplay();

  if(systemState == SystemState::Blinds){

    String blindsSymbol = "/";
    if(servoPos >= 0 && servoPos <= 20){
      blindsSymbol = "|";
    }
    else if(servoPos >= 21 && servoPos <= 65){
      blindsSymbol = "/";
    }
    else if(servoPos >= 66 && servoPos <= 125){
      blindsSymbol = "-";
    }
    else if(servoPos >= 126 && servoPos <= 155){
      blindsSymbol = "\\";
    }
    else{
      blindsSymbol = "|";
    }
  
  
    int offsetFromOpen = abs(servoPos - 90);
    int percent = map(offsetFromOpen, 0, 90, 100, 0);
    DateTime now = clock.now();
  
    String currentTimeStr = formatTime({now.hour(), now.minute()});
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(currentTimeStr);
  
    display.setTextSize(4);
    display.setCursor(10, 18);
    display.print(blindsSymbol);
  
    display.setTextSize(2);
    display.setCursor(65, 14);
    display.print(servoPos);
    display.setTextSize(1);
    display.print((char)247); 
  
    display.setTextSize(1);
    display.setCursor(65, 34);
    display.print(percent);
    display.print("% OPEN");
  
    display.setCursor(0, 56);
  
    
    if(isFirstTimeCloser(openTime, closeTime)){
      display.print("Next opening: ");
      display.print(formatTime(openTime));
    }
    else{
      display.print("Next closing: ");
      display.print(formatTime(closeTime));
    }
  }
  else{
    display.setTextSize(2);
    display.setCursor(0, 0);

    if(systemState == SystemState::Open){
      display.println("Set open");
    }
    else{
      display.println("Set close");
    }
    display.println("time:");

    display.setCursor(20, 40);
    display.setTextSize(3);
    display.print(formatTime(tempTime));
  }

  display.display();
}

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.display();  
  servo.attach(5);

  while (!clock.begin()) {
  Serial.println("Couldn't find RTC, retrying...");
  delay(500);
  }

  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);

  refreshDisplay();
}

void saveValue(){
  if(systemState == SystemState::Open){
    openTime = tempTime;
  }
  else if(systemState == SystemState::Close){
    closeTime = tempTime;
  }
  systemState = SystemState::Blinds;
}


int readEncoder(){
  int newClk = digitalRead(ENCODER_CLK);
  if (newClk != lastClk) {
    lastClk = newClk;
    int dtValue = digitalRead(ENCODER_DT);
    if (newClk == LOW && dtValue == HIGH) {
      return 1;
    }
    if (newClk == LOW && dtValue == LOW) {
      return -1;
    }
  }
  return 0;
}

void checkButtons(){
  int newValue1 = digitalRead(button1);
  int newValue2 = digitalRead(button2);
  if(newValue1 == HIGH && newValue2 == HIGH){
    oldValue1 = newValue1;
    oldValue2 = newValue2;
    return; 
  }

  bool button1Pressed = (newValue1 != oldValue1) && (newValue1 == HIGH);
  if(button1Pressed)
  {
    if(systemState == SystemState::Blinds){
      systemState = SystemState::Open;
      tempTime = openTime;
      timeValue = convertTimeToValue(openTime);
    }
    else{
      saveValue();
    }
    action = true;
  }

  bool button2Pressed = (newValue2 != oldValue2) && (newValue2 == HIGH);
  if(button2Pressed)
  {
    if(systemState == SystemState::Blinds){
      systemState = SystemState::Close;
      tempTime = closeTime;
      timeValue = convertTimeToValue(closeTime);
    }
    else{
      systemState = SystemState::Blinds;
    }
    action = true;
  }

  oldValue1 = newValue1;
  oldValue2 = newValue2;
}


void changeTime(int value){
  timeValue = (timeValue + value + 144) % 144;
  tempTime = convertValueToTime(timeValue);
}

void checkSchedule(){
  DateTime now = clock.now();
  int nowValue = convertTimeToValue({now.hour(), now.minute()});
  int openValue = convertTimeToValue(openTime);
  int closeValue = convertTimeToValue(closeTime);

  if (nowValue != lastCheckedValue) {
    lastCheckedValue = nowValue;
    isTriggered = false; 
  }

  if (isTriggered) {
    return;
  }

  if(openValue == nowValue)
  {
    servoPos = 90;
    servo.write(servoPos);
    action = true;
    isTriggered = true;
  }
  else if (closeValue == nowValue)
  {
    servoPos = 180;
    servo.write(servoPos);
    action = true;
    isTriggered = true;
  }
}

int lastDisplayedMinute = -1;
void loop() {

  checkSchedule();

  DateTime now = clock.now();
  if (now.minute() != lastDisplayedMinute) {
    lastDisplayedMinute = now.minute();
    action = true;
  }

  checkButtons();
  int encoderValue = readEncoder();
  if (encoderValue != 0) {
    action = true;
    Serial.println(encoderValue);
    if (systemState == SystemState::Blinds) {
      servoPos = constrain(servoPos + encoderValue * 5, 0, 180);
      servo.write(servoPos);
    } else {
      changeTime(encoderValue);
    }
  }

  if (action) {
    refreshDisplay();
    action = false;
  }
}