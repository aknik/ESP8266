#include <SoftwareSerial.h>
#include "FS.h"

File f;

#define rxPin 14
#define txPin 12

#define MOD_CONTROL_LEFT    0x01
#define MOD_SHIFT_LEFT      0x02
#define MOD_ALT_LEFT        0x04
#define MOD_GUI_LEFT        0x08
#define MOD_CONTROL_RIGHT   0x10
#define MOD_SHIFT_RIGHT     0x20
#define MOD_ALT_RIGHT       0x40
#define MOD_GUI_RIGHT       0x80

#define KEY_ENTER             0x28     // Keyboard Return (ENTER)
#define KEY_NOKEY             0x00     // Reserved (no event indicated)
#define KEY_ERR_ROLLOVER      0x01     // Keyboard ErrorRollOver
#define KEY_POSTFAIL          0x02     // Keyboard POSTFail
#define KEY_ERR_UNDEFINED     0x03     // Keyboard ErrorUndefined


#define KEY_ESCAPE            0x29     // Keyboard ESCAPE
#define KEY_DELETE            0x2A     // Keyboard DELETE (Backspace)
#define KEY_TAB               0x2B     // Keyboard Tab
#define KEY_SPACE             0x2C     // Keyboard Spacebar
#define KEY_MINUS             0x2D     // Keyboard - and (underscore)
#define KEY_EQUAL             0x2E     // Keyboard = and +
#define KEY_L_BRACKET         0x2F     // Keyboard [ and {
#define KEY_R_BRACKET         0x30     // Keyboard ] and }
#define KEY_BACKSLASH         0x31     // Keyboard \ and |
#define KEY_HASH              0x32     // Keyboard Non-US # and ~
#define KEY_SEMICOLON         0x33     // Keyboard ; and :
#define KEY_APOSTROPHE        0x34     // Keyboard ‘ and “
#define KEY_GRAVE             0x35     // Keyboard Grave Accent and Tilde
#define KEY_COMMA             0x36     // Keyboard , and <
#define KEY_DOT               0x37     // Keyboard . and >
#define KEY_SLASH             0x38    24 // Keyboard / and ?
#define KEY_CAPS_LOCK         0x39     // Keyboard Caps Lock
#define KEY_F1                0x3A     // Keyboard F1
#define KEY_F2                0x3B     // Keyboard F2
#define KEY_F3                0x3C     // Keyboard F3
#define KEY_F4                0x3D     // Keyboard F4
#define KEY_F5                0x3E     // Keyboard F5
#define KEY_F6                0x3F     // Keyboard F6
#define KEY_F7                0x40     // Keyboard F7
#define KEY_F8                0x41     // Keyboard F8
#define KEY_F9                0x42     // Keyboard F9
#define KEY_F10               0x43     // Keyboard F10
#define KEY_F11               0x44     // Keyboard F11
#define KEY_F12               0x45     // Keyboard F12
#define KEY_PRTSCN            0x46     // Keyboard PrintScreen
#define KEY_SCR_LOCK          0x47     // Keyboard Scroll Lock
#define KEY_PAUSE             0x48     // Keyboard Pause
#define KEY_INSERT            0x49     // Keyboard Insert
#define KEY_HOME              0x4A     // Keyboard Home
#define KEY_PAGE_UP           0x4B     // Keyboard PageUp
#define KEY_DELETE            0x4C     // Keyboard Delete Forward
#define KEY_END               0x4D     // Keyboard End
#define KEY_PAGE_DOWN         0x4E     // Keyboard PageDown
#define KEY_ARROW_RIGHT       0x4F     // Keyboard RightArrow
#define KEY_ARROW_LEFT        0x50     // Keyboard LeftArrow
#define KEY_ARROW_DOWN        0x51     // Keyboard DownArrow
#define KEY_ARROW_UP          0x52     // Keyboard UpArrow

#define KEY_RIGHT_ARROW       0x4F     // Keyboard RightArrow  -> Alternative arrow keys names
#define KEY_LEFT_ARROW        0x50     // Keyboard LeftArrow
#define KEY_DOWN_ARROW        0x51     // Keyboard DownArrow
#define KEY_UP_ARROW          0x52     // Keyboard UpArrow

