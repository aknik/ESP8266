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

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.


void setup(){
  
  Serial.begin(115200);
  display.setBrightness(0x0a); //set the diplay to maximum brightness 0x0a
  WifiCon();
  
}


void loop() {
  
if (minute() == 15 && second() < 6 ) { WifiCon(); }

LCDprint();

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

void ponNTP()
{
//******************************************************************************************
//                          NTP                                                            *
//******************************************************************************************

unsigned int localPort = 2390;      // local port to listen for UDP packets
const char* ntpServerName = "hora.roa.es";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
int UdpNoReplyCounter = 0;
int cb = 0;

WiFiUDP udp;

IPAddress timeServerIP(192,168,2,1);

// WiFi.hostByName(ntpServerName, timeServerIP); //**********************COMENTAR ESTO PARA USAR SERVER NTP LOCAL ASIGNADO ARRIBA ********************************************
  
 
  
Serial.println(timeServerIP);
  
Serial.println("Starting UDP");
udp.begin(localPort);
Serial.print("Local port: ");
Serial.println(udp.localPort());

while (!cb) {

  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
  
  delay(1000);
  
  cb = udp.parsePacket();
  
  }
  

    UdpNoReplyCounter = 0;
    Serial.print("packet received, length=");
    Serial.println(cb);
    
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
    
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
  
  ponNTP();

WiFi.disconnect( true );
WiFi.mode( WIFI_OFF );
WiFi.forceSleepBegin();
delay( 1 );

// WAKE_RF_DISABLED to keep the WiFi radio disabled when we wake up
// ESP.deepSleep( SLEEPTIME, WAKE_RF_DISABLED );
//So, to confirm

//after:
//ESP.deepSleep(1000000, WAKE_RF_DISABLED),

//The only way to get WiFi working is
//ESP.deepSleep(1, WAKE_RF_DEFAULT)

//Would doing this have the same affect (just as low current use on next boot):

//WiFi.disconnect();
//WiFi.mode(WIFI_OFF);
//WiFi.forceSleepBegin();
//delay(1);
//ESP.deepSleep(1000000, WAKE_RF_DEFAULT);
  }
