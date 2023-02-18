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

// Left Eye
int left_vertical_read = 127;
int left_horizontal_read = 127;

AlphaFilter<int> _left_vert_filt;
AlphaFilter<int> _left_horz_filt;

// Right Eye
int right_vertical_read = 127;
int right_horizontal_read = 127;

AlphaFilter<int> _right_vert_filt;
AlphaFilter<int> _right_horz_filt;
//-----------------------------

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define LEFT_VERT_SERVO 0
#define LEFT_HORZ_SERVO 1

#define RIGHT_VERT_SERVO 2
#define RIGHT_HORZ_SERVO 3


MechatronicEye left_eye = MechatronicEye(LEFT_VERT_SERVO, LEFT_HORZ_SERVO);
EyeServoCommand left_command;

MechatronicEye right_eye = MechatronicEye(RIGHT_VERT_SERVO, RIGHT_HORZ_SERVO);
EyeServoCommand right_command;

void debug_filter_serial_plotter();

void calibration_routine();

void setServoPulse(uint8_t servo, double pulse);

enum MODES
{
  DUAL_MANUAL,
  SINGLE_MANUAL,
  RANDOM_WALK_TOGETHER,
  RANDOM_WALK,
  PLAY_DEAD, // Turn the eyes around,
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

unsigned long timer_value = 0;
#define RAND_INTERVAL 2000 // Interval that defines the gap between two random movements

#define DEAD_INTERVAL 400 // Interval that defines the gap between two random movements
int play_dead_index = 0;

void setup() {

  Serial.begin(57600);

  delay(4000); // added delay to give wireless ps2 module some time to startup, before configuring it

  pwm.begin(); // Powering up the servo board
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(1000);

  // Init Eye
  // left eye
  // left eye  further up further left
  left_eye.setLeftLimit(340);
  left_eye.setHorizontalCenter(290); // 320
  left_eye.setRightLimit(250);

  left_eye.setTopLimit(200);
  left_eye.setVerticalCenter(260); //340
  left_eye.setBottomLimit(340);

  // right eye
  right_eye.setLeftLimit(340);
  right_eye.setHorizontalCenter(290);
  right_eye.setRightLimit(250);

  right_eye.setTopLimit(200);
  right_eye.setVerticalCenter(260);
  right_eye.setBottomLimit(340);

  // Init Alpha Filters
  _left_vert_filt.setAlpha(0.055);
  _left_horz_filt.setAlpha(0.055);
  _right_vert_filt.setAlpha(0.055);
  _right_horz_filt.setAlpha(0.055);

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
      right_horizontal_read = map(_right_horz_filt.update(ps2x.Analog(PSS_RX)),
                                 0, 255,
                                 0, 10000);
      right_vertical_read = map(_right_vert_filt.update(ps2x.Analog(PSS_RY)),
                               0, 255,
                               0, 10000);

      left_command = left_eye.lookXY(right_horizontal_read, right_vertical_read);
      right_command = right_eye.lookXY(right_horizontal_read, right_vertical_read);

      break;

    case DUAL_MANUAL:
      // Both Eyes can be controlled individually

      // Left
      left_horizontal_read = map(_left_horz_filt.update(ps2x.Analog(PSS_LX)),
                            0, 255,
                            0, 10000);
      left_vertical_read = map(_left_vert_filt.update(ps2x.Analog(PSS_LY)),
                          0, 255,
                          0, 10000);

      left_command = left_eye.lookXY(left_horizontal_read, left_vertical_read);


      // Right
      right_horizontal_read = map(_right_horz_filt.update(ps2x.Analog(PSS_RX)),
                                 0, 255,
                                 0, 10000);
      right_vertical_read = map(_right_vert_filt.update(ps2x.Analog(PSS_RY)),
                               0, 255,
                               0, 10000);

      right_command = right_eye.lookXY(right_horizontal_read, right_vertical_read);
      break;
    
    case RANDOM_WALK:
      timer_value += 1;
      if (timer_value >= RAND_INTERVAL)
      {
        Serial.println("New position");
        left_command = left_eye.randomWalk();
        right_command = right_eye.randomWalk();
        timer_value = 0;
      }
      // Update alpha filter
      left_command = left_eye.randomWalk(left_eye.getLastRandVal());
      right_command = right_eye.randomWalk(right_eye.getLastRandVal());

      break;

    case RANDOM_WALK_TOGETHER:
      timer_value += 1;
      if (timer_value >= RAND_INTERVAL)
      {
        Serial.println("New position");
        left_command = left_eye.randomWalk();
        right_command = right_eye.randomWalk(left_eye.getLastRandVal());
        timer_value = 0;
      }
      // Update Alpha filter
      left_command = left_eye.randomWalk(left_eye.getLastRandVal());
      right_command = right_eye.randomWalk(left_eye.getLastRandVal());

      break;

    case PLAY_DEAD:
      timer_value += 1;
      if (timer_value >= DEAD_INTERVAL)
      {
        Serial.println("DEAD: New position");
        left_command = left_eye.deadRoll(play_dead_index); // CW
        right_command = right_eye.deadRoll(-play_dead_index-1); // inverse index --> CCW
        timer_value = 0;
        play_dead_index = (play_dead_index + 1) % 8;
      }
      left_command = left_eye.deadRoll(play_dead_index); // CW
      right_command = right_eye.deadRoll(-play_dead_index - 1);

      break;

    default:
      break;
    }

    // set the pwm values no matter what mode
    pwm.setPWM(left_command.horizontal_index, 0, left_command.horizontal_value_filt.getState());
    pwm.setPWM(left_command.vertical_index, 0, left_command.vertical_value_filt.getState());
    pwm.setPWM(right_command.horizontal_index, 0, right_command.horizontal_value_filt.getState());
    pwm.setPWM(right_command.vertical_index, 0, right_command.vertical_value_filt.getState());

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
          if (temp > 6)
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
    setServoPulse(LEFT_VERT_SERVO, 1500);
    setServoPulse(LEFT_HORZ_SERVO, 1500);
    
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
      setServoPulse(LEFT_HORZ_SERVO, calibration_value);  
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
      setServoPulse(LEFT_VERT_SERVO, calibration_value);
      pwm.setPWM(LEFT_HORZ_SERVO, 0, 297);

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
  Serial.print(left_horizontal_read);
  Serial.print(",");
  Serial.print("Horz_filt:");
  Serial.print(_left_horz_filt.getState());
  Serial.print(",");
  Serial.print("Vert_raw:");
  Serial.print(left_vertical_read);
  Serial.print(",");
  Serial.print("Vert_filt:");
  Serial.println(_left_vert_filt.getState());
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