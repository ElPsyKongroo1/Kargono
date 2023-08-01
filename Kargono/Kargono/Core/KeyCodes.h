#pragma once


// From GLFW/glfw3.h
/* The unknown key */
#define KG_KEY_UNKNOWN            -1

/* Printable keys */
#define KG_KEY_SPACE              32
#define KG_KEY_APOSTROPHE         39  /* ' */
#define KG_KEY_COMMA              44  /* , */
#define KG_KEY_MINUS              45  /* - */
#define KG_KEY_PERIOD             46  /* . */
#define KG_KEY_SLASH              47  /* / */
#define KG_KEY_0                  48
#define KG_KEY_1                  49
#define KG_KEY_2                  50
#define KG_KEY_3                  51
#define KG_KEY_4                  52
#define KG_KEY_5                  53
#define KG_KEY_6                  54
#define KG_KEY_7                  55
#define KG_KEY_8                  56
#define KG_KEY_9                  57
#define KG_KEY_SEMICOLON          59  /* ; */
#define KG_KEY_EQUAL              61  /* = */
#define KG_KEY_A                  65
#define KG_KEY_B                  66
#define KG_KEY_C                  67
#define KG_KEY_D                  68
#define KG_KEY_E                  69
#define KG_KEY_F                  70
#define KG_KEY_G                  71
#define KG_KEY_H                  72
#define KG_KEY_I                  73
#define KG_KEY_J                  74
#define KG_KEY_K                  75
#define KG_KEY_L                  76
#define KG_KEY_M                  77
#define KG_KEY_N                  78
#define KG_KEY_O                  79
#define KG_KEY_P                  80
#define KG_KEY_Q                  81
#define KG_KEY_R                  82
#define KG_KEY_S                  83
#define KG_KEY_T                  84
#define KG_KEY_U                  85
#define KG_KEY_V                  86
#define KG_KEY_W                  87
#define KG_KEY_X                  88
#define KG_KEY_Y                  89
#define KG_KEY_Z                  90
#define KG_KEY_LEFT_BRACKET       91  /* [ */
#define KG_KEY_BACKSLASH          92  /* \ */
#define KG_KEY_RIGHT_BRACKET      93  /* ] */
#define KG_KEY_GRAVE_ACCENT       96  /* ` */
#define KG_KEY_WORLD_1            161 /* non-US #1 */
#define KG_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define KG_KEY_ESCAPE             256
#define KG_KEY_ENTER              257
#define KG_KEY_TAB                258
#define KG_KEY_BACKSPACE          259
#define KG_KEY_INSERT             260
#define KG_KEY_DELETE             261
#define KG_KEY_RIGHT              262
#define KG_KEY_LEFT               263
#define KG_KEY_DOWN               264
#define KG_KEY_UP                 265
#define KG_KEY_PAGE_UP            266
#define KG_KEY_PAGE_DOWN          267
#define KG_KEY_HOME               268
#define KG_KEY_END                269
#define KG_KEY_CAPS_LOCK          280
#define KG_KEY_SCROLL_LOCK        281
#define KG_KEY_NUM_LOCK           282
#define KG_KEY_PRINT_SCREEN       283
#define KG_KEY_PAUSE              284
#define KG_KEY_F1                 290
#define KG_KEY_F2                 291
#define KG_KEY_F3                 292
#define KG_KEY_F4                 293
#define KG_KEY_F5                 294
#define KG_KEY_F6                 295
#define KG_KEY_F7                 296
#define KG_KEY_F8                 297
#define KG_KEY_F9                 298
#define KG_KEY_F10                299
#define KG_KEY_F11                300
#define KG_KEY_F12                301
#define KG_KEY_F13                302
#define KG_KEY_F14                303
#define KG_KEY_F15                304
#define KG_KEY_F16                305
#define KG_KEY_F17                306
#define KG_KEY_F18                307
#define KG_KEY_F19                308
#define KG_KEY_F20                309
#define KG_KEY_F21                310
#define KG_KEY_F22                311
#define KG_KEY_F23                312
#define KG_KEY_F24                313
#define KG_KEY_F25                314
#define KG_KEY_KP_0               320
#define KG_KEY_KP_1               321
#define KG_KEY_KP_2               322
#define KG_KEY_KP_3               323
#define KG_KEY_KP_4               324
#define KG_KEY_KP_5               325
#define KG_KEY_KP_6               326
#define KG_KEY_KP_7               327
#define KG_KEY_KP_8               328
#define KG_KEY_KP_9               329
#define KG_KEY_KP_DECIMAL         330
#define KG_KEY_KP_DIVIDE          331
#define KG_KEY_KP_MULTIPLY        332
#define KG_KEY_KP_SUBTRACT        333
#define KG_KEY_KP_ADD             334
#define KG_KEY_KP_ENTER           335
#define KG_KEY_KP_EQUAL           336
#define KG_KEY_LEFT_SHIFT         340
#define KG_KEY_LEFT_CONTROL       341
#define KG_KEY_LEFT_ALT           342
#define KG_KEY_LEFT_SUPER         343
#define KG_KEY_RIGHT_SHIFT        344
#define KG_KEY_RIGHT_CONTROL      345
#define KG_KEY_RIGHT_ALT          346
#define KG_KEY_RIGHT_SUPER        347
#define KG_KEY_MENU               348