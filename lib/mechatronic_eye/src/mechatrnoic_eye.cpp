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
    this->_commands.horizontal_value = map(horizontal, 0, 1000, 
                _limits._left_limit, _limits._right_limit);

    this->_commands.vertical_value = map(vertical, 0, 1000,
                _limits._top_limit, _limits._bottom_limit);
    
    return _commands;
}

// Extremes
EyeServoCommand MechatronicEye::lookUp()
{
    return this->lookXY(500, 0);
}
EyeServoCommand MechatronicEye::lookDown()
{
    return this->lookXY(500, 1000);
}

EyeServoCommand MechatronicEye::lookLeft()
{
    return this->lookXY(0, 500);
}
EyeServoCommand MechatronicEye::lookRight()
{
    return this->lookXY(1000, 500);
}

// Mids Vertical and Horizontal
EyeServoCommand MechatronicEye::lookMidUp()
{
    return this->lookXY(500, 250);
}
EyeServoCommand MechatronicEye::lookMidDown()
{
    return this->lookXY(500, 750);
}
EyeServoCommand MechatronicEye::lookMidLeft()
{
    return this->lookXY(250, 500);
}
EyeServoCommand MechatronicEye::lookMidRight()
{
    return this->lookXY(750, 500);
}

// Mids Mixed
EyeServoCommand MechatronicEye::lookMidUpLeft()
{
    return this->lookXY(250, 250);
}
EyeServoCommand MechatronicEye::lookMidUpRight()
{
    return this->lookXY(750, 250);
}
EyeServoCommand MechatronicEye::lookMidDownLeft()
{
    return this->lookXY(250, 750);
}
EyeServoCommand MechatronicEye::lookMidDownRight()
{
    return this->lookXY(750, 750);
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
