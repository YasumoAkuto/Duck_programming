#ifndef __MAINRESOURCE_H__
#define __MAINRESOURCE_H__


#define KEY_LEFT 1
#define KEY_RIGHT 2
#define KEY_UP 4
#define KEY_SPACE 8
#define KEY_E 16
#define KEY_P 32
#define KEY_1 64
#define KEY_2 128
#define KEY_B 256
#define KEY_R 512
#define KEY_3 1024
#define KEY_ESC 2048
#define KEY_UPARROW 4096
#define KEY_LEFTARROW 8192
#define KEY_RIGHTARROW 16384

int readKey();
int KeyInputTriggerSense(int key_input);



#endif