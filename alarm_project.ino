#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <RTClib.h>
#include <DS3232RTC.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS3231 rtc;

int clk_reset_button = 8;
int alarm_reset_button = 9;
int increment_button = 10;
int decrement_button = 11;
int confirm_alarm_button = 3;
int alarm_mode_button = 4;

DateTime now = rtc.now();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(alarm_reset_button, INPUT);
  pinMode(clk_reset_button, INPUT);
  pinMode(increment_button, INPUT);
  pinMode(decrement_button, INPUT);
  pinMode(confirm_alarm_button, INPUT);
  pinMode(alarm_mode_button, INPUT);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {

  lcd.setCursor(0, 0);
  lcd.print("Time: ")
    regular_clock_mode();


  if (digitalRead(clk_reset_button) == 1) {
    delay(1000);
    reset_normal_clock();
  }

  else if (digitalRead(alarm_mode_button) == 1) {
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
  lcd.clear();
  now = rtc.now();
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()));
}

void set_alarm() {
  lcd.clear();
  lcd.setCursor(1, 2);

  bool flag = true;

  int hrs = 0, mins = 0;
  while (flag == true) {

    // Update the display only when hours or minutes change
    lcd.setCursor(1, 2);
    lcd.print("Set Alarm: ");
    if (hrs < 10) {
      lcd.print("0");
    }
    lcd.print(hrs);
    lcd.print(" : ");
    if (mins < 10) {
      lcd.print("0");
    }
    lcd.print(mins);

    // Increment minutes
    if (digitalRead(increment_button) == HIGH) {
      mins++;
      delay(200);  // debounce
    }

    // Decrement minutes
    if (digitalRead(decrement_button) == HIGH) {
      mins--;
      delay(200);  // debounce
    }

    // Handle rollover for minutes and hours
    if (mins == 60) {
      mins = 0;
      hrs++;
    } else if (mins < 0) {
      mins = 59;
      hrs--;
    }

    // Handle rollover for hours
    if (hrs == 24) {
      hrs = 0;
    } else if (hrs < 0) {
      hrs = 23;
    }

    // Confirm the alarm setting
    if (digitalRead(alarm_confirm_button) == HIGH) {
      flag = false;
    }
  }

  // Reset alarm time if the reset button is pressed
  if (digitalRead(alarm_reset_button) == HIGH) {
    hrs = 0;
    mins = 0;
  }

  // Display the set alarm
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm set to ");
  if (hrs < 10) {
    lcd.print("0");
  }
  lcd.print(hrs);
  lcd.print(" : ");
  if (mins < 10) {
    lcd.print("0");
  }
  lcd.print(mins);

  // Adjust the alarm time on the RTC module
  rtc.setAlarm1(DateTime(now.year(), now.month(), now.day(), hrs, mins, 0), DS3231_A1_Hour);
}


void regular_clock_mode() {
  lcd.clear();
  lcd.setCursor(1, 2);

  int hrs = now.hour(), mins = now.minute(), secs = now.second();

  if (hrs < 0) {
    lcd.print("0");
  }
  lcd.print(now.hour());
  lcd.print(" : ");

  if (mins < 10) {
    lcd.print("0");
  }
  lcd.print(now.minute());
  lcd.print(" : ");

  if (secs < 10) {
    lcd.print("0");
  }
  lcd.print(now.second());
}
