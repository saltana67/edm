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
//#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include <AccelStepper.h>

#include"timer-api.h"

/*
 * mode/state functions prototypes
 */

void inline modeStopExec();

void inline modeVMinEnter();
void inline modeVMinExec();

void inline modeVMaxEnter();
void inline modeVMaxExec();

void inline modeManualEnter(boolean dir);
void inline modeManualExec();
void inline modeManualLeave(boolean dirUp);

void inline modeAutoEnter();
void inline modeAutoExec();
void inline modeAutoLeave();

void inline modeFlushEnter();
void inline modeFlushExec();

void inline modeZeroExec();

/* 
 * helper functions prototypes 
 */

// print current state to serial
void inline serialPrintState();

// update LCD content with current values
void inline updateLCDContent();

// print bit values of a byte at given position
void inline printBits(int col, int row, byte b);

// print signed integer with max 4 digits at given position, right aligned, blank padded
void inline printf5d(int col, int row, int d);

// print signed integer with max 8 digits at given position, right aligned, blank padded
void inline printf9d(int col, int row, int d);

// print unsigned long with max 3 digits at given position, right aligned, blank padded
void inline printf3ul(int col, int row, unsigned long d);


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
 * stepper
 **************/
const uint8_t STEPPER_PIN_STEP  = 42;
const uint8_t STEPPER_PIN_DIR   = 40; 
//AccelStepper::DIRECTION_CCW
//AccelStepper::DIRECTION_CW

AccelStepper stepper(AccelStepper::DRIVER, STEPPER_PIN_STEP, STEPPER_PIN_DIR);

float  rpmMax      = 150;//desired max rotations per minute
float  autoRpmMax  = 1.0;  //desired max rotations per minute in auto mode
/*
unsigned float  rotationAnglePerStep = 1.8; //degree per step
 */
unsigned int  stepsPerRevolution  = 200;//stepper full steps per one rotation: motor resolution
unsigned int  subSteps      = 4;        //partial steps per full step: stepper driver resolution
unsigned int  autoSubSteps  = subSteps; //partial steps per full step in auto mode: stepper driver resolution

float   rps           = ((float)rpmMax)/((float)60); //rotations per second
float   autoRps       = ((float)autoRpmMax)/((float)60); //rotations per second in auto mode

float   zSpeedMax     = rps * (float)(stepsPerRevolution * subSteps); //steps per second
float   zAcceleration = zSpeedMax; //steps per second per second, one second to max speed

float   autoSpeedMax     = autoRps * (float)(stepsPerRevolution * autoSubSteps); //steps per second in auto mode
float   autoAcceleration = autoSpeedMax * 10; //00; //steps per second per second, one second to max speed in auto mode

//const long POS_MAX_UP   = -2147483648L;
//const long POS_MAX_DOWN = 2147483647L;
const long POS_MAX_UP   = -10000000L;
const long POS_MAX_DOWN =  10000000L;

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

const int V_ADC_PIN     = A0;

const char LIGHTNING     = 1;
const char ARROW_UP      = 2;
const char ARROW_DOWN    = 3;
const char BETWEEN_MARKS = 4;
const char ARROW_RIGHT   = B01111110; //0176
const char ARROW_LEFT    = B01111111; //0177

uint8_t ARROW_DOWN_PATTERN[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};

