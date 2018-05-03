
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <Time.h>
#include "si5351.h"
#include "Wire.h"

Si5351 si5351;
Ticker tickerSetLow;

char ssid[] = "WLAN______";  //  your network SSID (name)
char pass[] = "...........";       // your network password


unsigned int localPort = 2390;      // local port to listen for UDP packets
/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE =  48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
//udp reply missing counter
int UdpNoReplyCounter = 0;
  
//how many total pulses we have
//three coimplete minutes + 2 head pulses and one tail pulse
#define MaxPulseNumber 183
#define FirstMinutePulseBegin 2
#define SecondMinutePulseBegin 62
#define ThirdMinutePulseBegin 122

//complete array of pulses for three minutes
//0 = no pulse, 1=100msec, 2=200msec
int ArrayImpulsi[MaxPulseNumber];
int ContaImpulsi = 0;
int UscitaDcfOn = 0;
int ContaImpulsiParziale = 0;
int Ore,Minuti,Secondi,Giorno,Mese,Anno,DayOfW;

const int timeZone = 1;     // Central European Time
int Dls;                    //DayLightSaving


void setup()
{

tickerSetLow.attach_ms(100, DcfOut );

si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
si5351.set_freq(7750000ULL, SI5351_CLK0);
si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_4MA);

  Serial.begin(115200);
  Serial.println();
  Serial.println("DCF77 emulator INIT");
  
  //first 2 pulses: 1 + blank to simulate the packet beginning
  //il primo bit e' un 1
  ArrayImpulsi[0] = 1;
  //segue l'impulso mancante che indica il sincronismo di ricerca inizio minuto
  ArrayImpulsi[1] = 0;

  //last pulse after the third 59° blank
  ArrayImpulsi[MaxPulseNumber - 1] = 1;
  
  ContaImpulsi = 0;
  UscitaDcfOn = 0;    //we begin with the output OFF

  //we begin connecting to wifi
  //NOTE testing WiFi.status() BEFORE the FIRST WiFi.begin() seems to hang the system
  //so we attempt a first connection BEFORE the main loop
  ConnettiWifi();
}

void loop(){
  
  //check the lan status
  if (WiFi.status() == WL_CONNECTED)
     LeggiEdecodificaTempo();
  else
    ConnettiWifi();

  delay(60000);
}

void ConnettiWifi(){
  int Timeout;
  WiFi.disconnect();
  Serial.println("disconnessione !");
  delay(1000);        //necessary ???
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  Timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (Timeout++ > 60){
      Serial.println("\nImpossible to connect WLAN!");
      WiFi.disconnect();
      return;
    }
  };
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void LeggiEdecodificaTempo() {

  int DayToEndOfMonth,DayOfWeekToEnd,DayOfWeekToSunday;

  WiFi.hostByName(ntpServerName, timeServerIP); 

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
      Serial.println("troppi errori di reply udp");
      ConnettiWifi();
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
    //note: we add two minutes because the dcf protocol send the time of the FOLLOWING minute
    //and our transmissione begin the next minute more
    time_t epoch = secsSince1900 - seventyYears + ( timeZone * 3600 ) + 120;
    // print Unix time:
    Serial.println(epoch);

    //calculate actual day to evaluate the summer/winter time of day ligh saving
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Serial.print("Tempo Locale ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print(Giorno);
    Serial.print('/');
    Serial.print(Mese);
    Serial.print('/');
    Serial.print(Anno);
    Serial.print(' ');
  
    //calcolo ora solare o legale
    Dls = 0;    //default winter time
    //From April to september we are surely on summer time
    if (Mese > 3 && Mese < 10) {
      Dls = 1;
    };
    //March, month of change winter->summer time, last last sunday of the month
    //March has 31days so from 25 included on sunday we can be in summer time
    if (Mese == 3 && Giorno > 24) {
      DayToEndOfMonth = 31 - Giorno;
      DayOfWeekToSunday = 7 - DayOfW;
      if (DayOfWeekToSunday >= DayToEndOfMonth)
        Dls = 1;
    };
    //Octobee, month of change summer->winter time, l'ultima Domenica del mese
    //Even Octobee has 31days so from 25 included on sunday we can be in winter time
    if (Mese == 10) {
      Dls = 1;
      if (Giorno > 24) {
        DayToEndOfMonth = 31 - Giorno;
        DayOfWeekToEnd = 7 - DayOfW;
        if (DayOfWeekToEnd >= DayToEndOfMonth)
        Dls = 0;
      };
    };

    Serial.print("Dls:");
    Serial.print(Dls);
    Serial.print(' ');
    //add one hour if we are in summer time
    if (Dls == 1)
      epoch += 3600;

    //now that we know the dls state, we can calculate the time too
    // print the hour, minute and second:
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    Serial.print(Ore); // print the hour
    Serial.print(':');
    Serial.print(Minuti); // print the minute
    Serial.print(':');
    Serial.println(Secondi); // print the second

    //if we are over about the 56° second we risk to begin the pulses too late, so it's better
    //to skit at the half of the next minute and NTP+recalculate all again
    if (Secondi > 56){
      delay(30000);
      return;      
    }

    //calculate bits array for the first minute
    CalcolaArray(FirstMinutePulseBegin);

    //add one minute ad calculate array again fot the second minute
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(SecondMinutePulseBegin);

    //one minute more for the third minute
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(ThirdMinutePulseBegin);

   
    //how many to the minute end ?
    //don't forget that we begin transmission at second 58°
    int DaPerdere = 58 - Secondi;
    delay(DaPerdere * 1000);
    //begin
    UscitaDcfOn = 1;

    //three minutes are needed to transmit all the packet
    //then wait more 30 secs to locate safely at the half of minute
    //NB 150+60=210sec, 60secs are lost from main routine

    delay(150000);

  };

  udp.stop() ;

}


