// vi:ts=4
// ----------------------------------------------------------------------------
// edm - Electrical discharge machining
// Created by Marcin Czernik (& Adam Ziemkiewicz)
//
// This example code is licensed under GNU General Public License v3.0
// ----------------------------------------------------------------------------
//

#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <Encoder.h>

hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip
// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;

//encoder settings
#define encAPin  41 
#define encBPin  43 

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder enc(encAPin, encBPin);
//   avoid using pins with LEDs attached

long encOldPosition  = -999;

void setup()
{
  int status;

  // initialize LCD with number of columns and rows: 
  // hd44780 returns a status from begin() that can be used
  // to determine if initalization failed.
  // the actual status codes are defined in <hd44780.h>
  // See the values RV_XXXX
  //
  // looking at the return status from begin() is optional
  // it is being done here to provide feedback should there be an issue
  //
  // note:
  //  begin() will automatically turn on the backlight
  //
  status = lcd.begin(LCD_COLS, LCD_ROWS);
  if(status) // non zero status means it was unsuccesful
  {
    status = -status; // convert negative status value to positive number

    // begin() failed so blink error code using the onboard LED if possible
    hd44780::fatalError(status); // does not return
  }
  //do wrap the line
  lcd.lineWrap();
  
  // Print a message to the LCD
  lcd.print("'Electrical discharge machining' by Marcin Czernik (& Adam Ziemkiewicz)");

  Serial.begin(9600);
}

//update display content four times per second 
unsigned long displayUpdateFreq   =   4; 

//update display content if _this_ miliseconds
//elapsed since last update 
unsigned long displayUpdatePeriod = 1000/displayUpdateFreq; 

//time of last update
unsigned long display_last_update = 0;

//millis elapsed since last update
unsigned long display_elapsed = 0;

void loop() {
  long encPosition = enc.read();
  if (encPosition != encOldPosition) {
    encOldPosition = encPosition;
    Serial.print(encPosition);
    Serial.println("");
  }

  //update lcd display content if necessary
  display_elapsed = millis() - display_last_update;
  
  if( display_elapsed > displayUpdatePeriod ) {
    //update encPosition display
    lcd.setCursor(12,3);    //clear old content
    lcd.print("        ");
    lcd.setCursor(12,3);    //display new value
    lcd.print(encPosition, DEC);

    //restart update period
    display_last_update = millis();
  }
}
