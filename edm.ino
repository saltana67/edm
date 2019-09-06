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

// This optional setting causes Encoder to use more optimized code,
// It must be defined before Encoder.h is included.
// comment next line out in case of problems.
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

/**************
 * LCD
 **************/
hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip
// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;

/**************
 * encoder
 **************/
//encoder pins
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
//   for Arduino Mega Interrupt Pins are: 2, 3, 18, 19, 20, 21, 
//   avoid using pins with LEDs attached (for Arduino Mega it is pin 13)
const int encAPin  = 41; 
const int encBPin  = 43; 

Encoder enc(encAPin, encBPin);

long encPosition    = -999;
long encOldPosition = -999;


/**************
 * mode switch
 **************/
//mode switch pins
const int SWPIN_RESERVE  = 23;  //  A     1
const int SWPIN_DOWN     = 25;  //  A     3  
const int SWPIN_UP       = 27;  //  A     5
const int SWPIN_AUTO     = 29;  //  A     7
const int SWPIN_STOP     = 31;  //  C     6
const int SWPIN_ZERO     = 22;  //  A     0
const int SWPIN_VMAX     = 24;  //  A     2
const int SWPIN_VMIN     = 26;  //  A     4
const int SWPIN_FLUSH    = 28;  //  A     6

//defines for bit number of input pin in port
//        name            bit     port  bit
const int SWBIT_RESERVE = 1;  //  A     1
const int SWBIT_DOWN    = 3;  //  A     3  
const int SWBIT_UP      = 5;  //  A     5
const int SWBIT_AUTO    = 7;  //  A     7
const int SWBIT_STOP    = 6;  //  C     6
const int SWBIT_ZERO    = 0;  //  A     0
const int SWBIT_VMAX    = 2;  //  A     2
const int SWBIT_VMIN    = 4;  //  A     4
const int SWBIT_FLUSH   = 6;  //  A     6

//mode constants
//we use pin bit number as mode id
//        name            bit                 port  bit
const int mode_ZERO     = SWBIT_ZERO;     //  A     0
const int mode_RESERVE  = SWBIT_RESERVE;  //  A     1
const int mode_VMAX     = SWBIT_VMAX;     //  A     2
const int mode_DOWN     = SWBIT_DOWN;     //  A     3  
const int mode_VMIN     = SWBIT_VMIN;     //  A     4
const int mode_UP       = SWBIT_UP;       //  A     5
const int mode_FLUSH    = SWBIT_FLUSH;    //  A     6
const int mode_AUTO     = SWBIT_AUTO;     //  A     7

const int mode_STOP     = 8;              //  C     6

const int mode_INIT     = 9;  //pseudo mode for starting ...


const int RELAY_PIN     = 49; //  L     0

const int EDM_CURRENT_ADC_PIN = A0;

//constant strings
//mode labels for displaying
const String label_INIT     = "INIT...";
const String label_RESERVE  = "RESERVE";
const String label_DOWN     = "  UP   ";  
const String label_UP       = " DOWN  ";
const String label_AUTO     = " AUTO  ";
const String label_STOP     = " STOP  ";
const String label_ZERO     = " ZERO  ";
const String label_VMAX     = " VMAX  ";
const String label_VMIN     = " VMIN  ";
const String label_FLUSH    = " FLUSH ";

const String label_BLANKMODE= "       ";

const String modeLabel[10] = {
  /*0: mode_ZERO*/     label_ZERO    ,
  /*1: mode_RESERVE*/  label_RESERVE ,
  /*2: mode_VMAX*/     label_VMAX    ,
  /*3: mode_DOWN*/     label_DOWN    ,
  /*4: mode_VMIN*/     label_VMIN    ,
  /*5: mode_UP*/       label_UP      ,
  /*6: mode_FLUSH*/    label_FLUSH   ,
  /*7: mode_AUTO*/     label_AUTO    ,
  /*8: mode_STOP*/     label_STOP    ,
  /*9: mode_INIT*/     label_INIT
};

//other strings
                            //0         1         
                            //01234567890123456789

const String label_INFO    = "MACZ(c) EDM  v.00001";

int mode = mode_INIT;

/*******************************************************/
void setup()
/*******************************************************/
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
  lcd.clear();
  lcd.print(label_INFO);

  //all switch input pulled up
  pinMode(SWPIN_RESERVE  , INPUT_PULLUP);
  pinMode(SWPIN_DOWN     , INPUT_PULLUP);
  pinMode(SWPIN_UP       , INPUT_PULLUP);
  pinMode(SWPIN_AUTO     , INPUT_PULLUP);
  pinMode(SWPIN_STOP     , INPUT_PULLUP);
  pinMode(SWPIN_ZERO     , INPUT_PULLUP);
  pinMode(SWPIN_VMAX     , INPUT_PULLUP);
  pinMode(SWPIN_VMIN     , INPUT_PULLUP);
  pinMode(SWPIN_FLUSH    , INPUT_PULLUP);

  mode = mode_INIT;
  
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

byte  modeSwitches    = 0;
byte  modeSwitchesOld = 0;

int   oldMode = mode;

/*******************************************************/
void loop() {
/*******************************************************/

  //set to true if something change since last loop iteration
  boolean changed = false;
  
  //check switch position and get new mode 
  modeSwitchesOld = modeSwitches;
  modeSwitches = ~PINA;

  if( modeSwitches != modeSwitchesOld ) {
    changed = true;
    if( modeSwitches == 0 )
       ;//do nothing for now
    else {
      for( int i = 7; i >= 0; i-- ){
        if( modeSwitches & (1<<i) ) {
          if( (modeSwitches & ~(1<<i)) == 0 ){
            oldMode = mode;
            mode = i;
          }
          break;
        }
      }
    }
  }

  //get encoder value
  encPosition = enc.read();
  if (encPosition != encOldPosition) {
    encOldPosition = encPosition;
    changed = true;
  }

  //print current state to serial id changed 
  if( changed ) {
    Serial.print(mode);
    Serial.print("\t");
    Serial.print(modeSwitches);
    Serial.print("\t");
    Serial.print(encPosition);
    Serial.println("");
  }
  
  //update lcd display content if necessary
  display_elapsed = millis() - display_last_update;
  
  if( display_elapsed > displayUpdatePeriod ) {

    //update mode
    lcd.setCursor(0,2);     //clear old content
    lcd.print(label_BLANKMODE);
    lcd.setCursor(0,2);
    lcd.print(modeLabel[mode]);

    //update modeSwitch state
    lcd.setCursor(0,3);     //clear old content
    lcd.print("        ");
    lcd.setCursor(0,3);
    lcd.print(modeSwitches,BIN);

    //update encPosition display
    lcd.setCursor(12,3);    //clear old content
    lcd.print("        ");
    lcd.setCursor(12,3);    //display new value
    lcd.print(encPosition, DEC);

    //restart update period
    display_last_update = millis();
  }
}
