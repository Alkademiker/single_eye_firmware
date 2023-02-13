
#pragma once

#define ADA_SERV_MAX 4096
#define ADA_SERV_MIN 0

struct EyeLimit
{
    // Horizontal
    int _left_limit;
    int _right_limit;
    int _horizontal_center;

    // Vertical
    int _top_limit;
    int _bottom_limit;
    int _vertical_center;
};

struct EyeServoCommand
{
    int horizontal_index;
    int horizontal_value;

    int vertical_index;
    int vertical_value;
};

enum EyePositions
{
    CENTER,
    // horizontal
    LEFT,
    MID_LEFT,
    MID_RIGHT,
    RIGHT,

    // vertical
    TOP,
    MID_TOP,
    MID_BOTTOM,
    BOTTOM,

    // Mid Mixed
    MID_TOP_RIGHT,
    MID_BOTTOM_RIGHT,
    MID_BOTTOM_LEFT,
    MID_TOP_LEFT,

    // Mixed
    TOP_RIGHT,
    BOTTOM_RIGHT,
    BOTTOM_LEFT,
    TOP_LEFT,

};

class MechatronicEye
{
private:
    // Command data
    EyeServoCommand _commands;
    
    // calibration data
    EyeLimit _limits;

public:
    MechatronicEye(
        int vertical_servo, 
        int horizontal_servo);

    MechatronicEye(
        int vertical_servo, 
        int horizontal_servo,
        EyeLimit limits);

    ~MechatronicEye();

    // Calibration functions
    // Horizontal
    void setLeftLimit(int limit);
    void setRightLimit(int limit);
    void setHorizontalCenter(int center);

    // Vertical
    void setTopLimit(int limit);
    void setBottomLimit(int limit);
    void setVerticalCenter(int center);

    EyeServoCommand lookXY(int horizontal, int vertical);

    EyeServoCommand lookAt(EyePositions position);
    EyeServoCommand randomWalk();
    EyeServoCommand deadRoll(int i);


    // Extremes
    EyeServoCommand lookCenter();

    EyeServoCommand lookUp();
    EyeServoCommand lookDown();

    EyeServoCommand lookLeft();
    EyeServoCommand lookRight();

    // Mids Vertical and Horizontal
    EyeServoCommand lookMidUp();
    EyeServoCommand lookMidDown();
    EyeServoCommand lookMidLeft();
    EyeServoCommand lookMidRight();

    // Mids Mixed
    EyeServoCommand lookMidUpLeft();
    EyeServoCommand lookMidUpRight();
    EyeServoCommand lookMidDownLeft();
    EyeServoCommand lookMidDownRight();
};
