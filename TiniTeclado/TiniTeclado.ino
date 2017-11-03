#define kbd_es_es
#include "DigiKeyboard.h"
#include <PS2Keyboard.h>

const int DataPin = 0;//8;//pin is 0
const int IRQpin =  2;//1;//not a pin but the interupt, pin is 2

uint8_t m = 0;

PS2Keyboard keyboard;

void setup()
{
delay(1000);
keyboard.begin(DataPin,IRQpin );
DigiKeyboard.sendKeyStroke(0);
}

void loop()
{
  
if (keyboard.available()) {
uint8_t c = keyboard.read();

if ( c==102 ){ m = 2; c = 0; }
if ( c==104 ){ m = 4; c = 0; }


if ( c==0xE0 ) { 
  
  if (keyboard.available()) c = keyboard.read(); 
  if ( c==104 ){ m = 0x40;  }
  
  if (keyboard.available()) c = keyboard.read();
  
  if ( c==0xf0 ) { 
  
  if (keyboard.available()) c = keyboard.read(); 
  
  if ( c==104 ) m = 0; c = 0;
  
 
                  }
  
  
  }
  


if ( c==0xf0 ) { 
  
  if (keyboard.available()) c = keyboard.read(); 
  
  if ( c==102 ) m = 0;
  if ( c==104 ) m = 0;
  
  c = 0; 
  }

 //if ( c != 0 )DigiKeyboard.print(c,HEX);
DigiKeyboard.sendKeyStroke(c,m);

  }

}


