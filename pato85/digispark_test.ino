#include <SoftSerial_INT0.h>
#include "DigiKeyboard.h"

#define P_RX 2                        // Reception PIN (SoftSerial)
#define P_TX 1                        // Transmition PIN (SoftSerial)

uint8_t key ;
uint8_t mod ;

char* texto;
      
SoftSerial ExternSerial(P_RX, P_TX); 


void setup(){

ExternSerial.begin(9600);
// initialize the digital pin as an output.

  DigiKeyboard.update();
  DigiKeyboard.sendKeyStroke(0);
  delay(500);
  //DigiKeyboard.println("");
  //DigiKeyboard.println("clear");


}


void loop() {

  DigiKeyboard.update();

  if (ExternSerial.overflow()) {
   DigiKeyboard.println("SoftwareSerial overflow!");}

  if (ExternSerial.available() >= 2) { 
    
    key = ExternSerial.read();
    mod = ExternSerial.read();


while (mod == 0xff) { 
    texto = texto + key; 
    key = ExternSerial.read();
    mod = ExternSerial.read();
    }

if (sizeof(texto) > 0) { 
  
  DigiKeyboard.print(texto);
  texto = "";
  key = 0;
    
}



      if (key == 0) //delay (a delay>255 is split into a sequence of delays)
      {
        DigiKeyboard.delay(mod);
        
      }
      else 
    
    DigiKeyboard.sendKeyStroke(key,mod);
    
delay(0);

                                    }
} 









