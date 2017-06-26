/*
  GPS (Holux M-1000) data reading program
  
  Programmed by: Jinseok Jeon (JeonLab.wordpress.com)
  Date: September 2013
*/  
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>

SoftwareSerial gps(14,12);// RX, TX

// WiFi parameters
const char* ssid = "WLAN_0BB0";
const char* password = "999999999";
WiFiServer server(80);

const int TimeZone = -5; 

int DSTbegin[] = { //DST 2013 - 2025 in Canada and US
  310, 309, 308, 313, 312, 311, 310, 308, 314, 313, 312, 310, 309};
int DSTend[] = { //DST 2013 - 2025 in Canada and US
  1103, 1102, 1101, 1106, 1105, 1104, 1103, 1101, 1107, 1106, 1105, 1103, 1102};
int DaysAMonth[] = { //number of days a month
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int gpsYear;
int gpsMonth;
int gpsDay = 0;
int gpsHour;
byte gpsMin;
byte gpsSec;
float gpsLat0;
float gpsLong0;
float gpsLat;
float gpsLong;
float gpsSpeed; //km/h
float gpsBearing; //deg

void setup() {
  Serial.begin(38400);
  gps.begin(38400);

  Serial.println("\n............................");
  Serial.println("\nSoftware serial started");
  delay(500);
  
  // Connect to WiFi network
WiFi.mode(WIFI_STA);
Serial.println();
Serial.println();
Serial.print("Intentando conexión a ");
Serial.println(ssid);

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
                                      }
Serial.println("");
Serial.println("Conexión WiFi correcta.");

// Start the server
server.begin();
Serial.println("Servidor WEB funcionando.");

// Print the IP address
Serial.println(WiFi.localIP());
  
}

void loop() 
{
  yield();
  
  if (gps.available() > 0) 
  {
    if (char(gps.read()) == 'R' && char(gps.read()) == 'M' && char(gps.read()) == 'C') 
    {
      Serial.println(".");
      gpsTime(gps.parseInt());
      gps.parseFloat(); //discard unnecessary part
      gpsLatLong(gps.parseInt(), gps.parseInt(), gps.parseInt(), gps.parseInt());
      gpsSpeed = gps.parseFloat()*1.852; //km/h
      gpsBearing = gps.parseFloat();
      gpsDate(gps.parseInt());
      if (gpsYear%4 == 0) DaysAMonth[1] = 29; //leap year check

      //Time zone adjustment
      gpsHour += TimeZone;
      //DST adjustment
      if (gpsMonth*100+gpsDay >= DSTbegin[gpsYear-13] && 
        gpsMonth*100+gpsDay < DSTend[gpsYear-13]) gpsHour += 1;
      if (gpsHour < 0)
      {
        gpsHour += 24;
        gpsDay -= 1;
        if (gpsDay < 1)
        {
          if (gpsMonth == 1)
          {
            gpsMonth = 12;
            gpsYear -= 1;
          }
          else
          {
            gpsMonth -= 1;
          }
          gpsDay = DaysAMonth[gpsMonth-1];
        } 
      }
      if (gpsHour >= 24)
      {
        gpsHour -= 24;
        gpsDay += 1;
        if (gpsDay > DaysAMonth[gpsMonth-1])
        {
          gpsDay = 1;
          gpsMonth += 1;
          if (gpsMonth > 12) gpsYear += 1;
        }
      }

      Serial.print(gpsDay);
      Serial.print("/");
      Serial.print(gpsMonth);
      Serial.print("/");
      Serial.print(gpsYear);
      Serial.print(" ");
      Serial.print(gpsHour);
      Serial.print(":");
      Serial.println(gpsMin);

WiFiClient client = server.available();
client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println("Connection: close");  // the connection will be closed after completion of the response
client.println("Refresh: 30");  // refresh the page automatically every 5 sec
client.println();
client.println("<!DOCTYPE html>");
client.println("<html xmlns='http://www.w3.org/1999/xhtml'>");
client.println("<head>\n<meta charset='UTF-8'>");
client.println("<title>GPS CLOCK</title>");
client.println("<link href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' rel='stylesheet'>");
client.println("</head>\n<body>");
client.println("<div class='container'>");
client.println("<div class='panel panel-default' margin:15px>");
client.println("<div class='panel-heading'>");
client.println("<H2>GPS CLOCK</H2>");
client.println("<div class='panel-body' style='background-color: blue'>");
client.println("<pre>");

client.print("GPS TIME : ");
client.print(gpsHour);
client.print(":");
client.print(gpsMin);
client.print("     ");

client.print(gpsDay);
client.print("/");
client.print(gpsMonth);
client.print("/");
client.print(gpsYear);
client.println("");


client.println("</pre>");
client.println("</div>");
client.println("</div>");
client.println("</div>");
client.print("</body>\n</html>");

delay(20000);
yield();
}




      
    }
  }
  

void gpsTime(long UTC)
{
  gpsHour = int(UTC/10000);
  gpsMin = int(UTC%10000/100);
  gpsSec = UTC%100;
}

void gpsLatLong(int lat1, int lat2, int long1, int long2)
{
  gpsLat = int(lat1/100) + (lat1%100)/60.0 + float(lat2)/10000.0/60.0;
  //  Serial.print(gpsLat,4);
  gpsLong = int(long1/100) + (long1%100)/60.0 + float(long2)/10000.0/60.0;
  //  Serial.print(gpsLong,4);
}

void gpsDate(long dateRead)
{
  gpsDay = int(dateRead/10000);
  gpsMonth = int(dateRead%10000/100);
  gpsYear = dateRead%100; //last 2 digits, e.g. 2013-> 13
}






