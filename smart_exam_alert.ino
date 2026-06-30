#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LEDs
const int led1 = 2;
const int led2 = 3;
const int led3 = 4;

// Buttons
const int durationBtn = 5;
const int startBtn = 6;
const int resetBtn = 7;

// Buzzer
const int buzzer = 8;

int durationMinutes = 1;
bool examStarted = false;

DateTime startTime;

bool warning30Given = false;
bool warning10Given = false;

void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  pinMode(durationBtn, INPUT_PULLUP);
  pinMode(startBtn, INPUT_PULLUP);
  pinMode(resetBtn, INPUT_PULLUP);

  pinMode(buzzer, OUTPUT);

  lcd.init();
  lcd.backlight();

  if (!rtc.begin())
  {
    lcd.clear();
    lcd.print("RTC Error!");
    while (1);
  }

  showMenu();
}

void loop()
{
  if (!examStarted)
  {
    if (digitalRead(durationBtn) == LOW)
    {
      delay(250);

      durationMinutes++;

      if (durationMinutes > 3)
        durationMinutes = 1;

      showMenu();
    }

    if (digitalRead(startBtn) == LOW)
    {
      delay(250);

      startTime = rtc.now();

      examStarted = true;

      warning30Given = false;
      warning10Given = false;

      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);

      lcd.clear();
    }

    return;
  }

  if (digitalRead(resetBtn) == LOW)
  {
    resetSystem();
    return;
  }

  DateTime now = rtc.now();

  long elapsed =
      now.unixtime() -
      startTime.unixtime();

  long totalSeconds =
      durationMinutes * 60;

  long remaining =
      totalSeconds - elapsed;

  if (remaining < 0)
    remaining = 0;

  int minutes = remaining / 60;
  int seconds = remaining % 60;

  lcd.setCursor(0, 0);
  lcd.print("Time Left      ");

  lcd.setCursor(0, 1);

  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");

  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
  lcd.print("   ");

  // 30-second warning
  if (remaining <= 30 && remaining > 10 && !warning30Given)
  {
    warning30Given = true;

    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);

    tone(buzzer, 1000);
    delay(300);
    noTone(buzzer);
  }

  // 10-second warning
  if (remaining <= 10 && remaining > 0 && !warning10Given)
  {
    warning10Given = true;

    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);

    for (int i = 0; i < 2; i++)
    {
      tone(buzzer, 1500);
      delay(200);
      noTone(buzzer);
      delay(200);
    }
  }

  // Time Up
  if (remaining == 0)
  {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("TIME UP!");

    lcd.setCursor(0, 1);
    lcd.print("Stop Writing");

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);

    for (int i = 0; i < 5; i++)
    {
      tone(buzzer, 2000);
      delay(200);

      noTone(buzzer);
      delay(200);
    }

    while (1);
  }

  delay(200);
}

void showMenu()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Select Time");

  lcd.setCursor(0, 1);
  lcd.print(durationMinutes);
  lcd.print(" Minute ");
}

void resetSystem()
{
  examStarted = false;

  warning30Given = false;
  warning10Given = false;

  durationMinutes = 1;

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);

  noTone(buzzer);

  showMenu();
}