uint8_t ARROW_UP_PATTERN[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

uint8_t LIGHTNING_PATTERN[8] = {
  0b00010,
  0b00100,
  0b01000,
  0b00100,
  0b00010,
  0b10100,
  0b11000,
  0b11100
};

uint8_t BETWEEN_MARKS_PATTERN[8] = {
  0b01010,
  0b00100,
  0b00000,
  0b11111,
  0b11111,
  0b00000,
  0b00100,
  0b01010
};

//constant strings
//mode labels for displaying
const String label_INIT     = "INIT...";
const String label_RESERVE  = "RESERVE";
const String label_DOWN     = "UP     ";  
const String label_UP       = "DOWN   ";
const String label_AUTO     = "AUTO   ";
const String label_STOP     = "STOP   ";
const String label_ZERO     = "ZERO   ";
const String label_VMAX     = "VMAX   ";
const String label_VMIN     = "VMIN   ";
const String label_FLUSH    = "FLUSH  ";

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

const String label_INFO    = "MACZ(c) EDM  v.00004";

int mode    = mode_INIT;

const int V_MIN = 0     ;//absolut minimal value for v
const int V_MAX = 1023  ;//absolut maximal value for v
int       vMin  = 400;//V_MAX/3    ;//current lower limit for v 
int       vMax  = 600;//V_MAX-vMin ;//current upper limit for v
int       v     = 0     ;//current v value

const int SPEED_MIN = 0     ;//absolut minimum value  for speed
const int SPEED_MAX = 127   ;//absolut maximum value  for speed
int       zSpeed    = 0     ;//current speed value
boolean   zUp       = true  ;//current direction

const int FLUSH_MIN = 0     ;//absolut minimal value for flush
const int FLUSH_MAX = 1023  ;//absolut maximal value for flush
int       zFlush    = (FLUSH_MAX - FLUSH_MIN)/20    ;//current flush value

int       pos = 0;

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

  //create special characters
  lcd.createChar(LIGHTNING      , LIGHTNING_PATTERN     );
  lcd.createChar(ARROW_UP       , ARROW_UP_PATTERN      );
  lcd.createChar(ARROW_DOWN     , ARROW_DOWN_PATTERN    );
  lcd.createChar(BETWEEN_MARKS  , BETWEEN_MARKS_PATTERN );

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
  
  //Serial.begin(9600);
  //Serial.begin(57600);
  Serial.begin(115200);
  Serial.print("zSpeedMax    : ");Serial.println(zSpeedMax);
  Serial.print("zAcceleration: ");Serial.println(zAcceleration);
  Serial.print("autoSpeedMax    : ");Serial.println(autoSpeedMax);
  Serial.print("autoAcceleration: ");Serial.println(autoAcceleration);

  Serial.print(mode);

  v = ((vMax + 1) > V_MAX ? V_MAX : (vMax + 1)); //vMin + ((vMax - vMin) / 2);
  timer_init_ISR_1KHz(TIMER_DEFAULT);
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

word    modeSwitches    = 0;
word    modeSwitchesOld = 0;

int     oldMode   = mode;
int     oldZSpeed = zSpeed;
boolean oldZUp    = zUp;       
int     oldZFlush = zFlush;
int     oldPos    = pos;

unsigned long lastLoopStart = micros();
unsigned long elapsed = 0;
unsigned long loopTime = 0;
unsigned int  loops = 0;

//#define isReadUFaked

#ifdef isReadUFaked
#define readU(x) readUFaked(x)
#else
#define readU(x) analogRead(x)
#endif

int fakedU = ((vMax + 1) > V_MAX ? V_MAX : (vMax + 1)); //vMin + ((vMax - vMin) / 2);

/*******************************************************/
void loop() {
/*******************************************************/

  //set to true if something change since last loop iteration
  boolean changed = false;

  //save old state
  oldMode = mode;
  modeSwitchesOld = modeSwitches;
  encOldPosition = encPosition;
  oldZSpeed  = zSpeed;
  oldZUp    = zUp;
  oldZFlush = zFlush;
  oldPos    = pos;
  
  //check switch position and get new mode
  byte swLo = ~PINA;
  byte swHi = ((~PINC)&(1<<SWBIT_STOP))>>SWBIT_STOP;
  
  modeSwitches = word(swHi,swLo);
   
  if( modeSwitches != modeSwitchesOld ) {
    changed = true;
    if( modeSwitches == 0 )
       ;//do nothing for now
    else {
      for( int i = 8; i >= 0; i-- ){
        if( modeSwitches & (1<<i) ) {
          if( (modeSwitches & ~(1<<i)) == 0 ){
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
    changed = true;
  }

  v = readU(V_ADC_PIN);
  
  pos = stepper.currentPosition();
  
  if( mode != oldMode ) {
    switch(oldMode) {
      case mode_UP   : modeManualLeave(true)  ;break;
      case mode_DOWN : modeManualLeave(false) ;break;
      case mode_AUTO : modeAutoLeave()        ;break;
    }
    switch(mode) {
      case mode_VMIN : modeVMinEnter()        ;break;
      case mode_VMAX : modeVMaxEnter()        ;break;
      case mode_UP   : modeManualEnter(true)  ;break;
      case mode_DOWN : modeManualEnter(false) ;break;
      case mode_AUTO : modeAutoEnter()        ;break;
      case mode_FLUSH: modeFlushEnter()       ;break;
    }
  }

  switch(mode) {
    case mode_VMIN : modeVMinExec()   ;break;
    case mode_VMAX : modeVMaxExec()   ;break;
    case mode_UP   :
    case mode_DOWN : modeManualExec() ;break;
    case mode_AUTO : modeAutoExec()   ;break;
    case mode_FLUSH: modeFlushExec()  ;break;
    case mode_ZERO : modeZeroExec()   ;break;
    case mode_STOP : modeStopExec()   ;break;
  }
  
  //print current state to serial if changed 
  if( changed ) {
    serialPrintState();
  }
  
  //update lcd display content if necessary
  updateLCDContent();

  //calcAvgLoopTime();
}

void timer_handle_interrupts(int timer) {
  stepper.run();
}

/*
 * mode/state functions
 */

void inline modeStopExec(){
  stepper.setSpeed(0l);
  stepper.setMaxSpeed(0L);
  stepper.stop();
}

void inline modeVMinEnter(){
  encPosition = vMin;
  enc.write(encPosition);
}
void inline modeVMinExec(){
  if( encPosition != encOldPosition ){
    if( encPosition < V_MIN ) {
      encPosition = V_MIN;
      enc.write(encPosition);
    }
    else if( encPosition > V_MAX ){
      encPosition = V_MAX;
      enc.write(encPosition);
    }
    if( encPosition != encOldPosition ){
      vMin = encPosition;
      if( vMin > vMax )
        vMax = vMin;
    }
  }
}
void inline modeVMaxEnter(){
  encPosition = vMax;
  enc.write(encPosition);
}
void inline modeVMaxExec(){
  if( encPosition != encOldPosition ){
    if( encPosition < V_MIN ) {
      encPosition = V_MIN;
      enc.write(encPosition);
    }
    else if( encPosition > V_MAX ){
      encPosition = V_MAX;
      enc.write(encPosition);
    }
    if( encPosition != encOldPosition ){
      vMax = encPosition;
      if( vMax < vMin)
        vMin = vMax;
    }
  }
}

void inline modeManualEnter(boolean dirUp){
  zUp = dirUp;
  zSpeed = SPEED_MIN;
  //zSpeed = SPEED_MAX; //SPEED_MIN;
  encPosition = zSpeed;
  enc.write(encPosition);

  stepper.setMaxSpeed(0L);
  //stepper.setMaxSpeed(zSpeedMax /*0L*/);
  stepper.setAcceleration(zAcceleration);
  
  if( dirUp )
    stepper.moveTo(POS_MAX_UP);
  else    
    stepper.moveTo(POS_MAX_DOWN);
}
void inline modeManualExec(){
  if( encPosition != encOldPosition ){
    if( encPosition < SPEED_MIN ) {
      encPosition = SPEED_MIN;
      enc.write(encPosition);
    }
    else if( encPosition > SPEED_MAX ){
      encPosition = SPEED_MAX;
      enc.write(encPosition);
    }
    if( encPosition != encOldPosition ){
      zSpeed = encPosition;
      float zz = zSpeedMax * (((float)(encPosition - SPEED_MIN))/((float)SPEED_MAX));
      stepper.setMaxSpeed(zz);
    }
  }
  //stepper.run();
}
void inline modeManualLeave(boolean dirUp){
  (void)(dirUp);//unused
  stepper.setSpeed(0l);
  stepper.setMaxSpeed(0L);
  stepper.stop();
}

// stepper move values
const long ONE_STEP_UP   = 1L;
const long ONE_STEP_DOWN =-1L;

long workingPos = zFlush;
long nextFlush  = zFlush;

const int WORKING            = 0;

const int WORKING_GOING_DOWN =-1;
const int WORKING_WAITING    = 0;
const int WORKING_GOING_UP   = 1;

const int FLUSHING_GOING_UP  = 1;
const int FLUSHING_WAITING   = 2;
const int FLUSHING_RETURNING = 3;

int autoMode = WORKING;

int autoModeWorking = WORKING_WAITING;


void inline modeAutoWorkingExec();
void inline modeAutoFlushingGoingUpExec();
void inline modeAutoFlushingWaitingExec();
void inline modeAutoFlushingReturningExec();

void inline modeAutoEnter(){
  Serial.println("modeAutoEnter");

#ifdef isReadUFaked 
  fakedU = ((vMax + 1) > V_MAX ? V_MAX : (vMax + 1)); //vMin + ((vMax - vMin) / 2);
  enc.write(fakedU);
#endif

  autoMode = WORKING;
  workingPos = stepper.currentPosition();
  stepper.setMaxSpeed(autoSpeedMax);
  stepper.setAcceleration(autoAcceleration);
  stepper.moveTo(workingPos);
  nextFlush  = workingPos - zFlush;
}

void inline modeAutoExec(){
  switch(autoMode) {
    case WORKING            : modeAutoWorkingExec()           ; break;
    case FLUSHING_GOING_UP  : modeAutoFlushingGoingUpExec()   ; break;
    case FLUSHING_WAITING   : modeAutoFlushingWaitingExec()   ; break;
    case FLUSHING_RETURNING : modeAutoFlushingReturningExec() ; break;
  }
}
void inline modeAutoLeave(){
  Serial.println("modeAutoLeave"); 
  stepper.setSpeed(0l);
  stepper.setMaxSpeed(0L);
  stepper.stop();
}

void inline modeAutoWorkingExec(){
  //Serial.println("modeAutoWorkingExec"); 
  workingPos = stepper.currentPosition();
  boolean isRunning = stepper.isRunning();
  long tp = stepper.targetPosition();
  long d2g = stepper.distanceToGo();
  float s = stepper.speed();
  
  //Serial.print("autoWorking | workingPos | nextFlush | v | vMin | vMax | isRunning | v > vMax | v < vMin :  "); 
//  Serial.print("autoWorking | workingPos | tp | d2g | speed | isRunning | nextFlush  | v > vMax | v < vMin :  "); 
//  Serial.print(workingPos); 
//  Serial.print(" | "); 
//  Serial.print(tp);
//  Serial.print(" | "); 
//  Serial.print(d2g);
//  Serial.print(" | "); 
//  Serial.print(s);
//  Serial.print(" | "); 
//  Serial.print(isRunning);
//  Serial.print(" | "); 
//  Serial.print(nextFlush);
////  Serial.print(" | "); 
////  Serial.print(v);
////  Serial.print(" | "); 
////  Serial.print(vMin);
////  Serial.print(" | "); 
////  Serial.print(vMax);
//  Serial.print(" | "); 
//  Serial.print((v > vMax));
//  Serial.print(" | "); 
//  Serial.print((v < vMin));
//  Serial.println();
   
  if( workingPos <= nextFlush ){
    //enter AutoFlush sub mode
    autoMode = FLUSHING_GOING_UP;
    
    stepper.setMaxSpeed(zSpeedMax);
    stepper.setAcceleration(zAcceleration);
    stepper.moveTo((workingPos + zFlush) > 0 ? (workingPos + zFlush) : 0);//move at least zFlash steps back
    return;    
  }
  if( isRunning /*stepper.isRunning()*/ )
    return; //wait in this state till stepper performed last one step movement and stopped
    
  if( v > vMax ){
    autoModeWorking = WORKING_GOING_DOWN;
    stepper.move(ONE_STEP_DOWN);  
  }
  else if( v < vMin ){
    autoModeWorking = WORKING_GOING_UP;
    stepper.move(ONE_STEP_UP);
    nextFlush++; //pull up nextFlush position with us, 
                 //so we always flush after zFlush downward steps
  }
  else
    autoModeWorking = WORKING_WAITING;

}
void inline modeAutoFlushingGoingUpExec(){
  boolean isRunning = stepper.isRunning();
  Serial.print("modeAutoFlushingGoingUpExec: isRunning: "); 
  Serial.print(isRunning); 
  Serial.println(); 
  //if( stepper.isRunning() )
  if( isRunning )
    return; //wait in this state till stepper preformed movement to zero and stopped
  //enter returning sub mode
  autoMode = FLUSHING_WAITING;
}
void inline modeAutoFlushingWaitingExec(){
  Serial.println("modeAutoFlushingWaitingExec"); 
  //here we would wait for flushing to end
  //enter returning sub mode
  autoMode = FLUSHING_RETURNING;
  stepper.moveTo(workingPos);  //move back to last position in auto working mode  
}
void inline modeAutoFlushingReturningExec(){
  boolean isRunning = stepper.isRunning();
  Serial.print("modeAutoFlushingReturningExec: isRunning: "); 
  Serial.print(isRunning); 
  Serial.println(); 
  //if( stepper.isRunning() )
  if( isRunning )
    return; //wait in this state till stepper performed movement to last position and stopped
  //enter working sub mode
  autoMode = WORKING;
  stepper.setMaxSpeed(autoSpeedMax);
  stepper.setAcceleration(autoAcceleration);
  nextFlush  = stepper.currentPosition() - zFlush;
}

void inline modeFlushEnter(){
  encPosition = zFlush;
  enc.write(encPosition);
}
void inline modeFlushExec(){
  if( encPosition != encOldPosition ){
    if( encPosition < FLUSH_MIN ) {
      encPosition = FLUSH_MIN;
      enc.write(encPosition);
    }
    else if( encPosition > FLUSH_MAX ){
      encPosition = FLUSH_MAX;
      enc.write(encPosition);
    }
    if( encPosition != encOldPosition ){
      zFlush = encPosition;
    }
  }
}
void inline modeZeroExec(){
  pos = 0;
  stepper.setCurrentPosition(0L);
}


/*
 * helper functions
 */

void inline calcAvgLoopTime(){
  //calc average loop time, very stupid implementation really
  //delayMicroseconds(200); //just for testing avg loop time calculations
  elapsed += (micros() - lastLoopStart);
  //loopTime = (micros() - lastLoopStart);
  loops++;
  if( loops >= 256 ){
    loopTime = (elapsed/256);
    loops = 0;
    elapsed = 0;    
  }
  lastLoopStart = micros();  
}

// print current state to serial
void inline serialPrintState() {
    Serial.print(mode);
    Serial.print("\t");
    Serial.print(modeSwitches);
    Serial.print("\t");
    Serial.print(encPosition);
    Serial.println("");  
}

// update LCD content with current values
void inline updateLCDContent(){
  //update lcd display content if necessary
  display_elapsed = millis() - display_last_update;
  
  if( display_elapsed > displayUpdatePeriod ) {
    //update mode
    lcd.setCursor(0,1);
    lcd.print(modeLabel[mode]);

    if( mode == mode_AUTO ){
      char c1 = ' ';
      char c2 = ' ';
      switch(autoMode){
        case WORKING            : c1=LIGHTNING;
                                  switch(autoModeWorking) {
                                    case(WORKING_GOING_UP)  : c2=ARROW_UP     ; break;
                                    case(WORKING_GOING_DOWN): c2=ARROW_DOWN   ; break;
                                    default                 : c2=BETWEEN_MARKS; break;
                                  }
                                  break;
        case FLUSHING_GOING_UP  : c1='F'; c2=ARROW_UP    ; break;
        case FLUSHING_WAITING   : c1='F';                ; break;
        case FLUSHING_RETURNING : c1='F'; c2=ARROW_DOWN  ; break;
      }
      lcd.setCursor(5,1);
      lcd.write(c1);
      lcd.write(c2);
    }
    //update modeSwitch state
    printBits(0,2, modeSwitches);

    //update vMin value
    printf5d(15,1,vMin);

    //update v value
    printf5d(15,2,v);
    
    //update vMax value
    printf5d(15,3,vMax);

    //update zSpeed value
    printf5d(10,1,zSpeed);

    //update zFlush value
    printf5d(10,2,zFlush);

    //update pos value
//    printf5d(10,3,pos);
    printf9d(6,3,pos);

    //update encoder position value
    printf5d(0,3,encPosition);

//    //update loop period value
//    printf3ul(6,3,loopTime);

    //restart update period
    display_last_update = millis();

//    switch(mode) {
//      case mode_VMIN:  lcd.setCursor(15,1); lcd.write(RIGHT_ARROW);lcd.setCursor(15,1); lcd.blink(); break; 
//      default: lcd.noBlink();
//    }
  }  
}

// print bit values of a byte at given position
void inline printBits(int col, int row, byte b){
    lcd.setCursor(col,row);
    for( int i = 7; i >= 0; i-- ){
      if( ((1<<i) & b) != 0 )
        lcd.print(1);
      else
        lcd.print(0);
    }
}

// print signed integer with max 4 digits at given position, right aligned, blank padded
void inline printf5d(int col, int row, int d){
    lcd.setCursor(col,row);
    if( d >= 0 ){
      lcd.print(" ");
      if( d < 1000 )
        lcd.print(" ");
      if( d < 100 )
        lcd.print(" ");
      if( d < 10 )
        lcd.print(" ");
    }
    else {
      if( d > -1000 )
        lcd.print(" ");
      if( d > -100 )
        lcd.print(" ");
      if( d > -10 )
        lcd.print(" ");
    }
    
    lcd.print(d);
}

// print signed integer with max 8 digits at given position, right aligned, blank padded
void inline printf9d(int col, int row, int d){
    lcd.setCursor(col,row);
    if( d >= 0 ){
      lcd.print(" ");
      if( d < 10000000 )
        lcd.print(" ");
      if( d < 1000000 )
        lcd.print(" ");
      if( d < 100000 )
        lcd.print(" ");
      if( d < 10000 )
        lcd.print(" ");
      if( d < 1000 )
        lcd.print(" ");
      if( d < 100 )
        lcd.print(" ");
      if( d < 10 )
        lcd.print(" ");
    }
    else {
      if( d > -10000000 )
        lcd.print(" ");
      if( d > -1000000 )
        lcd.print(" ");
      if( d > -100000 )
        lcd.print(" ");
      if( d > -10000 )
        lcd.print(" ");
      if( d > -1000 )
        lcd.print(" ");
      if( d > -100 )
        lcd.print(" ");
      if( d > -10 )
        lcd.print(" ");
    }
    
    lcd.print(d);
}

// print unsigned long with max 3 digits at given position, right aligned, blank padded
void inline printf3ul(int col, int row, unsigned long d){
    lcd.setCursor(col,row);

      if( d < 100 )
        lcd.print(" ");
      if( d < 10 )
        lcd.print(" ");
    
    lcd.print(d);
}

 
int readUFaked(int pin){
  (void)(pin);//unused 
  delayMicroseconds(150);
  //delay(250);
  if( (mode != mode_AUTO) || (autoMode != WORKING) ){
    return fakedU;
  }
  if( encPosition < V_MIN ) {
      encPosition = V_MIN;
      enc.write(encPosition);
  }
  else if( encPosition > V_MAX ){
      encPosition = V_MAX;
      enc.write(encPosition);
  }
  fakedU = encPosition;
  return fakedU;
}
