#include <Arduino.h>

#include <PS2X_lib.h> //for v1.6

#include <mechatronic_eye.hpp>

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT 13 // 14
#define PS2_CMD 11 // 15
#define PS2_SEL 10 // 16
#define PS2_CLK 12 // 17

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures false
//#define rumble      true
#define rumble false

PS2X ps2x; // create PS2 Controller Class

int error = 0;
byte type = 0;
byte vibrate = 0;

int vertical_read = 127;
int horizontal_read = 127;

int vert_out = 0;
int horz_out = 0;

//-----------------------------
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define VERT_SERVO 0
#define HORZ_SERVO 1

// *********** Here are some things you will need to tweak ************
/*  Depending on your servo and linkage set, as well as at what position
    you attach the arms, you will need to tweak the range of each servo.
      1) Make sure you connect each servo to power before you attach any
         arms/linkage. This will make sure they are all centered.
      2) Put on the arms, run the code and make sure nothing is going to
         collide.
      3) From there, attach your linkages and fine-tune your ranges. The
         eyeball should be centered at the end of each movement.
      4) Tweak both your linkages and your code to achieve best results.
*/
#define xMid 375 // X axis centerpoint (mine was near 330)
                 // lower number => eye looks left farther
                 // NOTE: eye should end roughly centered

#define xRange 80 // X axis range of motion (start small ~50)
                  // lower number => eye moves less L and R
                  // NOTE: eye should end roughly centered

#define yMid 370 // Y axis centerpoint (mine was near 330)
                 // higher number => eye looks right farther
                 // NOTE: eye should end roughly centered

#define yRange 80 // Y axis range of motion (start small ~50)
                  // lower number => eye moves less up and down
                  // NOTE: eye should end roughly centered

int spd = 3; // Speed can range from 0(fast) => 15(slow)

int gap = 1000; // Delay between movements in milliseconds

uint8_t xEye = HORZ_SERVO; // This is the servo board pins for the eye X axis

uint8_t yEye = VERT_SERVO; // This is the servo board pins for the eye Y axis

const int lookButtonPin = 3; // This is the Arduino digital pin for 'look' button

/********************** Probably don't change these *******************/
#define rightLimit xMid - (xRange / 2) // This represents the X axis right limit
#define leftLimit xMid + (xRange / 2)  // This represents the X axis left limit
#define upLimit yMid - (yRange / 2)    // This represents the Y axis up limit
#define downLimit yMid + (yRange / 2)  // This represents the Y axis down limit
#define USMIN 800
#define USMAX 2200

void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(57600);

  delay(1000); // added delay to give wireless ps2 module some time to startup, before configuring it

  pwm.begin(); // Powering up the servo board
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(1000);
  Serial.println("Ready to Look!\r");

  delay(1000);

  // CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  Serial.println("Starting Program...");
  // setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

  if (error == 0)
  {
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
    if (pressures)
      Serial.println("true ");
    else
      Serial.println("false");
    Serial.print("rumble = ");
    if (rumble)
      Serial.println("true)");
    else
      Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }
  else if (error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");

  else if (error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if (error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");

  else
  {
    Serial.print("Error is something else: ");
    Serial.println(error);
  }
    

  //  Serial.print(ps2x.Analog(1), HEX);

  type = ps2x.readType();
  switch (type)
  {
  case 0:
    Serial.print("Unknown Controller type found ");
    break;
  case 1:
    Serial.print("DualShock Controller found ");
    break;
  case 2:
    Serial.print("GuitarHero Controller found ");
    break;
  case 3:
    Serial.print("Wireless Sony DualShock Controller found ");
    break;
  }

  error=0;
  type = 1;

  Serial.println("Setup exiting");

}

void loop() {

  /* You must Read Gamepad to get new values and set vibration values
    ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
    if you don't enable the rumble, use ps2x.read_gamepad(); with no values
    You should call this at least once a second
  */
  if (error == 1) // skip loop if no controller found
    return;

  if (type == 2)
  {                      // Guitar Hero Controller
    ps2x.read_gamepad(); // read controller

    if (ps2x.ButtonPressed(GREEN_FRET))
      Serial.println("Green Fret Pressed");
    if (ps2x.ButtonPressed(RED_FRET))
      Serial.println("Red Fret Pressed");
    if (ps2x.ButtonPressed(YELLOW_FRET))
      Serial.println("Yellow Fret Pressed");
    if (ps2x.ButtonPressed(BLUE_FRET))
      Serial.println("Blue Fret Pressed");
    if (ps2x.ButtonPressed(ORANGE_FRET))
      Serial.println("Orange Fret Pressed");

    if (ps2x.ButtonPressed(STAR_POWER))
      Serial.println("Star Power Command");

    if (ps2x.Button(UP_STRUM)) // will be TRUE as long as button is pressed
      Serial.println("Up Strum");
    if (ps2x.Button(DOWN_STRUM))
      Serial.println("DOWN Strum");

    if (ps2x.Button(PSB_START)) // will be TRUE as long as button is pressed
      Serial.println("Start is being held");
    if (ps2x.Button(PSB_SELECT))
      Serial.println("Select is being held");

    if (ps2x.Button(ORANGE_FRET))
    { // print stick value IF TRUE
      Serial.print("Wammy Bar Position:");
      Serial.println(ps2x.Analog(WHAMMY_BAR), DEC);
    }
  }
  else
  {                                    // DualShock Controller
    ps2x.read_gamepad(false, vibrate); // read controller and set large motor to spin at 'vibrate' speed

    horizontal_read = ps2x.Analog(PSS_RX);
    vertical_read = ps2x.Analog(PSS_RY);

    horz_out = map(horizontal_read, 0, 255, leftLimit, rightLimit);
    vert_out = map(vertical_read, 0, 255, downLimit, upLimit);
  
    // if (vert_out !=331)
    // {
    //   Serial.println(vert_out);
    // }
    
    
    pwm.setPWM(0, 0, vert_out);
    pwm.setPWM(1, 0, horz_out);


    // // up and down
    // for (uint16_t pulseLen = xMid; pulseLen > rightLimit; pulseLen--)
    // {
    //   pwm.setPWM(xEye, 0, pulseLen);
    //   delay(spd);
    // }
  }
}