#define KEY_NUM_LOCK          0x53     // Keypad Num Lock and Clear
#define KEY_KPAD_SLASH        0x54     // Keypad /
#define KEY_KPAD_TIMES        0x55     // Keypad *
#define KEY_KPAD_MINUS        0x56     // Keypad -
#define KEY_KPAD_PLUS         0x57     // Keypad +
#define KEY_KPAD_ENTER        0x58     // Keypad ENTER
#define KEY_KPAD_1            0x59     // Keypad 1 and End
#define KEY_KPAD_2            0x5A     // Keypad 2 and Down Arrow
#define KEY_KPAD_3            0x5B     // Keypad 3 and PageDn
#define KEY_KPAD_4            0x5C     // Keypad 4 and Left Arrow
#define KEY_KPAD_5            0x5D     // Keypad 5
#define KEY_KPAD_6            0x5E     // Keypad 6 and Right Arrow
#define KEY_KPAD_7            0x5F     // Keypad 7 and Home
#define KEY_KPAD_8            0x60     // Keypad 8 and Up Arrow
#define KEY_KPAD_9            0x61     // Keypad 9 and PageUp
#define KEY_KPAD_0            0x62     // Keypad 0 and Insert
#define KEY_KPAD_DOT          0x63     // Keypad . and Delete
#define KEY_INTL_BACKSLASH    0x64     // Keyboard Non-US \ and |
#define KEY_APPLICATION       0x65     // Keyboard Application
#define KEY_POWER             0x66     // Keyboard Power
#define KEY_KPAD_EQUAL        0x67     // Keypad =
#define KEY_F13               0x68     // Keyboard F13
#define KEY_F14               0x69     // Keyboard F14
#define KEY_F15               0x6A     // Keyboard F15
#define KEY_F16               0x6B     // Keyboard F16
#define KEY_F17               0x6C     // Keyboard F17
#define KEY_F18               0x6D     // Keyboard F18
#define KEY_F19               0x6E     // Keyboard F19
#define KEY_F20               0x6F     // Keyboard F20
#define KEY_F21               0x70     // Keyboard F21
#define KEY_F22               0x71     // Keyboard F22
#define KEY_F23               0x72     // Keyboard F23
#define KEY_F24               0x73     // Keyboard F24
#define KEY_EXECUTE           0x74     // Keyboard Execute
#define KEY_HELP              0x75     // Keyboard Help
#define KEY_MENU              0x76     // Keyboard Menu
#define KEY_SELECT            0x77     // Keyboard Select
#define KEY_STOP              0x78     // Keyboard Stop
#define KEY_AGAIN             0x79     // Keyboard Again
#define KEY_UNDO              0x7A     // Keyboard Undo
#define KEY_CUT               0x7B     // Keyboard Cut
#define KEY_COPY              0x7C     // Keyboard Copy
#define KEY_PASTE             0x7D     // Keyboard Paste
#define KEY_FIND              0x7E     // Keyboard Find
#define KEY_MUTE              0x7F     // Keyboard Mute
#define KEY_VOLUME_UP         0x80     // Keyboard Volume Up
#define KEY_VOLUME_DOWN       0x81     // Keyboard Volume Down
#define KEY_LOCK_CAPS_LOCK    0x82     // Keyboard Locking Caps Lock
#define KEY_LOCK_NUM_LOCK     0x83     // Keyboard Locking Num Lock
#define KEY_LOCK_SCR_LOCK     0x84     // Keyboard Locking Scroll Lock
#define KEY_KPAD_COMMA        0x85     // Keypad Comma
#define KEY_EQUAL_SIGN        0x86     // Keypad Equal Sign
#define KEY_INTL1             0x87     // Keyboard International1
#define KEY_INTL2             0x88     // Keyboard International2
#define KEY_INTL3             0x89     // Keyboard International3
#define KEY_INTL4             0x8A     // Keyboard International4
#define KEY_INTL5             0x8B     // Keyboard International5
#define KEY_INTL6             0x8C     // Keyboard International6
#define KEY_INTL7             0x8D     // Keyboard International7
#define KEY_INTL8             0x8E     // Keyboard International8
#define KEY_INTL9             0x8F     // Keyboard International9
#define KEY_LANG1             0x90     // Keyboard LANG1
#define KEY_LANG2             0x91     // Keyboard LANG2
#define KEY_LANG3             0x92     // Keyboard LANG3
#define KEY_LANG4             0x93     // Keyboard LANG4
#define KEY_LANG5             0x94     // Keyboard LANG5
#define KEY_LANG6             0x95     // Keyboard LANG6
#define KEY_LANG7             0x96     // Keyboard LANG7
#define KEY_LANG8             0x97     // Keyboard LANG8
#define KEY_LANG9             0x98     // Keyboard LANG9
#define KEY_ALT_ERASE         0x99     // Keyboard Alternate Erase
#define KEY_SYSREQ            0x9A     // Keyboard SysReq/Attention
#define KEY_CANCEL            0x9B     // Keyboard Cancel
#define KEY_CLEAR             0x9C     // Keyboard Clear
#define KEY_PRIOR             0x9D     // Keyboard Prior
#define KEY_RETURN            0x9E     // Keyboard Return
#define KEY_SEPARATOR         0x9F     // Keyboard Separator
#define KEY_OUT               0xA0     // Keyboard Out
#define KEY_OPER              0xA1     // Keyboard Oper
#define KEY_CLEAR             0xA2     // Keyboard Clear/Again
#define KEY_CRSEL             0xA3     // Keyboard CrSel/Props
#define KEY_EXSEL             0xA4     // Keyboard ExSel
                                       // Keys 0xA5 to 0xAF reserved
