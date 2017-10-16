#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <Timezone.h>

extern "C"
{
#include "user_interface.h"
}

//Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);

//Australia Eastern Time Zone (Sydney, Melbourne)
TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    //UTC + 11 hours
TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    //UTC + 10 hours
Timezone ausET(aEDT, aEST);

//US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);
 
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

void GetTime()
{
String TiMeS,TMnow,MyNtP;                                 // The current TimeStamp from Google....
int tIMeh,tIMem,tIMes,newt,newt1,inChar; 
int tIMed,tIMemo,tIMey;                                                // Integer values from Gooogle time reply
String DsTable;                                           // Discarded parts if the Strings......
String theDate,theDate1,theDateTr;              // The current TimeStamp Date from Google....
char mes[4];

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

Serial.println(TiMeS);
// Date: Mon, 16 Oct 2017 10:32:34 GMT


 
inChar = TiMeS.charAt(30);
newt=inChar-48;             
inChar = TiMeS.charAt(29);
newt1=inChar-48;
tIMes=(newt1*10)+newt;      // SEGUNDOS

inChar = TiMeS.charAt(27);
newt=inChar-48;
inChar = TiMeS.charAt(26);
newt1=inChar-48;
tIMem=(newt1*10)+newt;      // MINUTOS

inChar = TiMeS.charAt(24);
newt=inChar-48;
inChar = TiMeS.charAt(23);
newt1=inChar-48;
tIMeh=(newt1*10)+newt;      //  HORAS



// Date: Mon, 16 Oct 2017 10:32:34 GMT
// Serial.println(TiMeS.substring(23,31));


inChar = TiMeS.charAt(12);
newt=inChar-48;             
inChar = TiMeS.charAt(11);
newt1=inChar-48;
tIMed=(newt1*10)+newt;      //  DIA MES

Serial.println(tIMed);

inChar = TiMeS.charAt(21);
newt=inChar-48;             // Convert Numerical Char to int.....
inChar = TiMeS.charAt(20);
newt1=inChar-48;
tIMey=2000+((newt1*10)+newt);

mes[0] = TiMeS.charAt(14);
mes[1] = TiMeS.charAt(15);
mes[2] = TiMeS.charAt(16);
mes[3] = 0;  //  included the null character (written '\0') ourselves. 

Serial.println(mes);
tIMemo = GetMonthIndex(mes);
Serial.println(tIMemo);

// setTime(hr,min,sec,day,month,yr);
setTime(tIMeh,tIMem,tIMes,tIMed,tIMemo,tIMey); 

    unsigned long epoch = now();
    
    TimeChangeRule *tcr;
    time_t utc;
    utc = epoch;
    
    setTime(CE.toLocal(utc, &tcr));

    printTime(utc, "UTC", "Universal Coordinated Time");
    printTime(CE.toLocal(utc, &tcr), tcr -> abbrev, "Madrid");
    printTime(usET.toLocal(utc, &tcr), tcr -> abbrev, "New York");
    printTime(ausET.toLocal(utc, &tcr), tcr -> abbrev, "Sydney");
    Serial.println("");

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


//Function to print time with time zone
void printTime(time_t t, char *tz, char *loc)
{
  sPrintI00(hour(t));
  sPrintDigits(minute(t));
  sPrintDigits(second(t));
  Serial.print(' ');
  Serial.print(dayShortStr(weekday(t)));
  Serial.print(' ');
  sPrintI00(day(t));
  Serial.print(' ');
  Serial.print(monthShortStr(month(t)));
  Serial.print(' ');
  Serial.print(year(t));
  Serial.print(' ');
  Serial.print(tz);
  Serial.print(' ');
  Serial.print(loc);
  Serial.println();
}

//Print an integer in "00" format (with leading zero).
//Input value assumed to be between 0 and 99.
void sPrintI00(int val)
{
  if (val < 10) Serial.print('0');
  Serial.print(val, DEC);
  return;
}

//Print an integer in ":00" format (with leading zero).
//Input value assumed to be between 0 and 99.
void sPrintDigits(int val)
{
  Serial.print(':');
  if (val < 10) Serial.print('0');
  Serial.print(val, DEC);
}


void WifiCon() {

const char *ssid     = "WLAN_0XXX";
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
  
  GetTime();
  printhora();
  
WiFi.disconnect( true );
WiFi.mode( WIFI_OFF );
WiFi.forceSleepBegin();
delay( 1 );

// WAKE_RF_DISABLED to keep the WiFi radio disabled when we wake up
// ESP.deepSleep( SLEEPTIME, WAKE_RF_DISABLED );

  }




