#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>

extern "C"
{
#include "user_interface.h"
}

const int CLK = D6; //Set the CLK pin connection to the display
const int DIO = D5; //Set the DIO pin connection to the display

uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };

const byte digit_pattern[16] =
{
  B00111111,  // 0
  B00000110,  // 1
  B01011011,  // 2
  B01001111,  // 3
  B01100110,  // 4
  B01101101,  // 5
  B01111101,  // 6
  B00000111,  // 7
  B01111111,  // 8
  B01101111,  // 9
  B01110111,  // A
  B01111100,  // b
  B00111001,  // C
  B01011110,  // d
  B01111001,  // E
  B01110001   // F
};

int ledState = LOW;
unsigned long previousMillis = 0; 
const long interval = 1000;  
int numCounter = 0;
int minutos = 0;
int hora = 0;
int TimeZone = 2;

WiFiServer server(80);

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.


 
void setup () {

  display.setBrightness(0x0a); //set the diplay to maximum brightness 0x0a
  
  Serial.begin(115200);
  WifiCon();
  
}

void loop () {

if (minute() == 0 && second() < 6 ) { WifiCon(); }
//Serial.println(minute()%10);
//printhora();
LCDprint();
// delay(500);


}

void printhora() {
Serial.print(hour());
Serial.print(':');
if (minute() < 10 ) { Serial.print('0'); }
Serial.print(minute()); 
Serial.print(':'); 
if (second() < 10 ) { Serial.print('0'); }
Serial.println(second());   

if (day() < 10 ) { Serial.print('0'); }
Serial.print(day());
if (month() < 10 ) { Serial.print('0'); }
Serial.print(month());
Serial.println(year());
} 

int GetMonthIndex(char* buff)
{
static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
int month = (strstr(month_names, buff)-month_names)/3+1;
return month;
}

void GetTime(int TZ)
{
String TiMeS,TMnow,MyNtP;                                 // The current TimeStamp from Google....
int tIMeh,tIMem,tIMes,newt,newt1,inChar; 
int tIMed,tIMemo,tIMey;                                                // Integer values from Gooogle time reply
String DsTable;                                           // Discarded parts if the Strings......
String theDate,theDate1,theDateTr;              // The current TimeStamp Date from Google....
char mes[3];

WiFiClient client;
while (!!!client.connect("google.com", 80)) {

}
client.print("HEAD / HTTP/1.1\r\n\r\n");
while(!!!client.available()) {
   yield();
}
while(client.available()){
client.readStringUntil('\n');
 theDate1 = client.readStringUntil('\r'); //Date: Tue, 10 Nov 2015 19:55:38 GMT
  if (theDate1.startsWith("Date:")) {
    TiMeS=theDate1;
    theDate = theDate1.substring(6,23);
        client.flush();
        client.stop();             
        }
}

inChar = TiMeS.charAt(30);
newt=inChar-48;             // Convert Numerical Char to int.....
inChar = TiMeS.charAt(29);
newt1=inChar-48;
tIMes=(newt1*10)+newt;

inChar = TiMeS.charAt(27);
newt=inChar-48;
inChar = TiMeS.charAt(26);
newt1=inChar-48;
tIMem=(newt1*10)+newt;

inChar = TiMeS.charAt(24);
newt=inChar-48;
inChar = TiMeS.charAt(23);
newt1=inChar-48;
tIMeh=(newt1*10)+newt;

inChar = TiMeS.charAt(12);
newt=inChar-48;             // Convert Numerical Char to int.....
inChar = TiMeS.charAt(11);
newt1=inChar-48;
tIMed=(newt1*10)+newt;


inChar = TiMeS.charAt(21);
newt=inChar-48;             // Convert Numerical Char to int.....
inChar = TiMeS.charAt(20);
newt1=inChar-48;
tIMey=2000+((newt1*10)+newt);

mes[0] = TiMeS.charAt(14);
mes[1] = TiMeS.charAt(15);
mes[2] = TiMeS.charAt(16);


tIMemo = GetMonthIndex(mes);

setTime(tIMeh,tIMem,tIMes,tIMed,tIMemo,tIMey); 
time_t t = now();
setTime(t + TimeZone*3600 + 1);

}

void LCDprint() {
  
hora = hour();
minutos = minute();

data[0] = digit_pattern[hora / 10];
data[1] = digit_pattern[hora % 10 ];
data[2] = digit_pattern[minutos / 10];
data[3] = digit_pattern[minutos % 10];

unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;  
    } else {
      ledState = LOW;
           }
                                                   }
    
  if (ledState == HIGH) { data[1] = data[1] | B10000000 ; }
  display.setSegments(data);  
  yield();

}

void WifiCon() {

const char *ssid     = "WLAN_XXXX";
const char *password = ".999999999."; 

IPAddress ip( 192, 168, 2, 88 );
IPAddress gateway( 192, 168, 2, 1 );
IPAddress subnet( 255, 255, 255, 0 );

  WiFi.forceSleepWake();
  delay( 1 );
  // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
  WiFi.persistent( false );
  WiFi.mode( WIFI_STA );
  WiFi.config( ip, gateway, subnet );
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED)
  {
     delay(100);
  }

  Serial.print("\n\nConexión Wifi.\nIP address: ");
  Serial.println(WiFi.localIP()); 
  
  GetTime(TimeZone);
  printhora();
  
WiFi.disconnect( true );
WiFi.mode( WIFI_OFF );
WiFi.forceSleepBegin();
delay( 1 );

// WAKE_RF_DISABLED to keep the WiFi radio disabled when we wake up
// ESP.deepSleep( SLEEPTIME, WAKE_RF_DISABLED );

  }