#define KEY_KPAD_00           0xB0     // Keypad 00
#define KEY_KPAD_000          0xB1     // Keypad 000
#define KEY_THOUSANDS_SEP     0xB2     // Thousands Separator
#define KEY_DECIMAL_SEP       0xB3     // Decimal Separator
#define KEY_CURRENCY          0xB4     // Currency Unit
#define KEY_SUB_CURRENCY      0xB5     // Currency Sub-unit
#define KEY_KPAD_LEFT_PAREN   0xB6     // Keypad (
#define KEY_KPAD_RIGHT_PAREN  0xB7     // Keypad )
#define KEY_KPAD_LEFT_BRACE   0xB8     // Keypad {
#define KEY_KPAD_RIGHT_BRACE  0xB9     // Keypad }
#define KEY_KPAD_TAB          0xBA     // Keypad Tab
#define KEY_KPAD_BACKSPACE    0xBB     // Keypad Backspace
#define KEY_KPAD_A            0xBC     // Keypad A
#define KEY_KPAD_B            0xBD     // Keypad B
#define KEY_KPAD_C            0xBE     // Keypad C
#define KEY_KPAD_D            0xBF     // Keypad D
#define KEY_KPAD_E            0xC0     // Keypad E
#define KEY_KPAD_F            0xC1     // Keypad F
#define KEY_KPAD_XOR          0xC2     // Keypad XOR
#define KEY_KPAD_CARET        0xC3     // Keypad ^
#define KEY_KPAD_PERCENT      0xC4     // Keypad %
#define KEY_KPAD_LESS_THAN    0xC5     // Keypad <
#define KEY_KPAD_GREAT_THAN   0xC6     // Keypad >
#define KEY_KPAD_AND          0xC7     // Keypad &
#define KEY_KPAD_DBL_AND      0xC8     // Keypad &&
#define KEY_KPAD_OR           0xC9     // Keypad |
#define KEY_KPAD_DBL_OR       0xCA     // Keypad ||
#define KEY_KPAD_COLON        0xCB     // Keypad :
#define KEY_KPAD_HASH         0xCC     // Keypad #
#define KEY_KPAD_SPACE        0xCD     // Keypad Space
#define KEY_KPAD_AT           0xCE     // Keypad @
#define KEY_KPAD_EXCLAMATION  0xCF     // Keypad !
#define KEY_KPAD_MEM_STORE    0xD0     // Keypad Memory Store
#define KEY_KPAD_MEM_RECALL   0xD1     // Keypad Memory Recall
#define KEY_KPAD_MEM_CLEAR    0xD2     // Keypad Memory Clear
#define KEY_KPAD_MEM_ADD      0xD3     // Keypad Memory Add
#define KEY_KPAD_MEM_SUB      0xD4     // Keypad Memory Subtract
#define KEY_KPAD_MEM_MULTIPLY 0xD5     // Keypad Memory Multiply
#define KEY_KPAD_MEM_DIVIDE   0xD6     // Keypad Memory Divide
#define KEY_PLUS_MINUS        0xD7     // Keypad +/-
#define KEY_CLEAR             0xD8     // Keypad Clear
#define KEY_CLEAR_ENTRY       0xD9     // Keypad Clear Entry
#define KEY_BINARY            0xDA     // Keypad Binary
#define KEY_OCTAL             0xDB     // Keypad Octal
#define KEY_DECIMAL           0xDC     // Keypad Decimal
#define KEY_HEXADECIMAL       0xDD     // Keypad Hexadecimal
                                       // Keys 0xDE to 0xDF reserved
