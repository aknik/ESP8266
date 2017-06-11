

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

/* Cambiando las MAC tanto del AP como del cliente Wifi en el ESP8266 */

extern "C" {
#include "user_interface.h"
}

uint8_t  MAC_softAP[] = { 0xf8, 0xfb, 0x56, 0x2c, 0xb5, 0x01 };                      
uint8_t  MAC_STA[]    = { 0xf8, 0xfb, 0x56, 0x2c, 0xb5, 0x02 };   

bool a = wifi_set_macaddr(STATION_IF, &MAC_STA[0]);
bool b = wifi_set_macaddr(SOFTAP_IF, &MAC_softAP[0]);


/* Cambiando las MAC tanto del AP como del cliente Wifi en el ESP8266 */



/* Set these to your desired credentials. */
const char *ssid = "MOVISTAR_B501";
const char *password = "11111111";

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
	server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
	delay(1000);
	Serial.begin(115200);
	Serial.println();



WiFi.mode(WIFI_AP);



   Serial.print("MAC[SoftAP]");
    uint8_t* MAC  = WiFi.softAPmacAddress(MAC_softAP);                   
    for (int i = 0; i < sizeof(MAC)+2; ++i){                                                          
         Serial.print(":");
         Serial.print(MAC[i],HEX);
         MAC_softAP[i] = MAC[i];                                         
    }
    Serial.println();
    Serial.print("MAC[STA]");
    MAC  = WiFi.macAddress(MAC_STA);                   
    for (int i = 0; i < sizeof(MAC)+2; ++i){
         Serial.print(":");
         Serial.print(MAC[i],HEX);
         MAC_STA[i] = MAC[i];                                            
    }
    Serial.println(); 




 
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);

	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	server.on("/", handleRoot);
	server.begin();
	Serial.println("HTTP server started");
}

void loop() {
	server.handleClient();
}
