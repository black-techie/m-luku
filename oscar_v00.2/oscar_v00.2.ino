#include <LiquidCrystal.h>
#include <Filters.h>
#include <TimerOne.h>
#include <elapsedMillis.h>
#include "ACS712.h"

const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

ACS712  ACS(A1, 5.0, 1023, 100);

elapsedMillis voltage_timer;

float testFrequency = 50;
float windowLength = 40.0 / testFrequency;

int Sensor = A0;
float intercept = - 0.04;
float slope = 0.0266;
float current_Volts = 0.0;
float mA = 0.0;

int buzzer = 4;
int RedLed = 3;
int YellowLed = 2;
int GreenLed = 1;

int a_sensor = A1;

unsigned long printPeriod = 1000;
unsigned long previousMillis = 0;

volatile double Energy = 0.0;

int previous_length[4] = {0, 0, 0, 0};
float Units = 0.01;

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);
  pinMode(RedLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(YellowLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(a_sensor, INPUT);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(calcurateEnergy);

  ACS.autoMidPoint();

  startScreen();
  voltage_timer = 0;
}

void loop() {
  volatile float rem_units = Units - Energy;
  RunningStatistics inputStats;
  inputStats.setWindowSecs( windowLength );
  mA = ACS.mA_AC() - 79.65;
  mA = mA <= 0 ? 0.0 : mA;
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

  homeScreen(Energy , current_Volts, rem_units);
}

int centerText (String x) {
  int len =  x.length();
  int rem = 20 - len;
  rem /= 2;
  return rem;
}

void homeScreen(double Energy_reading, float Vsensor, float units) {
  register String displayText[4] = {
    "M-LUKU",
    "Volt's : " + String(int(Vsensor)) + "v",
    "Unit's : " + String(units) + "kW/h",
    "Energy : " + (String(Energy_reading)) + "kW/h",
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
  if (units > 5) {
    digitalWrite(  GreenLed , 1);
    digitalWrite( YellowLed , 1);
  }
  if ( units > 0 && units  < 5) {
    digitalWrite(  GreenLed , 1);
    digitalWrite( YellowLed , 0);
  }
  if (units <= 0) {
    digitalWrite(  GreenLed , 0);
    digitalWrite( YellowLed , 0);
  }


}

void startScreen() {
  register String displayText = "WELCOME TO M-LUKU";
  register bool state = 1;
  lcd.setCursor(centerText(displayText), 0);
  lcd.print(displayText);
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

void calcurateEnergy (void) {
  Energy += (current_Volts * (mA / 1000) * 1) / 360000;
  Serial.println(1);
}
