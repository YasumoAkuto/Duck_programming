//キー入力の処理

#include <windows.h>
#include "MainResource.h"



/// <summary>
/// キー入力を読み取る
/// </summary>
int readKey() {
    int key_input = 0;
    if (GetAsyncKeyState('A') & 0x8000)         key_input |= KEY_LEFT;
    if (GetAsyncKeyState('D') & 0x8000)         key_input |= KEY_RIGHT;
    if (GetAsyncKeyState('W') & 0x8000)         key_input |= KEY_UP;
    if (GetAsyncKeyState(' ') & 0x8000)         key_input |= KEY_SPACE;
    if (GetAsyncKeyState('P') & 0x8000)         key_input |= KEY_P;
    if (GetAsyncKeyState('1') & 0x8000)         key_input |= KEY_1;
    if (GetAsyncKeyState('2') & 0x8000)         key_input |= KEY_2;
    if (GetAsyncKeyState('3') & 0x8000)         key_input |= KEY_3;
    if (GetAsyncKeyState('E') & 0x8000)         key_input |= KEY_E;
    if (GetAsyncKeyState('B') & 0x8000)         key_input |= KEY_B;
    if (GetAsyncKeyState('R') & 0x8000)         key_input |= KEY_R;
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)   key_input |= KEY_ESC;
    if (GetAsyncKeyState(VK_UP) & 0x8000)       key_input |= KEY_UPARROW;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000)       key_input |= KEY_LEFTARROW;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000)       key_input |= KEY_RIGHTARROW;
    return key_input;
}

/// <summary>
/// トリガセンスのキー入力を返す
/// </summary>
/// <returns>key_input_triggersense</returns>
int KeyInputTriggerSense(int key_input) {
    static int beforeKeyInput2 = 0;
    int key_input_triggersense = 0;
    if (key_input & KEY_LEFT)   if (!(beforeKeyInput2 & KEY_LEFT))  key_input_triggersense |= KEY_LEFT;
    if (key_input & KEY_RIGHT)  if (!(beforeKeyInput2 & KEY_RIGHT)) key_input_triggersense |= KEY_RIGHT; //右一回だけ
    if (key_input & KEY_UP)     if (!(beforeKeyInput2 & KEY_UP))    key_input_triggersense |= KEY_UP;
    if (key_input & KEY_SPACE)  if (!(beforeKeyInput2 & KEY_SPACE)) key_input_triggersense |= KEY_SPACE;
    if (key_input & KEY_P)      if (!(beforeKeyInput2 & KEY_P))     key_input_triggersense |= KEY_P;
    if (key_input & KEY_1)      if (!(beforeKeyInput2 & KEY_1))     key_input_triggersense |= KEY_1;
    if (key_input & KEY_2)      if (!(beforeKeyInput2 & KEY_2))     key_input_triggersense |= KEY_2;
    if (key_input & KEY_3)      if (!(beforeKeyInput2 & KEY_3))     key_input_triggersense |= KEY_3;
    if (key_input & KEY_E)      if (!(beforeKeyInput2 & KEY_E))     key_input_triggersense |= KEY_E;
    if (key_input & KEY_B)      if (!(beforeKeyInput2 & KEY_B))     key_input_triggersense |= KEY_B;
    if (key_input & KEY_R)      if (!(beforeKeyInput2 & KEY_R))     key_input_triggersense |= KEY_R;
    if (key_input & KEY_ESC)    if (!(beforeKeyInput2 & KEY_ESC))   key_input_triggersense |= KEY_ESC;
    if (key_input & KEY_UPARROW)if (!(beforeKeyInput2 & KEY_UPARROW))key_input_triggersense |= KEY_UPARROW;
    if (key_input & KEY_LEFTARROW)    if (!(beforeKeyInput2 & KEY_LEFTARROW))   key_input_triggersense |= KEY_LEFTARROW;
    if (key_input & KEY_RIGHTARROW)    if (!(beforeKeyInput2 & KEY_RIGHTARROW))   key_input_triggersense |= KEY_RIGHTARROW;
    beforeKeyInput2 = key_input;
    return key_input_triggersense;
}