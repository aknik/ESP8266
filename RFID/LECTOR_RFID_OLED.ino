// https://toschprod.wordpress.com/2013/08/05/rfid-spoofer/
// http://www.priority1design.com.au/fdx-b_animal_identification_protocol.html
// http://www.priority1design.com.au/em4100_protocol.html

#define LED 2 
#define RF  D1

int bits ;
int unos;
int EM4100[64];
int bit_set;
int error;
String Res;

static inline int32_t asm_ccount(void) {int32_t r; asm volatile ("rsr %0, ccount" : "=r"(r)); return r; }
static inline void asm_nop() { asm volatile ("nop"); }
 
#include <ESP8266WiFi.h>
#include <SSD1306.h>

SSD1306  display(0x3c, D5, D3);//0x3d , SDA, SCL

void setup() {

  display.init();
  display.flipScreenVertically();   
  display.display();
  
  WiFi.mode( WIFI_OFF );
  
  Serial.begin(115200);
  pinMode(RF, INPUT);
  pinMode(LED, OUTPUT);

}

void loop() {
  
   while (unos < 9) { 

    int a = (digitalRead(RF));
    delayMicroseconds(255);
    for (int k=0; k<13; k++) asm_nop(); //fine tuning
    
    int b = (digitalRead(RF));
    delayMicroseconds(255); 
    for (int k=0; k<13; k++) asm_nop(); //fine tuning

 if (a == 0 && b == 1) { 
    
    //Serial.print("1"); 
    //EM4100[unos] = 1;
    unos = unos + 1 ; 
    } else { break;}

}

 if (unos == 9) {   


    for(int i = 9; i < 64; i++) { 
      
    int a = (digitalRead(RF));
    delayMicroseconds(255);
    for (int k=0; k<13; k++) asm_nop(); //fine tuning
    
    int b = (digitalRead(RF));
    delayMicroseconds(255); 
    for (int k=0; k<13; k++) asm_nop(); //fine tuning


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
    Res = Res + String(recivedID, HEX);
                                  }
//Serial.print(Res);
//Serial.println(""); 


uint32_t ID = strtoll( &Res[2], NULL, 16);
              Res = Res.substring(0, 2);
uint32_t VENDOR = strtoll( &Res[0], NULL, 16);

display.clear();
display.setTextAlignment(TEXT_ALIGN_CENTER);
display.setFont(ArialMT_Plain_16);
display.drawStringMaxWidth(64, 15, 128, String(VENDOR, DEC));
display.drawStringMaxWidth(64, 33, 128, String(ID, DEC));
display.display();

Serial.print("V: ");
Serial.print(VENDOR);
Serial.print(" ID: ");
Serial.println(ID);

digitalWrite(LED,LOW); 
                            } 
             

} // if
  else { digitalWrite(LED,HIGH); }
  
     
    unos = 0 ; 
    bits = 0; 
    error = 0;
    Res = "";
    delay(0);




} // loop


