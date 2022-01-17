#include <LiquidCrystal.h>
#include <Filters.h>
#include "ACS712.h"

const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

ACS712  ACS(A1, 5.0, 1023, 100);

float testFrequency = 50;
float windowLength = 40.0 / testFrequency;

int Sensor = A0;
float intercept = - 0.04;
float slope = 0.0266;
float current_Volts;

int buzzer = 4;
int RedLed = 3;
int YellowLed = 2;
int GreenLed = 1;

int a_sensor = A1;

unsigned long printPeriod = 1000;
unsigned long previousMillis = 0;

int previous_length[4] = {0, 0, 0, 0};



void setup() {
  lcd.begin(20, 4);
  pinMode(RedLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(YellowLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(a_sensor, INPUT);

  ACS.autoMidPoint();

  startScreen();
}

void loop() {

  RunningStatistics inputStats;
  inputStats.setWindowSecs( windowLength );
  while ( true ) {
    Sensor = analogRead(A0);  // read the analog in value:
    inputStats.input(Sensor);  // log to Stats function
    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();
      current_Volts = intercept + slope * inputStats.sigma();
      current_Volts = current_Volts * (40.3231);
      break;
    }
  }
  int mA = ACS.mA_AC()-69.65;
  homeScreen(mA, current_Volts);

}

int centerText (String x) {
  int len =  x.length();
  int rem = 20 - len;
  rem /= 2;
  return rem;
}


void homeScreen(int Isensor, float Vsensor) {
  Isensor = Isensor <= 0 ? 0:Isensor;
  float power = (Vsensor * Isensor)/1000; 

  register String displayText[4] = {
    "M-LUKU",
    "Volt's : " + String(int(Vsensor)) + "v",
    "Unit's : 0kWh",
    "Energy : "+(String(power))+"kWh",
  };
  bool clearScreen =  false;
  for (int i = 0; i < 4; i++) {
    if ( ( previous_length[i] - displayText[i].length()) > 0 ) {
      previous_length[i] = displayText[i].length();
      clearScreen = true;
    }
  }

  if (clearScreen) {
    lcd.clear();
  }

  for (int i = 0; i < 4; i++) {
    lcd.setCursor((i == 0 ) ? centerText(displayText[i]) : 0, i);
    lcd.print(displayText[i] + "");
  }
  if (Isensor > 0.0) {
    digitalWrite(GreenLed, 1);
    digitalWrite(YellowLed, 1);
  }
}

void startScreen() {
  register String displayText = "WELCOME TO M-LUKU";
  lcd.setCursor(centerText(displayText), 0);
  lcd.print(displayText);
  register bool state = 1;
  for (int i = 0; i < 20; i++) {
    lcd.setCursor(i, 2);
    lcd.print(".");
    digitalWrite(RedLed, state);
    state = !state;
    delay(400);
  }
  tone(buzzer, 2000, 500);
  delay(800);
  lcd.clear();
  digitalWrite(RedLed, 0);
}
