// 
// https://www.hackster.io/rayburne/esp8266-turn-off-wifi-reduce-current-big-time-1df8ae

//You can create functions of timer and call_back to replace while(1) loop in the non-OS SDK.
//If the consecutive execution time of a piece of code exceeds 3.2s, you can disable the watchdog reset through the following methods:
//
//    Call the function of systerm_soft_wdt_stop() before executing this piece of code. It takes 6s from closing wdt to wdt hard trigger. 
//
//    Execute watchdog feed several times, i.e., systerm_soft_wdt_feed().
//
//    After completing the execution, call the function of systerm_soft_wdt_restart(). 
//
// ------------------begin ESP8266'centric----------------------------------


#define FREQUENCY    160                  // valid 80, 160
//
#include "ESP8266WiFi.h"
extern "C" {
#include "user_interface.h"
}
// ------------------end ESP8266'centric------------------------------------

#define LED_BUILTIN D4

void setup() {
/* Initializations */
// ------------------begin ESP8266'centric----------------------------------
WiFi.forceSleepBegin();                  // turn off ESP8266 RF
delay(1);                                // give RF section time to shutdown
system_update_cpu_freq(FREQUENCY);
// ------------------end ESP8266'centric------------------------------------

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
}

// the loop function runs over and over again forever
void loop() {

// ------------------begin ESP8266'centric----------------------------------
// pat the watchdog timer
wdt_reset();
// ------------------end ESP8266'centric------------------------------------


  
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(1000);                      // Wait for two seconds (to demonstrate the active low LED)
}
