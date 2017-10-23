#include <SoftSerial_INT0.h>
#include "DigiKeyboard.h"

#define P_RX 2                        // Reception PIN (SoftSerial)
#define P_TX 1                        // Transmition PIN (SoftSerial)

SoftSerial mySerial(P_RX, P_TX); 


void setup(){

mySerial.begin(9600);
DigiKeyboard.println("TEST");

}

void loop() {
  char receivedLetter = '?'; // if empty show question mark
  
  if (mySerial.available() > 0) { //Check if serial is avaliable, if this check is not done you will read 'ÿ'
    receivedLetter = mySerial.read(); //read the char received. This function returns -1 if there is nothing to read.
    // Serial.print("LETRA ");Serial.print(receivedLetter); Serial.println(" RECEBIDA"); // Write on serial monitor which letter was received. 
    DigiKeyboard.println(receivedLetter);
  }
  
} //end of loop
