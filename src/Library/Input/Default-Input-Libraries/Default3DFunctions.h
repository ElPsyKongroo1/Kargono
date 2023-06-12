#pragma once
namespace Default3DFunctions {
    // Mouse Movement
	void CAMERA_YAW_PITCH_MOUSE(double xpos, double ypos);
    // Scroll Wheel
	void CAMERA_FOV_MOUSE(double xoffset, double yoffset);
    // Button/Keyboard Click
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
    bool MOVE_FORWARD_KEY();
    bool MOVE_BACKWARD_KEY();
    bool MOVE_LEFT_KEY();
    bool MOVE_RIGHT_KEY();
    bool CAMERA_PITCH_UP_KEY();
    bool CAMERA_PITCH_DOWN_KEY();
    bool CAMERA_YAW_LEFT_KEY();
    bool CAMERA_YAW_RIGHT_KEY();
    // JoyStick
    bool MOVE_LEFT_RIGHT_STICK(float axis);
    bool MOVE_UP_DOWN_STICK(float axis);
    bool CAMERA_YAW_STICK(float axis);
    bool CAMERA_PITCH_STICK(float axis);
    // Gamepad Trigger
    void CAMERA_FOV_TRIGGER(float axis);
    void CAMERA_SPEED_TRIGGER(float axis);
}