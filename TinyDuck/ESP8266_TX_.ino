#include <SoftwareSerial.h>

#define D7 13
#define D8 15

SoftwareSerial mySerial(14,12); // RX = D7, TX  = D8


void setup() {
  Serial.begin(9600); //Start Serial
  mySerial.begin(9600); //Start mySerial

  Serial.println("\nSoftware serial test started");

  for (char ch = ' '; ch <= 'z'; ch++) {
    mySerial.write(ch);
  }
  mySerial.println("");


  
} // end of setup


void loop() {

  char letter = random(65,91); //Numbers between 65 and 90, that are the upper case letters in ASCII table (https://www.arduino.cc/en/Reference/ASCIIchart).

  mySerial.write(letter); // write the byte to the serial so other NodeMCU can receive it.

  Serial.print("LETRA ");Serial.print(letter); Serial.println(" ENVIADA"); // Write on serial monitor which letter was sent.
  
  delay(1000); // Wait for a sec.
  
} // end of loop
