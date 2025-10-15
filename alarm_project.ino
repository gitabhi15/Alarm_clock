#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTClib.h>
#include <DS3232RTC.h>
#include <Keypad.h>

#define ROWS 4
#define COLS 4

byte rowPins[ROWS] = { 36, 38, 40, 42 };
byte colPins[COLS] = { 44, 46, 48, 50 };

char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

LiquidCrystal lcd(22, 24, 26, 28, 30, 32);
RTC_DS3231 rtc;
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int buzzPin = 11;
const int sqwPin = 2;

// Flags
volatile bool alarmTriggered = false;  // set in ISR
bool alarmMode = false;
bool settingMinutes = false;
bool settingHours = false;
bool showBootUp = false;
bool alarmReady = false;

// State variables
String mins = "";
String hrs = "";
unsigned long prevClockUpdate = 0;
const unsigned long clockUpdateInterval = 1000;
unsigned long bootUpStartTime = 0;
const unsigned long bootUpDuration = 4000;

void setup() {
  Serial.begin(9600);
  pinMode(buzzPin, OUTPUT);
  pinMode(sqwPin, INPUT_PULLUP);

  lcd.begin(16, 2);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  // Clear alarms and configure SQW pin for alarm interrupt mode
  rtc.clearAlarm(1);
  rtc.disableAlarm(2);
  rtc.writeSqwPinMode(DS3231_OFF);

  // Attach ISR for SQW pin (fires when alarm goes off, active LOW)
  attachInterrupt(digitalPinToInterrupt(sqwPin), alarmISR, FALLING);
  //syncClock();
}

void loop() {
  // Regular time display every second
  if (millis() - prevClockUpdate >= clockUpdateInterval && !alarmMode) {
    prevClockUpdate = millis();
    displayClock();
  }

  char key = keypad.getKey();
  if (key != NO_KEY) {
    handleKeyPress(key);
  }

  if (alarmMode) {
    handleAlarmSetting();
  }

  // Transition from boot-up screen to minutes input
  if (showBootUp && millis() - bootUpStartTime >= bootUpDuration) {
    lcd.clear();
    showBootUp = false;
    settingMinutes = true;
  }

  // If alarm has been triggered by RTC
  if (alarmTriggered) {
    soundAlarm();
  }
}

void handleKeyPress(char key) {
  // Enter alarm mode
  if (!alarmMode && key == 'A') {
    mins = "";
    hrs = "";
    alarmMode = true;
    showBootUp = true;
    bootUpStartTime = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("C to confirm");
    lcd.setCursor(0, 1);
    lcd.print("# to cancel");
    return;
  }

  // Handle alarm setting inputs (only after entering alarm mode)
  if (alarmMode && !showBootUp) {
    if (settingMinutes) {
      if (key == '#') {
        mins = "";
        clearRow(0);
      } else if (key == 'C') {
        settingMinutes = false;
        settingHours = true;
        clearRow(0);
      } else if (isdigit(key)) {
        if (mins.length() < 2) {
          mins += key;
        }
      }
    } else if (settingHours) {
      if (key == '#') {
        hrs = "";
        clearRow(0);
      } else if (key == 'C') {
        alarmReady = true;
      } else if (isdigit(key)) {
        if (hrs.length() < 2) {
          hrs += key;
        }
      }
    }
  }

  // Optional: allow canceling alarm mode before finishing
  if (alarmMode && key == 'D') {
    lcd.clear();
    lcd.print("Alarm canceled");
    delay(1000);
    lcd.clear();
    resetAlarmState();
  }
}


void handleAlarmSetting() {
  if (settingMinutes) {
    lcd.setCursor(0, 0);
    lcd.print("Minutes: ");
    lcd.setCursor(9, 0);
    lcd.print(mins + "  ");
  } else if (settingHours) {
    lcd.setCursor(0, 0);
    lcd.print("Hours: ");
    lcd.setCursor(9, 0);
    lcd.print(hrs + "  ");
  }

  if (alarmReady) {
    int m = mins.toInt();
    int h = hrs.toInt();
    DateTime now = rtc.now();

    // Set Alarm1 for today at H:M:00
    rtc.setAlarm1(DateTime(now.year(), now.month(), now.day(), h, m, 0), DS3231_A1_Hour);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm Set to:");
    lcd.setCursor(4, 1);
    lcd.print(h);
    lcd.print(" : ");
    lcd.print(m);
    delay(2000);
    lcd.clear();

    resetAlarmState();
  }
}

void resetAlarmState() {
  alarmMode = false;
  settingMinutes = false;
  settingHours = false;
  showBootUp = false;
  alarmReady = false;
  mins = "";
  hrs = "";
}

void displayClock() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.setCursor(2, 1);
  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(" : ");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(" : ");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());
}

// ISR to mark alarm as triggered
void alarmISR() {
  alarmTriggered = true;
}

void soundAlarm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WAKE UP!!!");

  for (int i = 0; i < 5; i++) {
    digitalWrite(buzzPin, HIGH);
    delay(500);
    digitalWrite(buzzPin, LOW);
    delay(500);
  }

  // Clear alarm flag in RTC so it can be triggered again later
  rtc.clearAlarm(1);

  alarmTriggered = false;  // reset flag
  lcd.clear();
}

void syncClock() {
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void clearRow(int row) {
  lcd.setCursor(0, row);
  lcd.print("                    ");
}
