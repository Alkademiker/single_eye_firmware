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

#define RAND_INTERVAL 2000  // Interval that defines the gap between two random movements

MechatronicEye eye = MechatronicEye(VERT_SERVO, HORZ_SERVO);
EyeServoCommand command;

void debug_filter_serial_plotter();

void calibration_routine();

void setServoPulse(uint8_t servo, double pulse);

enum MODES {
  DUAL_MANUAL,
  SINGLE_MANUAL,
  RANDOM_WALK
};

enum STATES {
  INIT,
  NORMAL,
  CONTROLLER_TIMED_OUT,  // --> always go into random walk
  CALIBRATION
};

enum CALIBRATION_STATES {
  START_CENTERED,
  HORIZONTAL,
  VERTICAL,
  DONE
};

MODES current_mode = RANDOM_WALK;
STATES current_state = INIT;
CALIBRATION_STATES calibration_state = START_CENTERED;

unsigned long timer_value_random_walk = 0;

void setup() {

  Serial.begin(57600);

  delay(4000); // added delay to give wireless ps2 module some time to startup, before configuring it

  pwm.begin(); // Powering up the servo board
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(1000);

  // Init Eye
  eye.setLeftLimit(345);
  eye.setHorizontalCenter(297);
  eye.setRightLimit(255);

  eye.setTopLimit(250);
  eye.setVerticalCenter(320);
  eye.setBottomLimit(390);

  // Init Alpha Filters
  _vert_filt.setAlpha(0.055);
  _horz_filt.setAlpha(0.055); 

  delay(1000);

  
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

  type = ps2x.readType();
  
  error = 0;
  type = 3;

  if (ps2x.ButtonPressed(PSB_SELECT)){
    current_state = CALIBRATION;
  }
  else
  {
    current_state = NORMAL;
  }
}

void loop() {

  // TODO: check if controller timed out
  // if yes go into normal state and random walk

  switch (current_state)
  {
  case CALIBRATION:
    calibration_routine();
    break;
  default:

    ps2x.read_gamepad(false, vibrate); // read controller and set large motor to spin at 'vibrate' speed

    switch (current_mode)
    {
    case SINGLE_MANUAL:
    case DUAL_MANUAL:
      horizontal_read = map(_horz_filt.update(ps2x.Analog(PSS_RX)),
                            0, 255,
                            0, 10000);
      vertical_read = map(_vert_filt.update(ps2x.Analog(PSS_RY)),
                          0, 255,
                          0, 10000);

      command = eye.lookXY(horizontal_read, vertical_read);

      pwm.setPWM(command.horizontal_index, 0, command.horizontal_value);
      pwm.setPWM(command.vertical_index, 0, command.vertical_value);
      break;
    
    case RANDOM_WALK:
      timer_value_random_walk += 1;
      if (timer_value_random_walk >= RAND_INTERVAL)
      {
        Serial.println("New position");
        command = eye.randomWalk();
        timer_value_random_walk = 0;
      }  

      pwm.setPWM(command.horizontal_index, 0, command.horizontal_value);
      pwm.setPWM(command.vertical_index, 0, command.vertical_value);

    default:
      break;
    }

    // Mode switch: Keep R2 pressed and switch to new mode with triangle
    if (ps2x.NewButtonState())
    {
      if (ps2x.Button(PSB_R2))
      {
        if (ps2x.ButtonReleased(PSB_TRIANGLE))
        {
          Serial.print("New Mode: ");
          int temp = (int)current_mode;
          temp += 1;
          if (temp > 2)
          {
            temp = 0;
          }
          current_mode = (MODES)temp;
          Serial.println(temp);
        }
      }
    }
      break;
  }

  delay(1);
    
    
}

void calibration_routine()
{
  bool x_was_pressed = ps2x.ButtonReleased(PSB_CROSS);
  int analog = 127;
  double calibration_value = 1500;

  switch (calibration_state)
  {
  case START_CENTERED:
    setServoPulse(VERT_SERVO, 1500);
    setServoPulse(HORZ_SERVO, 1500);
    
    if (x_was_pressed)
    {
      calibration_state = HORIZONTAL;
      delay(100);
    }
    break;
  case HORIZONTAL:
    while (! x_was_pressed)
    {
      ps2x.read_gamepad();
      analog = ps2x.Analog(PSS_RX);
      calibration_value = map(analog,
                              0, 255,
                              800, 2200);

      Serial.print("Center Horizontal ");
      setServoPulse(HORZ_SERVO, calibration_value);  
      delay(30);
      x_was_pressed = ps2x.ButtonReleased(PSB_CROSS);
    }
    
    calibration_state = VERTICAL;
    
    break;
  case VERTICAL:
    while (!x_was_pressed)
    {
      ps2x.read_gamepad();
      analog = ps2x.Analog(PSS_RY);
      calibration_value = map(analog,
                              0, 255,
                              800, 2200);

      Serial.print("Center Horizontal ");
      setServoPulse(VERT_SERVO, calibration_value);
      pwm.setPWM(HORZ_SERVO, 0, 297);

      delay(30);
      x_was_pressed = ps2x.ButtonReleased(PSB_CROSS);
    }
    calibration_state = DONE;
    break;

  case DONE:
  default:

    Serial.println("We have reached end.");
    while (true)
    {
      /* code */
    }
    
    break;
  }
  Serial.print("Current State");
  Serial.println(calibration_state);

  delay(100);

}

void debug_filter_serial_plotter()
{
  Serial.print("Horz_raw:");
  Serial.print(horizontal_read);
  Serial.print(",");
  Serial.print("Horz_filt:");
  Serial.print(_horz_filt.getState());
  Serial.print(",");
  Serial.print("Vert_raw:");
  Serial.print(vertical_read);
  Serial.print(",");
  Serial.print("Vert_filt:");
  Serial.println(_vert_filt.getState());
}

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t servo, double pulse)
{
  double pulselength;

  pulselength = 1000000; // 1,000,000 us per second
  pulselength /= SERVO_FREQ; // in Hz
  //Serial.print(pulselength);
  //Serial.println(" us per period");
  pulselength /= 4096; // 12 bits of resolution
  //Serial.print(pulselength);
  //Serial.println(" us per bit");
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(servo, 0, pulse);
}