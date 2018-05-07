#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include <Time.h>
#include "si5351.h"
#include "Wire.h"
  
ESP8266WiFiMulti WiFiMulti;
Si5351 si5351; // Pines D1 SCL   D2 SDA
Ticker tickerSetLow;

#define LedPin D8

#define MaxPulseNumber 603
#define FirstMinutePulseBegin 2
#define SecondMinutePulseBegin 62
#define ThirdMinutePulseBegin 122
#define CuartoMinutePulseBegin 182
#define QuintoMinutePulseBegin 242
#define SextoMinutePulseBegin 302
#define SeptimoMinutePulseBegin 362
#define OctavoMinutePulseBegin 422
#define NovenoMinutePulseBegin 482
#define DecimoMinutePulseBegin 542

//complete array of pulses for three minutes
//0 = no pulse, 1=100msec, 2=200msec
int ArrayImpulsi[MaxPulseNumber];

int ContaImpulsi = 0;
int UscitaDcfOn = 0;
int ContaImpulsiParziale = 0;
int Ore,Minuti,Secondi,Giorno,Mese,Anno,DayOfW;

const int timeZone = 1;     // Central European Time

int Dls;                    //DayLightSaving

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "hora.roa.es", 3600, 6000000);

void setup(){

tickerSetLow.attach_ms(100, DcfOut );

si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
si5351.set_freq(7750000ULL, SI5351_CLK0);
si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
si5351.output_enable(SI5351_CLK0, 0) ; 


  Serial.begin(115200);
  Serial.println("\n\nDCF77 transmitter INIT.\n\n");
  WiFi.disconnect();
  
    WiFiMulti.addAP("......", ".......");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

Serial.println("\n\nConexión WiFi completada.\n\n");

  ArrayImpulsi[0] = 1;
  ArrayImpulsi[1] = 0;
  ArrayImpulsi[MaxPulseNumber - 1] = 1;
  ContaImpulsi = 0;
  UscitaDcfOn = 0;    
 
  timeClient.begin();
  timeClient.update();
  
  while ( epoch < 1525637993 ) {
    delay ( 500 );
    timeClient.update();
    epoch = timeClient.getEpochTime(); 
    Serial.print ( "*" );                 }

    si5351.output_enable(SI5351_CLK0, 1);
    Serial.println ( "\n" );
 
  
}

void loop() {

  //check the lan status
  if (WiFi.status() == WL_CONNECTED)
     LeggiEdecodificaTempo();
  else
    ESP.reset();


}

void LeggiEdecodificaTempo() {

  int DayToEndOfMonth,DayOfWeekToEnd,DayOfWeekToSunday;

    timeClient.update();
  
    long unsigned epoch = timeClient.getEpochTime();
    
    //setTime(epoch);
    
    Serial.println("\n\n\n");
    Serial.println(epoch);
    Serial.println(timeClient.getFormattedTime());

    //calculate actual day to evaluate the summer/winter time of day ligh saving
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Serial.print("Hora local: ");       // UTC is the time at Greenwich Meridian (GMT)
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
    
epoch += 120 ;   ////////////////  DCF77 envia los minutos siguientes al corriente.

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

    //add one minute ad calculate cuarto
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(CuartoMinutePulseBegin);

    //one minute more for the quinto
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(QuintoMinutePulseBegin);

        //add one minute ad calculate cuarto
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(SextoMinutePulseBegin);

    //one minute more for the quinto
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(SeptimoMinutePulseBegin);
   

    //one minute more for the quinto
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(OctavoMinutePulseBegin);

        //add one minute ad calculate cuarto
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(NovenoMinutePulseBegin);

    //one minute more for the quinto
    epoch += 60;
    DayOfW = weekday(epoch);
    Giorno = day(epoch);
    Mese = month(epoch);
    Anno = year(epoch);
    Ore = hour(epoch);
    Minuti = minute(epoch);
    Secondi = second(epoch);
    CalcolaArray(DecimoMinutePulseBegin);

    
    //how many to the minute end ?
    //don't forget that we begin transmission at second 58°
    int DaPerdere = 58 - Secondi;
    delay(DaPerdere * 1000);


    
    // comienza transmision de 10 minutos
    
    Serial.println("Comienza transmisión.");
    si5351.output_enable(SI5351_CLK0, 1); 
    UscitaDcfOn = 1;

    
 for (int n=0;n<10;n++)  {      
  
  Serial.println(timeClient.getFormattedTime());
  delay(60000);

                  }
                  
  si5351.output_enable(SI5351_CLK0, 0); 

  

  };


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
  //La libreria de Arduino el Domingo es 1 y en DCF77 el domingo es 7 !!!
  DayOfW = DayOfW - 1; if (DayOfW == 0) DayOfW = 7;
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

//for debug: print the whole 180 secs array

    Serial.print(':'); for (n=0;n<60;n++){  Serial.print(ArrayImpulsi[n+ArrayOffset]); } Serial.println();


}


void DcfOut() {

  //Serial.println(second());
  
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



