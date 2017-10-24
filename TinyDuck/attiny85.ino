#include <SoftSerial_INT0.h>
#include "DigiKeyboard.h"

#define P_RX 2                        // Reception PIN (SoftSerial)
#define P_TX 1                        // Transmition PIN (SoftSerial)

uint8_t key ;
uint8_t mod ;
      
SoftSerial mySerial(P_RX, P_TX); 


void setup(){

mySerial.begin(9600);
DigiKeyboard.sendKeyStroke(0);

}

void loop() {
  
  if (mySerial.available() == 2) { 
    
    key = mySerial.read();
    mod = mySerial.read();

      if (key == 0) //delay (a delay>255 is split into a sequence of delays)
      {
        DigiKeyboard.delay(mod);
        
      }
      else 
       

    DigiKeyboard.sendKeyStroke(key,mod);

  }