#define KEY_L_CONTROL         0xE0     // Keyboard LeftControl
#define KEY_L_SHIFT           0xE1     // Keyboard LeftShift
#define KEY_L_ALT             0xE2     // Keyboard LeftAlt
#define KEY_L_GUI             0xE3     // Keyboard Left GUI
#define KEY_R_CONTROL         0xE4     // Keyboard RightControl
#define KEY_R_SHIFT           0xE5     // Keyboard RightShift
#define KEY_R_ALT             0xE6     // Keyboard RightAlt
#define KEY_R_GUI             0xE7     // Keyboard Right GUI

SoftwareSerial mySerial(rxPin,txPin); // RX = rxPin, TX  = txPin

const static uint8_t HIDTable[] =  {
  0x00, // 0
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2A,0x00,0x00, // 10
  0x00,0x00,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 20
  0x00,0x00,0x00,0x00,0x00,0x00,0x29,0x00,0x00,0x00, // 30
  0x00,0x2c,0x1e,0x1f,0x20,0x21,0x22,0x23,0x2d,0x25,
  0x26,0x30,0x30,0x36,0x38,0x37,0x24,0x27,0x1e,0x1f,
  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x37,0x36,0x64,
  0x27,0x64,0x2d,0x1f,0x04,0x05,0x06,0x07,0x08,0x09,
  0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,
  0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,
  0x2f,0x35,0x30,0x2f,0x38,0x2f,0x04,0x05,0x06,0x07,
  0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,
  0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
  0x1c,0x1d,0x34,0x1e,0x32,0x21,0x2c // 127
};

const static uint8_t MODTable[] =  {
  0x00, // 0
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 10
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 20
  0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00, // 30
  0x00,0x00,0x02,0x02,0x40,0x02,0x02,0x02,0x00,0x02,
  0x02,0x02,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x00,
  0x02,0x02,0x02,0x40,0x02,0x02,0x02,0x02,0x02,0x02,
  0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
  0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
  0x40,0x40,0x40,0x02,0x02,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x40,0x40,0x40,0x40,0x00 // 127
};


uint8_t key ;
uint8_t mod ;

String last = "";
int defaultDelay = 0;

String bufferStr = "";


void setup() {
  
  Serial.begin(9600); //Start Serial
  mySerial.begin(9600); //Start mySerial    
  // Transmition PIN (SoftSerial)
   // pinMode(rxPin, INPUT);
   // pinMode(txPin, OUTPUT);
  
  SPIFFS.begin();
  
  Serial.flush();
  mySerial.flush();
  delay(1000);
  Serial.println("\nDucky started");

  while (mySerial.available() > 0 )  { delay(0);   } //  Espera señal de ATTINY de que está listo para recibir datos
    
}

 

void loop(){

     f = SPIFFS.open("/test.txt", "r");
     if (!f) {
    Serial.println("file open failed");
    ESP.restart();}
    Serial.println("\n\n\n====== Reading SPIFFS file =======");
 
       
    while(f.available()) {

    bufferStr=f.readStringUntil('\n');

    bufferStr.replace("\r","\n");
    bufferStr.replace("\n\n","\n");
    bufferStr.replace("\n ","\n");
      
      int latest_return = bufferStr.indexOf("\n");
      if(latest_return == -1){
         Serial.println("\nrun: "+bufferStr);
        Line(bufferStr);
        bufferStr = "";
      } else{
         Serial.println("\nrun: '"+bufferStr.substring(0, latest_return)+"'");
        Line(bufferStr.substring(0, latest_return));
        last=bufferStr.substring(0, latest_return);
        bufferStr = bufferStr.substring(latest_return + 1);
      }
    }
    
    bufferStr = "";
        
 
} // LOOP

void DigiKeyboard(uint8_t key ){

      
      if(key > 0 || mod > 0 ) {
        
      mySerial.flush();
      
      mySerial.write(key);
      mySerial.write(mod);
      
      while (mySerial.available() != 2)  { delay(0);   } // Espera confirmacion del ATTINY

      // Serial.print(key,HEX);Serial.print(",");
      // Serial.print(mod,HEX);Serial.print(",");

                      
///////////////////////////if(key == 0 ) delay(mod);
                      
      
      
                                } 
}// THE END -0x5d




