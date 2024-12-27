#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;

int reset_button = 8;
int alarm_reset_button = 9;
int switch_button = 10;

DateTime = rtc.now();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(alarm_reset_button, INPUT);
  pinMode(reset_button, INPUT);
  pinMode(switch_button, INPUT);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {

  while (digitalRead(reset_button) != 1 || digitalRead(alarm_reset_button) != 1) {
    lcd.setCursor(0, 0);
    lcd.print("Time: ")
      regular_clock_mode();
  }

  if (digitalRead(reset_button) == 1) {
    lcd.clear();
    delay(1000);
    reset_normal_clock();
  }

  else if (digitalRead(alarm_reset_button) == 1) {
    lcd.clear();
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("Set alarm: ");

    set_alarm();
  }
}

//if reset is high, set back the alarm to 00:00

//if set_alarm is high, set clock to 00:00:00 and then make use of increment and decrement operations

//if alarm_start is high, return to regular clock and countdown

void reset_normal_clock() {
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()));
}

void reset_alarm_clock() {
}

void set_alarm() {
  lcd.setCursor(1, 2);
  lcd.print("00 : 00");
}

void regular_clock_mode() {
  lcd.setCursor(1, 2);
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.print(":");
  lcd.print(now.second());
}
