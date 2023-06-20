#pragma once
namespace BreakoutActiveFunctions {
    // Scroll Wheel
    void CAMERA_FOV_MOUSE(double xoffset, double yoffset);
    // Button/Key Click
    bool TOGGLE_FLASHLIGHT(GLInputLink* gamePadButton);
    bool TOGGLE_DEVICE_MOUSE_MOVEMENT(GLInputLink* gamePadButton);
    bool EXIT_APPLICATION(GLInputLink* gamePadButton);
    bool RANDOM_FLASHLIGHT_COLOR(GLInputLink* gamePadButton);
    bool TOGGLE_MENU(GLInputLink* gamePadButton);
    bool CAMERA_DEINCREMENT_SENSITIVITY(GLInputLink* gamePadButton);
    bool CAMERA_INCREMENT_SENSITIVITY(GLInputLink* gamePadButton);
    bool CAMERA_DEINCREMENT_SPEED(GLInputLink* gamePadButton);
    bool CAMERA_INCREMENT_SPEED(GLInputLink* gamePadButton);
    bool INCREASE_PADDLE_SPEED_TOGGLE(GLInputLink* gamePadButton);
    bool MODIFY_PADDLE_DIRECTION_LEFT(GLInputLink* gamePadButton);
    bool MODIFY_PADDLE_DIRECTION_RIGHT(GLInputLink* gamePadButton);



    
    // Button/Key Release Click
    bool RESET_PADDLE_SPEED(GLInputLink* gamePadButton);
    bool RESET_PADDLE_DIRECTION(GLInputLink* gamePadButton);

    // Keyboard Hold
    bool MOVE_RIGHT_2D();
    bool MOVE_LEFT_2D();
    //Testing Hold Functions
    bool INCREMENT_BALL_SPEED();
    bool DEINCREMENT_BALL_SPEED();
    bool MOVE_BALL_LEFT();
    bool MOVE_BALL_RIGHT();
    bool MOVE_BALL_UP();
    bool MOVE_BALL_DOWN();

    // Joystick
    bool MOVE_LEFT_RIGHT_STICK_2D(float axis);
    bool MOVE_UP_DOWN_STICK_2D(float axis);
    // Trigger
    void CAMERA_SPEED_TRIGGER(float axis);
}