void Line(String _line)
{
  int firstSpace = _line.indexOf(" ",1);
  //Serial.println(_line.substring(0,firstSpace));
  if(firstSpace == -1) Press(_line);
    else if(_line.substring(0,firstSpace) == "STRING"){
    
    for(int i=firstSpace+1;i<_line.length();i++) { 
      
      
      
      Serial.print(_line[i],DEC);Serial.print(">");Serial.print(HIDTable[_line[i]],HEX);Serial.print(_line[i]);
      
      mod = MODTable[_line[i]];
   
      key = HIDTable[_line[i]];
      
      DigiKeyboard(key);
  
      
      }

    
  }
  else if(_line.substring(0,firstSpace) == "DELAY"){
    int delaytime = _line.substring(firstSpace + 1).toInt();
    
while (delaytime > 0) {
                                               
if (delaytime > 255) {
        mod = 255; DigiKeyboard(0);
        delaytime = delaytime - 255;
} else {
        mod = delaytime; DigiKeyboard(0);
        delaytime = 0;
}
                      }
    
    mod = delaytime; DigiKeyboard(0);
  }
  else if(_line.substring(0,firstSpace) == "DEFAULTDE }LAY") defaultDelay = _line.substring(firstSpace + 1).toInt();
  else if(_line.substring(0,firstSpace) == "REM"){} //nothing :/
  else if(_line.substring(0,firstSpace) == "REPLAY") {
    int replaynum = _line.substring(firstSpace + 1).toInt();
    while(replaynum)
    {
      Line(last);
      --replaynum;
    }
  } else{
      String remain = _line;

      while(remain.length() > 0){
        int latest_space = remain.indexOf(" ");
        if (latest_space == -1){
          Press(remain);
          remain = "";
        }
        else{
          Press(remain.substring(0, latest_space));
          remain = remain.substring(latest_space + 1);
        }
        delay(5);
      } }
  }


void Press(String b){

  //Serial.println(b);

  if(b.length() == 1) DigiKeyboard(char(b[0]));
  else if (b.equals("ENTER")) DigiKeyboard(0x28);
  else if (b.equals("CTRL")) mod = (MOD_CONTROL_LEFT);
  else if (b.equals("SHIFT")) mod = (MOD_SHIFT_LEFT);
  else if (b.equals("ALT")) mod = (MOD_ALT_LEFT);
  else if (b.equals("GUI")) mod = (MOD_GUI_LEFT);
  else if (b.equals("UP") || b.equals("UPARROW")) DigiKeyboard(KEY_ARROW_UP);
  else if (b.equals("DOWN") || b.equals("DOWNARROW")) DigiKeyboard(KEY_ARROW_DOWN);
  else if (b.equals("LEFT") || b.equals("LEFTARROW")) DigiKeyboard(KEY_ARROW_LEFT);
  else if (b.equals("RIGHT") || b.equals("RIGHTARROW")) DigiKeyboard(KEY_ARROW_RIGHT);
  else if (b.equals("DELETE")) DigiKeyboard(KEY_DELETE);
  else if (b.equals("PAGEUP")) DigiKeyboard(KEY_PAGE_UP);
  else if (b.equals("PAGEDOWN")) DigiKeyboard(KEY_PAGE_DOWN);
  else if (b.equals("HOME")) DigiKeyboard(KEY_HOME);
  else if (b.equals("ESC")) DigiKeyboard(KEY_ESCAPE);
  else if (b.equals("BACKSPACE")) DigiKeyboard(KEY_DELETE);
  else if (b.equals("INSERT")) DigiKeyboard(KEY_INSERT);
  else if (b.equals("TAB")) DigiKeyboard(KEY_TAB);
  else if (b.equals("END")) DigiKeyboard(KEY_END);
  else if (b.equals("CAPSLOCK")) DigiKeyboard(KEY_CAPS_LOCK);
  else if (b.equals("F1")) DigiKeyboard(KEY_F1);
  else if (b.equals("F2")) DigiKeyboard(KEY_F2);
  else if (b.equals("F3")) DigiKeyboard(KEY_F3);
  else if (b.equals("F4")) DigiKeyboard(KEY_F4);
  else if (b.equals("F5")) DigiKeyboard(KEY_F5);
  else if (b.equals("F6")) DigiKeyboard(KEY_F6);
  else if (b.equals("F7")) DigiKeyboard(KEY_F7);
  else if (b.equals("F8")) DigiKeyboard(KEY_F8);
  else if (b.equals("F9")) DigiKeyboard(KEY_F9);
  else if (b.equals("F10")) DigiKeyboard(KEY_F10);
  else if (b.equals("F11")) DigiKeyboard(KEY_F11);
  else if (b.equals("F12")) DigiKeyboard(KEY_F12);
  else if (b.equals("SPACE")) DigiKeyboard(' ');
  //else Serial.println("not found :'"+b+"'("+String(b.length())+")");
}
