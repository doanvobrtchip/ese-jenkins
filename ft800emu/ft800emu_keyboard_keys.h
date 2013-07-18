/**
 * Key defines
 * $Id$
 * \file ft800emu_keyboard_keys.h
 * \brief Key defines
 * \date 2013-07-18 20:16GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_KEYBOARD_KEYS_H
#define FT800EMU_KEYBOARD_KEYS_H
// #include <...>

// System includes

// Project includes

#ifndef FT800EMU_SDL

#include "ft800emu_system_windows.h"
#define FT800EMU_KEY_ESCAPE          DIK_ESCAPE
#define FT800EMU_KEY_1               DIK_1
#define FT800EMU_KEY_2               DIK_2
#define FT800EMU_KEY_3               DIK_3
#define FT800EMU_KEY_4               DIK_4
#define FT800EMU_KEY_5               DIK_5
#define FT800EMU_KEY_6               DIK_6
#define FT800EMU_KEY_7               DIK_7
#define FT800EMU_KEY_8               DIK_8
#define FT800EMU_KEY_9               DIK_9
#define FT800EMU_KEY_0               DIK_0
#define FT800EMU_KEY_MINUS           DIK_MINUS
#define FT800EMU_KEY_EQUALS          DIK_EQUALS
#define FT800EMU_KEY_BACK            DIK_BACK
#define FT800EMU_KEY_TAB             DIK_TAB
#define FT800EMU_KEY_Q               DIK_Q
#define FT800EMU_KEY_W               DIK_W
#define FT800EMU_KEY_E               DIK_E
#define FT800EMU_KEY_R               DIK_R
#define FT800EMU_KEY_T               DIK_T
#define FT800EMU_KEY_Y               DIK_Y
#define FT800EMU_KEY_U               DIK_U
#define FT800EMU_KEY_I               DIK_I
#define FT800EMU_KEY_O               DIK_O
#define FT800EMU_KEY_P               DIK_P
#define FT800EMU_KEY_LBRACKET        DIK_LBRACKET
#define FT800EMU_KEY_RBRACKET        DIK_RBRACKET
#define FT800EMU_KEY_RETURN          DIK_RETURN
#define FT800EMU_KEY_LCONTROL        DIK_LCONTROL
#define FT800EMU_KEY_A               DIK_A
#define FT800EMU_KEY_S               DIK_S
#define FT800EMU_KEY_D               DIK_D
#define FT800EMU_KEY_F               DIK_F
#define FT800EMU_KEY_G               DIK_G
#define FT800EMU_KEY_H               DIK_H
#define FT800EMU_KEY_J               DIK_J
#define FT800EMU_KEY_K               DIK_K
#define FT800EMU_KEY_L               DIK_L
#define FT800EMU_KEY_SEMICOLON       DIK_SEMICOLON
#define FT800EMU_KEY_APOSTROPHE      DIK_APOSTROPHE
#define FT800EMU_KEY_GRAVE           DIK_GRAVE
#define FT800EMU_KEY_LSHIFT          DIK_LSHIFT
#define FT800EMU_KEY_BACKSLASH       DIK_BACKSLASH
#define FT800EMU_KEY_Z               DIK_Z
#define FT800EMU_KEY_X               DIK_X
#define FT800EMU_KEY_C               DIK_C
#define FT800EMU_KEY_V               DIK_V
#define FT800EMU_KEY_B               DIK_B
#define FT800EMU_KEY_N               DIK_N
#define FT800EMU_KEY_M               DIK_M
#define FT800EMU_KEY_COMMA           DIK_COMMA
#define FT800EMU_KEY_PERIOD          DIK_PERIOD
#define FT800EMU_KEY_SLASH           DIK_SLASH
#define FT800EMU_KEY_RSHIFT          DIK_RSHIFT
#define FT800EMU_KEY_MULTIPLY        DIK_MULTIPLY
#define FT800EMU_KEY_LMENU           DIK_LMENU
#define FT800EMU_KEY_SPACE           DIK_SPACE
#define FT800EMU_KEY_CAPITAL         DIK_CAPITAL
#define FT800EMU_KEY_F1              DIK_F1
#define FT800EMU_KEY_F2              DIK_F2
#define FT800EMU_KEY_F3              DIK_F3
#define FT800EMU_KEY_F4              DIK_F4
#define FT800EMU_KEY_F5              DIK_F5
#define FT800EMU_KEY_F6              DIK_F6
#define FT800EMU_KEY_F7              DIK_F7
#define FT800EMU_KEY_F8              DIK_F8
#define FT800EMU_KEY_F9              DIK_F9
#define FT800EMU_KEY_F10             DIK_F10
#define FT800EMU_KEY_NUMLOCK         DIK_NUMLOCK
#define FT800EMU_KEY_SCROLL          DIK_SCROLL
#define FT800EMU_KEY_NUMPAD7         DIK_NUMPAD7
#define FT800EMU_KEY_NUMPAD8         DIK_NUMPAD8
#define FT800EMU_KEY_NUMPAD9         DIK_NUMPAD9
#define FT800EMU_KEY_SUBTRACT        DIK_SUBTRACT
#define FT800EMU_KEY_NUMPAD4         DIK_NUMPAD4
#define FT800EMU_KEY_NUMPAD5         DIK_NUMPAD5
#define FT800EMU_KEY_NUMPAD6         DIK_NUMPAD6
#define FT800EMU_KEY_ADD             DIK_ADD
#define FT800EMU_KEY_NUMPAD1         DIK_NUMPAD1
#define FT800EMU_KEY_NUMPAD2         DIK_NUMPAD2
#define FT800EMU_KEY_NUMPAD3         DIK_NUMPAD3
#define FT800EMU_KEY_NUMPAD0         DIK_NUMPAD0
#define FT800EMU_KEY_DECIMAL         DIK_DECIMAL
#define FT800EMU_KEY_OEM_102         DIK_OEM_102
#define FT800EMU_KEY_F11             DIK_F11
#define FT800EMU_KEY_F12             DIK_F12
#define FT800EMU_KEY_F13             DIK_F13
#define FT800EMU_KEY_F14             DIK_F14
#define FT800EMU_KEY_F15             DIK_F15
#define FT800EMU_KEY_KANA            DIK_KANA
#define FT800EMU_KEY_ABNT_C1         DIK_ABNT_C1
#define FT800EMU_KEY_CONVERT         DIK_CONVERT
#define FT800EMU_KEY_NOCONVERT       DIK_NOCONVERT
#define FT800EMU_KEY_YEN             DIK_YEN
#define FT800EMU_KEY_ABNT_C2         DIK_ABNT_C2
#define FT800EMU_KEY_NUMPADEQUALS    DIK_NUMPADEQUALS
#define FT800EMU_KEY_PREVTRACK       DIK_PREVTRACK
#define FT800EMU_KEY_AT              DIK_AT
#define FT800EMU_KEY_COLON           DIK_COLON
#define FT800EMU_KEY_UNDERLINE       DIK_UNDERLINE
#define FT800EMU_KEY_KANJI           DIK_KANJI
#define FT800EMU_KEY_STOP            DIK_STOP
#define FT800EMU_KEY_AX              DIK_AX
#define FT800EMU_KEY_UNLABELED       DIK_UNLABELED
#define FT800EMU_KEY_NEXTTRACK       DIK_NEXTTRACK
#define FT800EMU_KEY_NUMPADENTER     DIK_NUMPADENTER
#define FT800EMU_KEY_RCONTROL        DIK_RCONTROL
#define FT800EMU_KEY_MUTE            DIK_MUTE
#define FT800EMU_KEY_CALCULATOR      DIK_CALCULATOR
#define FT800EMU_KEY_PLAYPAUSE       DIK_PLAYPAUSE
#define FT800EMU_KEY_MEDIASTOP       DIK_MEDIASTOP
#define FT800EMU_KEY_VOLUMEDOWN      DIK_VOLUMEDOWN
#define FT800EMU_KEY_VOLUMEUP        DIK_VOLUMEUP
#define FT800EMU_KEY_WEBHOME         DIK_WEBHOME
#define FT800EMU_KEY_NUMPADCOMMA     DIK_NUMPADCOMMA
#define FT800EMU_KEY_DIVIDE          DIK_DIVIDE
#define FT800EMU_KEY_SYSRQ           DIK_SYSRQ
#define FT800EMU_KEY_RMENU           DIK_RMENU
#define FT800EMU_KEY_PAUSE           DIK_PAUSE
#define FT800EMU_KEY_HOME            DIK_HOME
#define FT800EMU_KEY_UP              DIK_UP
#define FT800EMU_KEY_PRIOR           DIK_PRIOR
#define FT800EMU_KEY_LEFT            DIK_LEFT
#define FT800EMU_KEY_RIGHT           DIK_RIGHT
#define FT800EMU_KEY_END             DIK_END
#define FT800EMU_KEY_DOWN            DIK_DOWN
#define FT800EMU_KEY_NEXT            DIK_NEXT
#define FT800EMU_KEY_INSERT          DIK_INSERT
#define FT800EMU_KEY_DELETE          DIK_DELETE
#define FT800EMU_KEY_LWIN            DIK_LWIN
#define FT800EMU_KEY_RWIN            DIK_RWIN
#define FT800EMU_KEY_APPS            DIK_APPS
#define FT800EMU_KEY_POWER           DIK_POWER
#define FT800EMU_KEY_SLEEP           DIK_SLEEP
#define FT800EMU_KEY_WAKE            DIK_WAKE
#define FT800EMU_KEY_WEBSEARCH       DIK_WEBSEARCH
#define FT800EMU_KEY_WEBFAVORITES    DIK_WEBFAVORITES
#define FT800EMU_KEY_WEBREFRESH      DIK_WEBREFRESH
#define FT800EMU_KEY_WEBSTOP         DIK_WEBSTOP
#define FT800EMU_KEY_WEBFORWARD      DIK_WEBFORWARD
#define FT800EMU_KEY_WEBBACK         DIK_WEBBACK
#define FT800EMU_KEY_MYCOMPUTER      DIK_MYCOMPUTER
#define FT800EMU_KEY_MAIL            DIK_MAIL
#define FT800EMU_KEY_MEDIASELECT     DIK_MEDIASELECT
#define FT800EMU_KEY_BACKSPACE       DIK_BACKSPACE
#define FT800EMU_KEY_NUMPADSTAR      DIK_NUMPADSTAR
#define FT800EMU_KEY_LALT            DIK_LALT
#define FT800EMU_KEY_CAPSLOCK        DIK_CAPSLOCK
#define FT800EMU_KEY_NUMPADMINUS     DIK_NUMPADMINUS
#define FT800EMU_KEY_NUMPADPLUS      DIK_NUMPADPLUS
#define FT800EMU_KEY_NUMPADPERIOD    DIK_NUMPADPERIOD
#define FT800EMU_KEY_NUMPADSLASH     DIK_NUMPADSLASH
#define FT800EMU_KEY_RALT            DIK_RALT
#define FT800EMU_KEY_UPARROW         DIK_UPARROW
#define FT800EMU_KEY_PGUP            DIK_PGUP
#define FT800EMU_KEY_LEFTARROW       DIK_LEFTARROW
#define FT800EMU_KEY_RIGHTARROW      DIK_RIGHTARROW
#define FT800EMU_KEY_DOWNARROW       DIK_DOWNARROW
#define FT800EMU_KEY_PGDN            DIK_PGDN
#define FT800EMU_KEY_CIRCUMFLEX      DIK_CIRCUMFLEX

#endif

#endif /* #ifndef FT800EMU_KEYBOARD_KEYS_H */

/* end of file */
