#pragma once
namespace Default2DFunctions {
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
    // Keyboard Hold
    bool MOVE_UP_2D();
    bool MOVE_DOWN_2D();
    bool MOVE_RIGHT_2D();
    bool MOVE_LEFT_2D();
    bool MOVE_UP_LEFT_2D();
    bool MOVE_DOWN_LEFT_2D();
    bool MOVE_UP_RIGHT_2D();
    bool MOVE_DOWN_RIGHT_2D();
    // Joystick
    bool MOVE_LEFT_RIGHT_STICK_2D(float axis);
    bool MOVE_UP_DOWN_STICK_2D(float axis);
    // Trigger
    void CAMERA_SPEED_TRIGGER(float axis);
}