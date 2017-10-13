#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>
 
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
const char *ssid     = "WLAN_XXXX";
const char *password = ".999999999.";

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.

//******************************************************************************************
// NTP                                                            *
//******************************************************************************************

unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "hora.roa.es";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
//udp reply missing counter
int UdpNoReplyCounter = 0;

//******************************************************************************************

void setup(){
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED)
  {
     delay(100);
  }
  Serial.print("\n\nConexión Wifi.\nIP address: ");
  Serial.println(WiFi.localIP());
  
  display.setBrightness(0x0a); //set the diplay to maximum brightness 0x0a
  
  ponNTP();
  
}


void loop() {
  
if (minute()%10 == 0) { ponNTP(); }

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
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 
  
IPAddress timeServerIP(192,168,2,1);
Serial.println(timeServerIP);

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
    //try max 3 times (every minute) after that we force the wifi to reconnect
    if (UdpNoReplyCounter++ == 3){
      Serial.println("Error reply udp");
      UdpNoReplyCounter = 0;
    };
  } else {
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
    
time_t t = epoch;
setTime(t + TimeZone*3600 );

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }

}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
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
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
