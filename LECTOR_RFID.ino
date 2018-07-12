// https://toschprod.wordpress.com/2013/08/05/rfid-spoofer/

const int Led = D4;
int bits ;
int unos;
int EM4100[65];
int bit_set;
int error;


#include <ESP8266WiFi.h>


void setup() {
  WiFi.mode( WIFI_OFF );
  
  Serial.begin(115200);
  pinMode(5, INPUT);
  pinMode(Led, OUTPUT);

}

void loop() {
  
   while (unos < 9) { 

    int a = (digitalRead(5) );
    delayMicroseconds(256);
    
    int b = (digitalRead(5)  );
    delayMicroseconds(256); 

 if (a == 0 && b == 1) { 
    
    //Serial.print("1"); 
    EM4100[unos] = 1;
    unos = unos + 1 ; 
    } else { break;}

}

 if (unos == 9) {   


    for(int i = 9; i < 64; i++) { 
      
    int a = (digitalRead(5) );
    delayMicroseconds(255);
    
    int b = (digitalRead(5)  );
    delayMicroseconds(255); 


if (a == 1 && b == 0) { 
    
    //Serial.print("0"); 
    unos = 0 ; 
    EM4100[i] = 0 ; 
    bits = bits + 1 ; 
    }
  
  if (a == 0 && b == 1) { 
    
    //Serial.print("1"); 
    unos = unos + 1; 
    EM4100[i] = 1 ; 
    } 
    
    }// for netxt

    //for(int i = 9; i < 64; i++) { Serial.print(EM4100[i]); }
    //Serial.println("");
    
    
//int recivedID = EM4100[9] | (EM4100[10] << 1) | (EM4100[11] << 2) | (EM4100[12] << 3) | (EM4100[14] << 4) | (EM4100[15] << 5) | (EM4100[16] << 6) | (EM4100[17] << 7);

//Serial.println(recivedID,HEX);

 // CRC ROW
  for(int r = 0; r < 10; r++) {
    // Calculate number of 1 per row 
    for(int i = 0; i < 4; i++) {
      if(EM4100[(r*5)+i+9]) { 
        bit_set++;
      } 
    }

    // If number of 1 is even, parity = 1 else 0
    if((bit_set % 2) != 0) { 
      if (EM4100[(5*r)+4+9] != 1){ error = 1; };
    } else { 
       if (EM4100[(5*r)+4+9] != 0){ error = 1; };
    }

    bit_set = 0; 
  }
  
  // CRC COLUMN
  bit_set = 0;

  for(int c = 0; c < 4; c++) {

    // Calculate number of 1 per column 
    for(int i = 0; i < 10; i++) {

      if(EM4100[c+(i*5)+9]) { 
        bit_set++;
      }
    }

    // If number of 1 is even, parity = 1 else 0 
    if((bit_set % 2) != 0) {
      if (EM4100[50+9+c] != 1) { error = 1; };
    } else {
      if (EM4100[50+9+c] != 0) { error = 1; };
    }

    bit_set = 0; 
  }


if (error != 1){ 
    for(int i = 9; i < 59; i=i+5) { int recivedID = EM4100[i+3] | (EM4100[i+2] << 1) | (EM4100[i+1] << 2) | (EM4100[i] << 3)  ; 
    Serial.print(recivedID,HEX); }
  Serial.println(""); 
              }




} // if
    
    unos = 0 ; 
    bits = 0; 
    error = 0;

   
} // loop


