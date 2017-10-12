// include the library code:
#include <ESP8266WiFi.h>
#include <Time.h>

int TZ = +2; 

// WiFi connection
const char *ssid     = "WLAN_XXXX";
const char *password = ".999999999.";

WiFiServer server(80);

extern "C"
{
#include "user_interface.h"
}


 
 
void setup () {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED)
  {
     delay(100);
  }
  Serial.print("\n\nConexión Wifi.\nIP address: ");
  Serial.println(WiFi.localIP());
  server.begin();


}

void loop () {

GetTime(TZ);
printhora();
delay(5000);


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
setTime(t + TZ*3600 + 1);

}
