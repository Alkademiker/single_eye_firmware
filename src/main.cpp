#include <Arduino.h>

#include <PS2X_lib.h> //for v1.6
#include <alpha_filter.hpp>

#include <mechatronic_eye.hpp>  // For eye handling

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

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

AlphaFilter<int> _vert_filt;
AlphaFilter<int> _horz_filt;
//-----------------------------

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define VERT_SERVO 0
#define HORZ_SERVO 1

MechatronicEye eye = MechatronicEye(VERT_SERVO, HORZ_SERVO);
EyeServoCommand command;



void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(57600);

  delay(1000); // added delay to give wireless ps2 module some time to startup, before configuring it

  pwm.begin(); // Powering up the servo board
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(1000);
  // Serial.println("Ready to Look!\r");

  // Init Eye
  eye.setLeftLimit(335);
  eye.setHorizontalCenter(375);
  eye.setRightLimit(415);

  eye.setTopLimit(330);
  eye.setVerticalCenter(370);
  eye.setBottomLimit(410);

  // Init Alpha Filters
  _vert_filt.setAlpha(0.055);
  _horz_filt.setAlpha(0.055); 

  delay(1000);

  
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

  type = ps2x.readType();
  
  error = 0;
  type = 3;

}

void loop() {

                              // DualShock Controller
    ps2x.read_gamepad(false, vibrate); // read controller and set large motor to spin at 'vibrate' speed

    horizontal_read = ps2x.Analog(PSS_RX);
    vertical_read = ps2x.Analog(PSS_RY);

    _horz_filt.update(horizontal_read);
    _vert_filt.update(vertical_read);

    command = eye.lookXY(horizontal_read, vertical_read);

    Serial.print("Horz_raw:");
    Serial.print(horizontal_read);
    Serial.print(",");
    Serial.print("Horz_filt:");
    Serial.print(_horz_filt.update(horizontal_read));
    Serial.print(",");
    Serial.print("Vert_raw:");
    Serial.print(vertical_read);
    Serial.print(",");
    Serial.print("Vert_filt:");
    Serial.println(_vert_filt.update(vertical_read));

    pwm.setPWM(command.horizontal_index, 0, command.horizontal_value);
    pwm.setPWM(command.vertical_index, 0, command.vertical_value);
    
    
}