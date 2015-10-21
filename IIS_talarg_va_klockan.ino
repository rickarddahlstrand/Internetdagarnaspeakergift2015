// Internetdagarna speakers gift - simple breadboard clock
//
// Heavily modified version of the Adafruit clock example using a seven segment display & DS1307 real-time clock from Adafruit.
// Modified and explanded by Rickard Dahlstrand, IIS.
//
// Must have the RTClib library installed too!  See:
//   https://github.com/mizraith/RTClib
//
// Designed specifically to work with the Adafruit LED 7-Segment backpacks
// and DS1307 real-time clock breakout:
// ----> http://www.adafruit.com/products/881
// ----> http://www.adafruit.com/products/880
// ----> http://www.adafruit.com/products/879
// ----> http://www.adafruit.com/products/878
// ----> https://www.adafruit.com/products/264
//
// Adafruit invests time and resources providing this open source code, 
// please support Adafruit and open-source hardware by purchasing 
// products from Adafruit!
//
// Originally written by Tony DiCola for Adafruit Industries.
// Released under a MIT license: https://opensource.org/licenses/MIT
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "RTClib.h"
#include "RTC_DS3231.h"


// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70


// Create display and DS3231 objects.  These are global variables that
// can be accessed from both the setup and loop function below.
Adafruit_7segment clockDisplay = Adafruit_7segment();
RTC_DS3231 rtc;

// Keep track of the hours, minutes, seconds displayed by the clock.
// Start off at 0:00:00 as a signal that the time should be read from
// the DS3231 to initialize it.
int hours = 0;
int minutes = 0;
int seconds = 0;

// The Global timeobject.
DateTime now;

// Set to false to display time in 12 hour format, or true to use 24 hour. Overriden by setting pin 5 to HIGH/LOW.
boolean TIME_24_HOUR = true;

// Remember if the colon was drawn on the display so it can be blinked
// on and off every second.
bool blinkColon = false;

// Reads the clock from the RTC and overwrited the RAM-clock. 
void gettimefromrtc() {

   // Get the time from the DS1307.
    now = rtc.now();
    // Print out the time for debug purposes:
    Serial.print("Read date & time from DS1307: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    // Now set the hours and minutes.
    hours = now.hour();
    minutes = now.minute();
    seconds = now.second();
    
}

void setup() {
  // Setup function runs once at startup to initialize the display
  // and DS1307 clock.

  // Setup Serial port to print debug output.
  Serial.begin(115200);
  Serial.println("Clock starting!");

  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);

  // Setup the DS1307 real-time clock.
  rtc.begin();

  gettimefromrtc();  

  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

}

void loop() {
  // Loop function runs over and over again to implement the clock logic.

  TIME_24_HOUR = !digitalRead(5);

  if (!digitalRead(3)) {
    now = rtc.now();
    if (now.minute() == 59) {
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), 0, 0));
    } else {
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute()+1, 0));
    }
    gettimefromrtc(); 
  }

  if (!digitalRead(4)) {
    now = rtc.now();
    if (now.hour() == 23) {
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), 0, now.minute(), 0));
    } else {
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour()+1, now.minute(), 0));
    }
    gettimefromrtc(); 
  }

  
  // Check if it's the top of the hour and get a new time reading
  // from the DS1307.  This helps keep the clock accurate by fixing
  // any drift.
  if (minutes == 0) {
    gettimefromrtc();
  }

  // Show the time on the display by turning it into a numeric
  // value, like 3:30 turns into 330, by multiplying the hour by
  // 100 and then adding the minutes.
  int displayValue = hours*100 + minutes;

  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR) {
    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (hours > 11) {
      displayValue -= 1200;
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (hours == 0) {
      displayValue += 1200;
    }
  }

  // Now print the time value to the display.
  
  clockDisplay.print(displayValue, DEC);

  // Add zero padding when in 24 hour mode and it's midnight.
  // In this case the print function above won't have leading 0's
  // which can look confusing.  Go in and explicitly add these zeros.
  if (TIME_24_HOUR && hours == 0) {
    // Pad hour 0.
  clockDisplay.writeDigitNum(1, 0);


    // Also pad when the 10's minute is 0 and should be padded.
    if (minutes < 10) {
      clockDisplay.writeDigitNum(3, 0);
    }
  }

  if (hours < 10) {
    clockDisplay.writeDigitNum(0, 0);
  }


  // Blink the colon by flipping its value every loop iteration
  // (which happens every second).
  blinkColon = !blinkColon;
  clockDisplay.drawColon(blinkColon);
  digitalWrite(13, !blinkColon);

  // Now push out to the display the new values that were set above.
  clockDisplay.writeDisplay();


  // Now increase the seconds by one.
  seconds += 1;
  // If the seconds go above 59 then the minutes should increase and
  // the seconds should wrap back to 0.
  if (seconds > 59) {
    seconds = 0;
    minutes += 1;
    // Again if the minutes go above 59 then the hour should increase and
    // the minutes should wrap back to 0.
    if (minutes > 59) {
      minutes = 0;
      hours += 1;
      // Note that when the minutes are 0 (i.e. it's the top of a new hour)
      // then the start of the loop will read the actual time from the DS1307
      // again.  Just to be safe though we'll also increment the hour and wrap
      // back to 0 if it goes above 23 (i.e. past midnight).
      if (hours > 23) {
        hours = 0;
      }
    }
  }

  // Pause for a second for time to elapse.  This value is in milliseconds
  // so 1000 milliseconds = 1 second.
  // Code above takes 2-3 ms to execute..
  delay(997);

  // Loop code is finished, it will jump back to the start of the loop
  // function again!
}
