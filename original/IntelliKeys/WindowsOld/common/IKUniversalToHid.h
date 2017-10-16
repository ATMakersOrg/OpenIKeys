
typedef struct
{
	int HIDUsagePage;
	int HIDUsageID;
	int universalCode;
} hidEntry;

static hidEntry UniversalToHid[] = {



{0x01,0x81,0},   //   System Power
{0x01,0x82,0},   //   System Sleep
{0x01,0x83,0},   //   System Wake
{0x07,0x00,0},   //   No Event
{0x07,0x01,0},   //   Overrun Error
{0x07,0x02,0},   //   POST Fail
{0x07,0x03,0},   //   ErrorUndefined
{0x07,0x04,UNIVERSAL_A},   //   a A
{0x07,0x05,UNIVERSAL_B},   //   b B
{0x07,0x06,UNIVERSAL_C},   //   c C
{0x07,0x07,UNIVERSAL_D},   //   d D
{0x07,0x08,UNIVERSAL_E},   //   e E
{0x07,0x09,UNIVERSAL_F },   //   f F
{0x07,0x0A,UNIVERSAL_G },   //   g G
{0x07,0x0B,UNIVERSAL_H  },   //   h H
{0x07,0x0C,UNIVERSAL_I  },   //   i I
{0x07,0x0D,UNIVERSAL_J },   //   j J
{0x07,0x0E,UNIVERSAL_K },   //   k K
{0x07,0x0F,UNIVERSAL_L  },   //   l L
{0x07,0x10,UNIVERSAL_M  },   //   m M
{0x07,0x11,UNIVERSAL_N },   //   n N
{0x07,0x12,UNIVERSAL_O },   //   o O
{0x07,0x13,UNIVERSAL_P },   //   p P
{0x07,0x14,UNIVERSAL_Q },   //   q Q
{0x07,0x15,UNIVERSAL_R  },   //   r R
{0x07,0x16,UNIVERSAL_S  },   //   s S
{0x07,0x17,UNIVERSAL_T  },   //   t T
{0x07,0x18,UNIVERSAL_U  },   //   u U
{0x07,0x19,UNIVERSAL_V },   //   v V
{0x07,0x1A,UNIVERSAL_W },   //   w W
{0x07,0x1B,UNIVERSAL_X },   //   x X
{0x07,0x1C,UNIVERSAL_Y },   //   y Y
{0x07,0x1D,UNIVERSAL_Z },   //   z Z
{0x07,0x1E,UNIVERSAL_1},   //   1 !
{0x07,0x1F,UNIVERSAL_2},   //   2 @
{0x07,0x20,UNIVERSAL_3},   //   3 #
{0x07,0x21,UNIVERSAL_4},   //   4 $
{0x07,0x22,UNIVERSAL_5},   //   5 %
{0x07,0x23,UNIVERSAL_6},   //   6 ^
{0x07,0x24,UNIVERSAL_7},   //   7 &
{0x07,0x25,UNIVERSAL_8},   //   8 *
{0x07,0x26,UNIVERSAL_9},   //   9 (
{0x07,0x27,UNIVERSAL_0},   //   0 )
{0x07,0x28,UNIVERSAL_ENTER},   //   Return
{0x07,0x28,UNIVERSAL_RETURN},   //   Return
{0x07,0x29,UNIVERSAL_ESCAPE},   //   Escape
{0x07,0x2A,UNIVERSAL_BACKSPACE},   //   Backspace
{0x07,0x2B,UNIVERSAL_TAB},   //   Tab
{0x07,0x2C,UNIVERSAL_SPACE},   //   Space
{0x07,0x2D,UNIVERSAL_MINUS},   //   - _
{0x07,0x2E,UNIVERSAL_EQUALS},   //   = +
{0x07,0x2F,UNIVERSAL_LEFT_BRACKET},   //   [ {
{0x07,0x30,UNIVERSAL_RIGHT_BRACKET},   //   ] }
{0x07,0x31,UNIVERSAL_BACKSLASH},   //   \ |  
{0x07,0x32,0},   //   Europe 1 (Note 2)
{0x07,0x33,UNIVERSAL_SEMICOLON},   //   ; :
{0x07,0x34,UNIVERSAL_QUOTE},   //   ' "
{0x07,0x35,UNIVERSAL_TILDE},   //   ` ~
{0x07,0x36,UNIVERSAL_COMMA},   //   , <
{0x07,0x37,UNIVERSAL_PERIOD},   //   . >
{0x07,0x38,UNIVERSAL_SLASH},   //   / ?
{0x07,0x39,UNIVERSAL_CAPS_LOCK},   //   Caps Lock
{0x07,0x3A,UNIVERSAL_F1},   //   F1
{0x07,0x3B,UNIVERSAL_F2},   //   F2
{0x07,0x3C,UNIVERSAL_F3},   //   F3
{0x07,0x3D,UNIVERSAL_F4},   //   F4
{0x07,0x3E,UNIVERSAL_F5},   //   F5
{0x07,0x3F,UNIVERSAL_F6},   //   F6
{0x07,0x40,UNIVERSAL_F7},   //   F7
{0x07,0x41,UNIVERSAL_F8},   //   F8
{0x07,0x42,UNIVERSAL_F9},   //   F9
{0x07,0x43,UNIVERSAL_F10},   //   F10
{0x07,0x44,UNIVERSAL_F11},   //   F11
{0x07,0x45,UNIVERSAL_F12},   //   F12
{0x07,0x46,UNIVERSAL_PRINT_SCREEN},   //   Print Screen (Note 1)
{0x07,0x47,UNIVERSAL_SCROLL_LOCK},   //   Scroll Lock
{0x07,0x48,0},   //   Break (Ctrl-Pause)
{0x07,0x48,UNIVERSAL_PAUSE},   //   Pause
{0x07,0x49,UNIVERSAL_INSERT},   //   Insert (Note 1)
{0x07,0x4A,UNIVERSAL_HOME},   //   Home (Note 1)
{0x07,0x4B,UNIVERSAL_PAGE_UP},   //   Page Up (Note 1)
{0x07,0x4C,UNIVERSAL_DELETE},   //   Delete (Note 1)
{0x07,0x4D,UNIVERSAL_END},   //   End (Note 1)
{0x07,0x4E,UNIVERSAL_PAGE_DOWN},   //   Page Down (Note 1)
{0x07,0x4F,UNIVERSAL_RIGHT_ARROW},   //   Right Arrow (Note 1)
{0x07,0x50,UNIVERSAL_LEFT_ARROW},   //   Left Arrow (Note 1)
{0x07,0x51,UNIVERSAL_DOWN_ARROW},   //   Down Arrow (Note 1)
{0x07,0x52,UNIVERSAL_UP_ARROW},   //   Up Arrow (Note 1)
{0x07,0x53,UNIVERSAL_NUM_LOCK},   //   Num Lock
{0x07,0x54,UNIVERSAL_NUMPAD_DIVIDE},   //   Keypad / (Note 1)
{0x07,0x55,UNIVERSAL_NUMPAD_MULTIPLY},   //   Keypad *
{0x07,0x56,UNIVERSAL_NUMPAD_SUBTRACT},   //   Keypad -
{0x07,0x57,UNIVERSAL_NUMPAD_ADD},   //   Keypad +
{0x07,0x58,UNIVERSAL_NUMPAD_ENTER},   //   Keypad Enter
{0x07,0x59,UNIVERSAL_NUMPAD_1},   //   Keypad 1 End
{0x07,0x5A,UNIVERSAL_NUMPAD_2},   //   Keypad 2 Down
{0x07,0x5B,UNIVERSAL_NUMPAD_3},   //   Keypad 3 PageDn
{0x07,0x5C,UNIVERSAL_NUMPAD_4},   //   Keypad 4 Left
{0x07,0x5D,UNIVERSAL_NUMPAD_5},   //   Keypad 5
{0x07,0x5E,UNIVERSAL_NUMPAD_6},   //   Keypad 6 Right
{0x07,0x5F,UNIVERSAL_NUMPAD_7},   //   Keypad 7 Home
{0x07,0x60,UNIVERSAL_NUMPAD_8},   //   Keypad 8 Up
{0x07,0x61,UNIVERSAL_NUMPAD_9},   //   Keypad 9 PageUp
{0x07,0x62,UNIVERSAL_NUMPAD_0},   //   Keypad 0 Insert
{0x07,0x63,UNIVERSAL_NUMPAD_DECIMAL},   //   Keypad . Delete
{0x07,0x64,0},   //   Europe 2 (Note 2)
{0x07,0x65,0},   //   App
{0x07,0x66,0},   //   Keyboard Power

{0x07,0x67,UNIVERSAL_NUMPAD_EQUAL},   //   Keypad =
//{0x07,0x58,UNIVERSAL_NUMPAD_EQUAL},   //   Keypad =

{0x07,0x68,UNIVERSAL_F13},   //   F13
{0x07,0x69,UNIVERSAL_F14},   //   F14
{0x07,0x6A,UNIVERSAL_F15},   //   F15
{0x07,0x6B,0},   //   F16
{0x07,0x6C,0},   //   F17
{0x07,0x6D,0},   //   F18
{0x07,0x6E,0},   //   F19
{0x07,0x6F,0},   //   F20
{0x07,0x70,0},   //   F21
{0x07,0x71,0},   //   F22
{0x07,0x72,0},   //   F23
{0x07,0x73,0},   //   F24
{0x07,0x74,0},   //   Keyboard Execute
{0x07,0x75,0},   //   Keyboard Help
{0x07,0x76,0},   //   Keyboard Menu
{0x07,0x77,0},   //   Keyboard Select
{0x07,0x78,0},   //   Keyboard Stop
{0x07,0x79,0},   //   Keyboard Again
{0x07,0x7A,0},   //   Keyboard Undo
{0x07,0x7B,0},   //   Keyboard Cut
{0x07,0x7C,0},   //   Keyboard Copy
{0x07,0x7D,0},   //   Keyboard Paste
{0x07,0x7E,0},   //   Keyboard Find
{0x07,0x7F,0},   //   Keyboard Mute
{0x07,0x80,0},   //   Keyboard Volume Up
{0x07,0x81,0},   //   Keyboard Volume Dn
{0x07,0x82,0},   //   Keyboard Locking Caps Lock
{0x07,0x83,0},   //   Keyboard Locking Num Lock
{0x07,0x84,0},   //   Keyboard Locking Scroll Lock
{0x07,0x85,0},   //   Keypad , (Brazilian Keypad .)
{0x07,0x86,0},   //   Keyboard Equal Sign
{0x07,0x87,0},   //   Keyboard Int'l 1?(Ro)
{0x07,0x88,0},   //   Keyboard Intl'2????????????(Katakana/Hiragana)
{0x07,0x89,0},   //   Keyboard Int'l 2¥(Yen)
{0x07,0x8A,0},   //   Keyboard Int'l 4????? (???)???(Henkan)
{0x07,0x8B,0},   //   Keyboard Int'l 5???(Muhenkan)
{0x07,0x8C,0},   //   Keyboard Int'l 6 (PC9800 Keypad , )
{0x07,0x8D,0},   //   Keyboard Int'l 7
{0x07,0x8E,0},   //   Keyboard Int'l 8
{0x07,0x8F,0},   //   Keyboard Int'l 9
{0x07,0x90,0},   //   Keyboard Lang 1?/?(Hanguel/English)
{0x07,0x91,0},   //   Keyboard Lang 2??(Hanja)
{0x07,0x92,0},   //   Keyboard Lang 3????(Katakana)
{0x07,0x93,0},   //   Keyboard Lang 4????(Hiragana)
{0x07,0x94,0},   //   Keyboard Lang 5??/??(Zenkaku/Hankaku)
{0x07,0x95,0},   //   Keyboard Lang 6
{0x07,0x96,0},   //   Keyboard Lang 7
{0x07,0x97,0},   //   Keyboard Lang 8
{0x07,0x98,0},   //   Keyboard Lang 9
{0x07,0x99,0},   //   Keyboard Alternate Erase
{0x07,0x9A,0},   //   Keyboard SysReq/Attention
{0x07,0x9B,0},   //   Keyboard Cancel
{0x07,0x9C,0},   //   Keyboard Clear
{0x07,0x9D,0},   //   Keyboard Prior
{0x07,0x9E,0},   //   Keyboard Return
{0x07,0x9F,0},   //   Keyboard Separator
{0x07,0xA0,0},   //   Keyboard Out
{0x07,0xA1,0},   //   Keyboard Oper
{0x07,0xA2,0},   //   Keyboard Clear/Again
{0x07,0xA3,0},   //   Keyboard CrSel/Props
{0x07,0xA4,0},   //   Keyboard ExSel
{0x07,0xE0,UNIVERSAL_CONTROL},   //   Left Control
{0x07,0xE1,UNIVERSAL_SHIFT},   //   Left Shift
{0x07,0xE6,UNIVERSAL_OPTION},   //   Left Alt
{0x07,0xE6,UNIVERSAL_ALT},   //   Left Alt
{0x07,0xE6,UNIVERSAL_MENU},   //   Left Alt
{0x07,0xE3,UNIVERSAL_COMMAND},   //   Left GUI
{0x07,0xE4,UNIVERSAL_RIGHT_CONTROL},   //   Right Control
{0x07,0xE5,UNIVERSAL_RIGHT_SHIFT},   //   Right Shift
{0x07,0xE2,UNIVERSAL_ALTGR},   //   Right Alt
{0x07,0xE7,0},   //   Right GUI
{0x0C,0x00B5,0},   //   Scan Next Track
{0x0C,0x00B6,0},   //   Scan Previous Track
{0x0C,0x00B7,0},   //   Stop
{0x0C,0x00CD,0},   //   Play/ Pause
{0x0C,0x00E2,0},   //   Mute
{0x0C,0x00E5,0},   //   Bass Boost
{0x0C,0x00E7,0},   //   Loudness
{0x0C,0x00E9,0},   //   Volume Up
{0x0C,0x00EA,0},   //   Volume Down
{0x0C,0x0152,0},   //   Bass Up
{0x0C,0x0153,0},   //   Bass Down
{0x0C,0x0154,0},   //   Treble Up
{0x0C,0x0155,0},   //   Treble Down
{0x0C,0x0183,0},   //   Media Select
{0x0C,0x018A,0},   //   Mail
{0x0C,0x0192,0},   //   Calculator
{0x0C,0x0194,0},   //   My Computer
{0x0C,0x0221,0},   //   WWW Search
{0x0C,0x0223,0},   //   WWW Home
{0x0C,0x0224,0},   //   WWW Back
{0x0C,0x0225,0},   //   WWW Forward
{0x0C,0x0226,0},   //   WWW Stop
{0x0C,0x0227,0},   //   WWW Refresh
{0x0C,0x022A,0},   //   WWW Favorites


};