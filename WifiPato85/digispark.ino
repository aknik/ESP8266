#include <SoftSerial_INT0.h>
#include "DigiKeyboard.h"

#define P_RX 2                        // Reception PIN (SoftSerial)
#define P_TX 1                        // Transmition PIN (SoftSerial)

uint8_t key ;
uint8_t mod ;

   
SoftSerial ExternSerial(P_RX, P_TX); 

void setup(){

ExternSerial.begin(9600);
DigiKeyboard.sendKeyStroke(0);
ExternSerial.flush();

ExternSerial.write(0x78); // Señal de ready al ESP
ExternSerial.write(0x70); // Señal de ready al ESP

}


void loop() {


if (ExternSerial.overflow()) { DigiKeyboard.println("SoftwareSerial overflow!");}

if (ExternSerial.available() == 2 ) { 
    
    key = ExternSerial.read();
    mod = ExternSerial.read();
    
    ExternSerial.write(key);
    ExternSerial.write(mod);
    
    if (key == 0)  { DigiKeyboard.delay(mod); }
      else 
    
    DigiKeyboard.sendKeyStroke(key,mod);
    DigiKeyboard.delay(20);
    DigiKeyboard.update();
 }
delay(0);
                                  
} 









