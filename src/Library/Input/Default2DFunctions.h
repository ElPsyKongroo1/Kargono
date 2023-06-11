#pragma once
namespace Default2DFunctions {

	void CAMERA_FOV_MOUSE(double xoffset, double yoffset);

    bool TOGGLE_FLASHLIGHT(GLInputLink* gamePadButton);
    bool TOGGLE_DEVICE_MOUSE_MOVEMENT(GLInputLink* gamePadButton);
    bool EXIT_APPLICATION(GLInputLink* gamePadButton);
    bool RANDOM_FLASHLIGHT_COLOR(GLInputLink* gamePadButton);
    bool TOGGLE_MENU(GLInputLink* gamePadButton);
    bool CAMERA_DEINCREMENT_SENSITIVITY(GLInputLink* gamePadButton);
    bool CAMERA_INCREMENT_SENSITIVITY(GLInputLink* gamePadButton);
    bool CAMERA_DEINCREMENT_SPEED(GLInputLink* gamePadButton);
    bool CAMERA_INCREMENT_SPEED(GLInputLink* gamePadButton);
}