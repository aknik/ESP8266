#include <SoftSerial_INT0.h>
#include "DigiKeyboard.h"

#define P_RX 2                        // Reception PIN (SoftSerial)
#define P_TX 1                        // Transmition PIN (SoftSerial)
#define ExternSerial Serial1

String bufferStr = "";
String last = "";

int defaultDelay = 0;
uint8_t key ;
uint8_t mod ;
      
SoftSerial ExternSerial(P_RX, P_TX); 


void setup(){

ExternSerial.begin(9600);
// initialize the digital pin as an output.


  DigiKeyboard.sendKeyStroke(0);
  delay(500);
  DigiKeyboard.println("");
  DigiKeyboard.println("clear");


}


void loop() {
  

  if (mySerial.overflow()) {
   DigiKeyboard.println("SoftwareSerial overflow!");

  if (mySerial.available() >= 2) { 
    
    key = mySerial.read();
    mod = mySerial.read();

      if (key == 0) //delay (a delay>255 is split into a sequence of delays)
      {
        DigiKeyboard.delay(mod);
        
      }
      else 
       

    DigiKeyboard.sendKeyStroke(key,mod);

}
///////