void CalcolaArray(int ArrayOffset) {
  int n,Tmp,TmpIn;
  int ParityCount = 0;

  //i primi 20 bits di ogni minuto li mettiamo a valore logico zero
  for (n=0;n<20;n++)
    ArrayImpulsi[n+ArrayOffset] = 1;

  //DayLightSaving bit
  if (Dls == 1)
    ArrayImpulsi[17+ArrayOffset] == 2;
  else
    ArrayImpulsi[18+ArrayOffset] == 2;
    
  //il bit 20 deve essere 1 per indicare tempo attivo
  ArrayImpulsi[20+ArrayOffset] = 2;

  //calcola i bits per il minuto
  TmpIn = Bin2Bcd(Minuti);
  for (n=21;n<28;n++) {
    Tmp = TmpIn & 1;
    ArrayImpulsi[n+ArrayOffset] = Tmp + 1;
    ParityCount += Tmp;
    TmpIn >>= 1;
  };
  if ((ParityCount & 1) == 0)
    ArrayImpulsi[28+ArrayOffset] = 1;
  else
    ArrayImpulsi[28+ArrayOffset] = 2;

  //calcola i bits per le ore
  ParityCount = 0;
  TmpIn = Bin2Bcd(Ore);
  for (n=29;n<35;n++) {
    Tmp = TmpIn & 1;
    ArrayImpulsi[n+ArrayOffset] = Tmp + 1;
    ParityCount += Tmp;
    TmpIn >>= 1;
  }
  if ((ParityCount & 1) == 0)
    ArrayImpulsi[35+ArrayOffset] = 1;
  else
    ArrayImpulsi[35+ArrayOffset] = 2;
   ParityCount = 0;
  //calcola i bits per il giorno
  TmpIn = Bin2Bcd(Giorno);
  for (n=36;n<42;n++) {
    Tmp = TmpIn & 1;
    ArrayImpulsi[n+ArrayOffset] = Tmp + 1;
    ParityCount += Tmp;
    TmpIn >>= 1;
  }
  //calcola i bits per il giorno della settimana
  TmpIn = Bin2Bcd(DayOfW);
  for (n=42;n<45;n++) {
    Tmp = TmpIn & 1;
    ArrayImpulsi[n+ArrayOffset] = Tmp + 1;
    ParityCount += Tmp;
    TmpIn >>= 1;
  }
  //calcola i bits per il mese
  TmpIn = Bin2Bcd(Mese);
  for (n=45;n<50;n++) {
    Tmp = TmpIn & 1;
    ArrayImpulsi[n+ArrayOffset] = Tmp + 1;
    ParityCount += Tmp;
    TmpIn >>= 1;
  }
  //calcola i bits per l'anno
  TmpIn = Bin2Bcd(Anno - 2000);   //a noi interesa solo l'anno con ... il millenniumbug !
  for (n=50;n<58;n++) {
    Tmp = TmpIn & 1;
    ArrayImpulsi[n+ArrayOffset] = Tmp + 1;
    ParityCount += Tmp;
    TmpIn >>= 1;
  }
  //parita' di data
  if ((ParityCount & 1) == 0)
    ArrayImpulsi[58+ArrayOffset] = 1;
  else
    ArrayImpulsi[58+ArrayOffset] = 2;

  //ultimo impulso mancante
  ArrayImpulsi[59+ArrayOffset] = 0;

  /* for debug: print the whole 180 secs array
   * Serial.print(':');
  for (n=0;n<60;n++)
    Serial.print(ArrayImpulsi[n+ArrayOffset]);*/

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

void DcfOut() {
  
  if (UscitaDcfOn == 1) {
    switch (ContaImpulsiParziale++) {
      case 0:
        if (ArrayImpulsi[ContaImpulsi] != 0)

         si5351.output_enable(SI5351_CLK0, 0); 
         break;
      case 1:
        if (ArrayImpulsi[ContaImpulsi] == 1)
         
         si5351.output_enable(SI5351_CLK0, 1); 
        break;
      case 2:
        
        si5351.output_enable(SI5351_CLK0, 1); 
        break;
      case 9:
        if (ContaImpulsi++ == (MaxPulseNumber -1 )){     //one less because we FIRST tx the pulse THEN count it
          ContaImpulsi = 0;
          UscitaDcfOn = 0;
        };
        ContaImpulsiParziale = 0;
        break;
    };
  };

}

int Bin2Bcd(int dato) {
  int msb,lsb;

  if (dato < 10)
    return dato;
  msb = (dato / 10) << 4;
  lsb = dato % 10; 
  return msb + lsb;
}

