#define kbd_es_es
#include "DigiKeyboard.h"
#include <PS2Keyboard.h>

const int DataPin = 0;//8;//pin is 0
const int IRQpin =  2;//1;//not a pin but the interupt, pin is 2

uint8_t m = 0;
uint8_t c;  

PS2Keyboard keyboard;

void setup()
{
delay(1000);
keyboard.begin(DataPin,IRQpin );
DigiKeyboard.sendKeyStroke(0);
}

int sig()
{
uint8_t c;
if (keyboard.available()) c = keyboard.read(); 
return c;
}


void loop()
{
c = sig();

//DigiKeyboard.print(c,HEX);


if ( c==101 ){ m = m | 1; c = 0; }
if ( c==102 ){ m = m | 2; c = 0; }
if ( c==104 ){ m = m | 4; c = 0; }
if ( c==140 ){ m = m | 0x40 ; c = 0; }

if ( c==180 ){ m = m ^ 0x40 ; c = 0; }

if ( c==0xf0 ) { 
  c = sig(); 
  if ( c==101 ) m = m ^ 1;
  if ( c==102 ) m = m ^ 2;
  if ( c==104 ) m = m ^ 4;

  c = 0; 
  }
DigiKeyboard.sendKeyStroke(c,m);
}




