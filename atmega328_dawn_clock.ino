#define CLK 5 // tm1637 pins
#define DIO 4

#define DIM_PIN 3 // mosfet pin

#define LIGHT_DUTY 3 // 1...255
#define LIGHT_DURATION 120 // seconds

#define START_HOUR 6
#define START_MINUTE 20

#define END_HOUR 6
#define END_MINUTE 40

#define DAYS_OF_WEEK 0b0111110 // sa, fr, th, we, tu, mo, su

#define DAWN_STEP_PERIOD ((END_HOUR*60+END_MINUTE) - (START_HOUR*60+START_MINUTE)) * 60000 / 255

#include "GyverTM1637.h"
GyverTM1637 disp(CLK, DIO);

#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc; // i use ds1307, but it is working

uint32_t dot_timer = 0;
uint32_t dawn_timer = 0;
uint32_t light_timer = LIGHT_DURATION;
uint8_t duty = 0;
_Bool dot_flag = 0;
_Bool dawn_flag = 0;
_Bool light_flag = 1;

DateTime now;

void setup() {
  pinMode(DIM_PIN, OUTPUT);
  pinMode(13, OUTPUT);

  //  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // uncomment for time setting, flash, comment, flash again. else this time will be set after every reset

  now = rtc.now();
  disp.displayClock(now.hour(), now.minute());
  disp.brightness(0); // 0...7

  analogWrite(DIM_PIN, LIGHT_DUTY);
}

void loop() {
  if (millis() - dot_timer > 500) {
    dot_timer = millis();

    dot_flag = !dot_flag;
    disp.point(dot_flag);

    if (dot_flag) { // one time at second

      if (light_flag && !(--light_timer)) {
        light_flag = 0;
        analogWrite(DIM_PIN, 0);
      }

      now = rtc.now();
      if (now.second() == 0) { // one time at minute update display
        disp.displayClock(now.hour(), now.minute());
      }
      if ((1 << now.dayOfTheWeek()) & DAYS_OF_WEEK &&                 // if today is set
          now.hour() == START_HOUR && now.minute() == START_MINUTE) { // if time to start dawn, up flag
        dawn_flag = 1;
      }
    }
  }

  if (dawn_flag && millis() - dawn_timer > DAWN_STEP_PERIOD) { // if time to dawn, gradually increase brightness
    dawn_timer = millis();
    analogWrite(DIM_PIN, (duty * duty + 0xFF) >> 8); // why not analogWrite(DIM_PIN, duty)? It is simple crt correction :)
    duty++;
    if (duty == 255) dawn_flag = 0; // if the time is over, continue the lighting
    //if (duty == 0) dawn_flag = 0; // if the time is over, turn off the lighting
  }
}
