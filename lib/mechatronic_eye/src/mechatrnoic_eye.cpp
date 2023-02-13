#include "Arduino.h"
#include "mechatronic_eye.hpp"

MechatronicEye::MechatronicEye(int vertical_servo, int horizontal_servo) 
    : _commands(), _limits()
{
    _commands.vertical_index= vertical_servo;  
    _commands.horizontal_index = horizontal_servo; 
}

MechatronicEye::MechatronicEye(int vertical_servo,
                               int horizontal_servo,
                               EyeLimit limits)
    : _commands()
{
    _commands.vertical_index = vertical_servo;
    _commands.horizontal_index = horizontal_servo;
    _limits = limits;
}

MechatronicEye::~MechatronicEye()
{
}

/// @brief Calculates the servo command from an u_int8 input (0-255)
/// @param horizontal 0-1000, where 500 is the center, 0 is left and 1000 is right
/// @param vertical 0-1000, where 500 is the center, 0 is the top and 1000 is the bottom
/// @return EyeServoCommand containing the servo commands
EyeServoCommand MechatronicEye::lookXY(int horizontal, int vertical)
{
    // Serial.print("horz: ");
    // Serial.print(horizontal);
    // Serial.print(", vert: ");
    // Serial.println(vertical);
    this->_commands.horizontal_value = map(horizontal, 0, 10000, 
                _limits._left_limit, _limits._right_limit);

    this->_commands.vertical_value = map(vertical, 0, 10000,
                _limits._top_limit, _limits._bottom_limit);

    // Serial.print("out h: ");
    // Serial.print(this->_commands.horizontal_value);
    // Serial.print(", out v: ");
    // Serial.println(this->_commands.vertical_value);

    return _commands;
}
    
EyeServoCommand MechatronicEye::lookAt(EyePositions position)
{
    EyeServoCommand ret;
    switch (position)
    {
    case CENTER:
        ret = lookCenter();
        break;
    case LEFT:
        ret = lookLeft();
        break;
    case MID_LEFT:
        ret = lookMidLeft();
        break;
    case MID_RIGHT:
        ret = lookMidRight();
        break;
    case RIGHT:
        ret = lookRight();
        break;
    case TOP:
        ret = lookUp();
        break;
    case MID_TOP:
        ret = lookMidUp();
        break;
    case MID_BOTTOM:
        ret = lookMidDown();
        break;
    case BOTTOM:
        ret = lookDown();
        break;
    case MID_TOP_RIGHT:
        ret = lookMidUpRight();
        break;
    case MID_BOTTOM_RIGHT:
        ret = lookMidDownRight();
        break;
    case MID_BOTTOM_LEFT:
        ret = lookMidDownLeft();
        break;
    case MID_TOP_LEFT:
        ret = lookMidUpLeft();
        break;

    case TOP_RIGHT:
    case BOTTOM_RIGHT:
    case BOTTOM_LEFT:
    case TOP_LEFT:
    default:
        Serial.print("This case (");
        Serial.print(position);
        Serial.println(") is not supported");
        break;
    }
    return ret;
}

// Functional Programs

EyeServoCommand MechatronicEye::randomWalk()
{
    int rand = random(13);
    Serial.print("Random number: ");
    Serial.println(rand);
    return lookAt((EyePositions)rand);
}

EyeServoCommand MechatronicEye::deadRoll(int i)
{
    int j = static_cast<int>(i % 8);

    switch (j)
    {
    case 0:
        return this->lookMidUp();
    case 1:
        return this->lookMidUpRight();
    case 2:
        return this->lookMidRight();
    case 3:
        return this->lookMidDownRight();
    case 4:
        return this->lookMidDown();
    case 5:
        return this->lookMidDownLeft();
    case 6:
        return this->lookMidLeft();
    case 7:
        return this->lookMidUpLeft();
    }
}

// Extremes
EyeServoCommand MechatronicEye::lookCenter()
{
    return this->lookXY(5000, 5000);
}
EyeServoCommand MechatronicEye::lookUp()
{
    return this->lookXY(5000, 0);
}
EyeServoCommand MechatronicEye::lookDown()
{
    return this->lookXY(5000, 10000);
}

EyeServoCommand MechatronicEye::lookLeft()
{
    return this->lookXY(0, 5000);
}
EyeServoCommand MechatronicEye::lookRight()
{
    return this->lookXY(10000, 5000);
}

// Mids Vertical and Horizontal
EyeServoCommand MechatronicEye::lookMidUp()
{
    return this->lookXY(5000, 2500);
}
EyeServoCommand MechatronicEye::lookMidDown()
{
    return this->lookXY(5000, 7500);
}
EyeServoCommand MechatronicEye::lookMidLeft()
{
    return this->lookXY(2500, 5000);
}
EyeServoCommand MechatronicEye::lookMidRight()
{
    return this->lookXY(7500, 5000);
}

// Mids Mixed
EyeServoCommand MechatronicEye::lookMidUpLeft()
{
    return this->lookXY(2500, 2500);
}
EyeServoCommand MechatronicEye::lookMidUpRight()
{
    return this->lookXY(7500, 2500);
}
EyeServoCommand MechatronicEye::lookMidDownLeft()
{
    return this->lookXY(2500, 7500);
}
EyeServoCommand MechatronicEye::lookMidDownRight()
{
    return this->lookXY(7500, 7500);
}

// HELPER FUNCTIONS

void MechatronicEye::setLeftLimit(int limit)
{
    _limits._left_limit = limit;
}

void MechatronicEye::setRightLimit(int limit)
{
    _limits._right_limit = limit;
}

void MechatronicEye::setHorizontalCenter(int center)
{
    _limits._horizontal_center = center;
}

void MechatronicEye::setTopLimit(int limit)
{
    _limits._top_limit = limit;
}

void MechatronicEye::setBottomLimit(int limit)
{
    _limits._bottom_limit = limit;
}

void MechatronicEye::setVerticalCenter(int center)
{
    _limits._vertical_center = center;
}
