// IR_WiFi_esp -- Remote control for TV, DVD, etc.
//
// Webserver receiving commands that will be converted to IR signals to control TV/DVD/Amplifier.
// It will drive the IR LEDs to simulate the remote controls.
// The webserver receives commands from the javascript (supplied in the index.html file on the server)
// running on a tablet or similar.
// The remote control can also simulate a PT2261-like remote control (433 MHz) for on/off swiching of lights.
//
// The original version of this program used an ESP8266 and an Arduino for this project.  At a later stage
// the functions are combined and only an ESP8266 is used.
//
// 15-04-2015, ES: First set-up
// 14-10-2015, ES: Find strongest WiFi signal
// 15-10-2015, ES: Show hostname on netwerk
// 09-03-2016, ES: Selective choice of WiFi netwerk
// 31-05-2016, ES: Combine ESP8266 and Arduino functions
// 20-07-2017, ES: Corrections for IRremoteESP8266 v2.0
//
// Wiring:
// NodeMCU  GPIO    Pin to program  Connect to
// -------  ------  --------------  --------------
// D1       GPIO0    0              433 MHz module
// D2       GPIO4    4              IR leds driver
//
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <stdio.h>
#include <string.h>
#include <FS.h>
#include <Time.h>
#include <ArduinoOTA.h>
extern "C"
{
  #include "user_interface.h"
}


//******************************************************************************************
// Some definitions                                                                        *
//******************************************************************************************
// Debug buffer size
#define DEBUG_BUFFER_SIZE 100
// Version number
#define VERSION "20-jul-2017"
// Output pins
#define IR_PIN D5
#define RF_PIN D0

//***********************Added for IR receiver **********************************
uint16_t RECV_PIN = D7;
IRrecv irrecv(RECV_PIN);
decode_results results;

//******************************************************************************************
// Forward declaration of methods                                                          *
//******************************************************************************************
char* dbgprint( const char* forTZmat, ... ) ;


//******************************************************************************************
// Global data section.                                                                    *
//******************************************************************************************
bool            myDEBUG = true ;          // Enable or disable debug output
String          ssid ;                    // SSID of selected WiFi network
WiFiServer      server(80) ;              // A server on port 80
AsyncWebServer  cmdserver ( 80 ) ;        // Instance of embedded webserver
IRsend          irsend ( IR_PIN ) ;       // Object for IR sender
bool            cmdreq = false ;          // Command received by server
String          argument[5] ;             // Arguments in command received by server


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
//                                  D B G P R I N T                                        *
//******************************************************************************************
// Send a line of info to serial output.  Works like vsprintf(), but checks the BEDUg flag.*
// Print only if myDEBUG flag is true.  Always returns the the formatted string.           *
//******************************************************************************************
char* dbgprint ( const char* format, ... )
{
  static char sbuf[DEBUG_BUFFER_SIZE] ;                // For debug lines
  va_list     varArgs ;                                // For variable number of params

  va_start ( varArgs, format ) ;                       // Prepare parameters
  vsnprintf ( sbuf, sizeof(sbuf), format, varArgs ) ;  // Format the message
  va_end ( varArgs ) ;                                 // End of using parameters
  if ( myDEBUG )                                       // DEBUG on?
  {
    Serial.print ( "D: " ) ;                           // Yes, print prefix
    Serial.println ( sbuf ) ;                          // and the info
  }
  return sbuf ;                                        // Return stored string
}


//******************************************************************************************
//                                   H E X T O L O N G                                     *
//******************************************************************************************
// Convert string to unsigned long.                                                        *
//******************************************************************************************
unsigned long hextolong ( const char* hexstr )
{
  unsigned long res = 0 ;

  while ( *hexstr )
  {
    if ( *hexstr >= '0' && *hexstr <= '9' )
    {
      res = ( res << 4 ) + *hexstr - '0' ;
    }
    else if ( *hexstr >= 'A' && *hexstr <= 'F' )
    {
      res = ( res << 4 ) + *hexstr - 'A' + 10 ;
    }
    hexstr++ ;
  }
  return res ;
}


//******************************************************************************************
//                                    S N D I T V                                          *
//******************************************************************************************
// Send code to KPN itv.                                                                   *
// Command is coded like "mSMSmsMSMSmsmsmsmsmsmsmsmsmsmsmsmsMSm".                          *
//******************************************************************************************
void snditv ( const char* str )
{
  uint16_t   sndbuf[50] ;
  uint16_t   i = 0 ;

  while ( *str )                          // Convert all characters
  {
    switch ( *str )
    {
      case 's' :                          // Short space
        sndbuf[i] = 317 ;
        break ;
      case 'S' :                          // Long space
        sndbuf[i] = 634 ;
        break ;
      case 'm' :                          // Short mark
        sndbuf[i] = 317 ;
        break ;
      case 'M' :                          // Long mark
        sndbuf[i] = 634 ;
        break ;
    }
    i++ ;
    str++ ;
  }
  irsend.sendRaw ( sndbuf, i, 56 ) ;      // Send code 56 kHz
}



//******************************************************************************************
//                               S N D 4 3 3 C O D E                                       *
//******************************************************************************************
// Send a frame to the 433 MHz module.                                                     *
// pw   is the (shortest) pulsewidth, for example 150 microseconds.                        *
// cmd  is a string containing one character for every bit to send: '0', '1' or 'f', for   *
//         example: "111110fffff0" for "A on".                                             *
// rept is the number of times to repeat, for exaple 8.                                    *
//******************************************************************************************
void send433code ( int pw, const char *cmd, int rept )
{
  int pw3 = 3 * pw ;    // Timing for long pulse/pause
  int i, j ;

  for ( i = 0 ; i < rept ; i++ )
  {
    for ( j = 0 ; cmd[j] ; j++ )  // Stops at end of string
    { 
      switch ( cmd[j] )
      {
        case '0' :                // Short pulse, long pause, short puls, long pause
    digitalWrite ( RF_PIN, HIGH ) ;
    delayMicroseconds ( pw ) ;
    digitalWrite ( RF_PIN, LOW ) ;
    delayMicroseconds ( pw3 ) ;
    digitalWrite( RF_PIN, HIGH ) ;
    delayMicroseconds ( pw ) ;
    digitalWrite ( RF_PIN, LOW ) ;
    delayMicroseconds ( pw3 ) ;
    break;
  case '1' :                // Long pulse, short pause, long pulse, short pause
    digitalWrite ( RF_PIN, HIGH ) ;
    delayMicroseconds ( pw3 ) ;
    digitalWrite ( RF_PIN, LOW ) ;
    delayMicroseconds ( pw ) ;
    digitalWrite ( RF_PIN, HIGH ) ;
    delayMicroseconds ( pw3 ) ;
    digitalWrite ( RF_PIN, LOW ) ;
    delayMicroseconds ( pw ) ;
    break;
  case 'f' :                // short pulse, long pause, long pulse, short pause
    digitalWrite ( RF_PIN, HIGH ) ;
    delayMicroseconds ( pw ) ;
    digitalWrite ( RF_PIN, LOW ) ;
    delayMicroseconds ( pw3 ) ;
    digitalWrite ( RF_PIN, HIGH ) ;
    delayMicroseconds ( pw3 ) ;
    digitalWrite ( RF_PIN, LOW ) ;
    delayMicroseconds ( pw ) ;
    break;
      }
    }
    // Send termination/synchronization-signal. Total length: 32 periods
    digitalWrite ( RF_PIN, HIGH ) ;
    delayMicroseconds ( pw ) ;
    digitalWrite ( RF_PIN, LOW ) ;
    delayMicroseconds ( pw * 31 ) ;
  }
}


//******************************************************************************************
//                             G E T E N C R Y P T I O N T Y P E                           *
//******************************************************************************************
// Read the encryption type of the network and return as a 4 byte name                     *
//*********************4********************************************************************
const char* getEncryptionType ( int thisType )
{
  switch (thisType) 
  {
    case ENC_TYPE_WEP:
      return "WEP " ;
    case ENC_TYPE_TKIP:
      return "WPA " ;
    case ENC_TYPE_CCMP:
      return "WPA2" ;
    case ENC_TYPE_NONE:
      return "None" ;
    case ENC_TYPE_AUTO:
      return "Auto" ;
  }
  return "????" ;
}


//******************************************************************************************
//                                L I S T N E T W O R K S                                  *
//******************************************************************************************
// List the available networks and select the strongest.                                   *
// Acceptable networks are those who have a "SSID.pw" file in the SPIFFS.                  *
//******************************************************************************************
void listNetworks()
{
  int         maxsig = -1000 ;   // Used for searching strongest WiFi signal
  int         newstrength ;
  byte        encryption ;       // TKIP(WPA)=2, WEP=5, CCMP(WPA)=4, NONE=7, AUTO=8 
  const char* acceptable ;       // Netwerk is acceptable for connection
  int         i, j ;
  bool        found ;            // True if acceptable network found
  String      path ;             // Full filespec to see if SSID is an acceptable one
  
  // scan for nearby networks:
  dbgprint ( "* Scan Networks *" ) ;
  int numSsid = WiFi.scanNetworks() ;
  if ( numSsid == -1 )
  {
    dbgprint ( "Couldn't get a wifi connection" ) ;
    return ;
  }
  // print the list of networks seen:
  dbgprint ( "Number of available networks: %d",
            numSsid ) ;
  // Print the network number and name for each network found and
  // find the strongest acceptable network
  for ( i = 0 ; i < numSsid ; i++ )
  {
    acceptable = "" ;                                    // Assume not acceptable
    path = String ( "/" ) + WiFi.SSID ( i ) + String ( ".pw" ) ;
    newstrength = WiFi.RSSI ( i ) ;
    if ( found = SPIFFS.exists ( path ) )                // Is this SSID acceptable?
    {
      acceptable = "Acceptable" ;
      if ( newstrength > maxsig )                        // This is a better Wifi
      {
        maxsig = newstrength ;
        ssid = WiFi.SSID ( i ) ;                         // Remember SSID name
      }
    }
    encryption = WiFi.encryptionType ( i ) ;
    dbgprint ( "%2d - %-25s Signal: %3d dBm Encryption %4s  %s",
               i + 1, WiFi.SSID ( i ).c_str(), WiFi.RSSI ( i ),
               getEncryptionType ( encryption ),
               acceptable ) ;
  }
  dbgprint ( "------------------------------------------------------------------" ) ;
}


//******************************************************************************************
//                               C O N N E C T W I F I                                     *
//******************************************************************************************
// Connect to WiFi using passwords available in the SPIFFS.                                *
//******************************************************************************************
void connectwifi()
{
  String path ;                                        // Full file spec
  String pw ;                                          // Password from file
  File   pwfile ;                                      // File containing password for WiFi
  char*  pfs ;                                         // Poiter to formatted string
  
  path = String ( "/" )  + ssid + String ( ".pw" ) ;   // Form full path
  pwfile = SPIFFS.open ( path, "r" ) ;                 // File name equal to SSID
  pw = pwfile.readStringUntil ( '\n' ) ;               // Read password as a string
  pw.trim() ;                                          // Remove CR                              
  WiFi.begin ( ssid.c_str(), pw.c_str() ) ;            // Connect to selected SSID
  dbgprint ( "Try WiFi %s", ssid.c_str() ) ;           // Message to show during WiFi connect
  if (  WiFi.waitForConnectResult() != WL_CONNECTED )  // Try to connect
  {
    dbgprint ( "WiFi Failed!" ) ;
    return;
  }
  pfs = dbgprint ( "IP = %d.%d.%d.%d",
                   WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] ) ;
  #if defined ( USETFT )
  tft.println ( pfs ) ;
  #endif
}


//******************************************************************************************
//                                   O T A S T A R T                                       *
//******************************************************************************************
// Update via WiFi has been started by Arduino IDE.                                        *
//******************************************************************************************
void otastart()
{
  dbgprint ( "OTA Started" ) ;
}


//******************************************************************************************
//                             G E T C O N T E N T T Y P E                                 *
//******************************************************************************************
// Returns the contenttype of a file to send.                                              *
//******************************************************************************************
String getContentType ( String filename )
{
  if      ( filename.endsWith ( ".html" ) ) return "text/html" ;
  else if ( filename.endsWith ( ".png"  ) ) return "image/png" ;
  else if ( filename.endsWith ( ".gif"  ) ) return "image/gif" ;
  else if ( filename.endsWith ( ".jpg"  ) ) return "image/jpeg" ;
  else if ( filename.endsWith ( ".ico"  ) ) return "image/x-icon" ;
  else if ( filename.endsWith ( ".zip"  ) ) return "application/x-zip" ;
  else if ( filename.endsWith ( ".gz"   ) ) return "application/x-gzip" ;
  else if ( filename.endsWith ( ".pw"   ) ) return "" ;              // Passwords are secret
  return "text/plain" ;
}


//******************************************************************************************
//                                H A N D L E F S                                          *
//******************************************************************************************
// Handling of requesting files from the SPIFFS. Example: /favicon.ico                     *
//******************************************************************************************
void handleFS ( AsyncWebServerRequest *request )
{
  String fnam ;                                         // Requested file
  String ct ;                                           // Content type

  fnam = request->url() ;
  dbgprint ( "onFileRequest received %s", fnam.c_str() ) ;
  ct = getContentType ( fnam ) ;                        // Get content type
  if ( ct == "" )                                       // Empty is illegal
  {
    request->send ( 404, "text/plain", "File not found" ) ;  
  }
  else
  {
    request->send ( SPIFFS, fnam, ct ) ;                // Okay, send the file
  }
}


//******************************************************************************************
//                             H A N D L E I R C M D                                       *
//******************************************************************************************
// Handling of the various IR / RF433 commands available in array arguments.               *
//******************************************************************************************
void handleIRCmd()
{
  unsigned long   li ;                                 // Code as unsigned long int
  
  if ( argument[0] == "samsung" )                      // Samsung DVD request?
  {
    li = hextolong ( argument[1].c_str() ) ;           // Convert code to long int
    irsend.sendSAMSUNG ( li, 32 ) ;                    // Send 32 bits SAMSUNG format
  }
  else if ( argument[0] == "kpnitv" )                  // KPN itv request?
  {
    snditv ( argument[1].c_str() ) ;                   // Send KPN itv format
  }
  else if ( argument[0] == "lgtv" )                    // LG tv request?
  {
    li = hextolong ( argument[1].c_str() ) ;           // Convert code to long int
    irsend.sendNEC ( li, 32 ) ;                        // Send 32 bits NEC format
  }
  else if ( argument[0] == "rf433" )                   // RF433 request?
  {
    send433code ( argument[1].toInt(), argument[2].c_str(), argument[3].toInt() ) ;
  }
}


//******************************************************************************************
//                             H A N D L E C M D                                           *
//******************************************************************************************
// A command is received by the server.  IR commands are not executed directly; they are   *
// handled in the main loop.  All commands contain at least one argument like "/?xxxx" or  *
// "/?xxxx&aaaa&bbbb".                                                                     *
// The maximal number of parameters is 5. They will be stored in array "arguments" and the *
// cmdreq flag is set to signal the main loop that a command is ready to be executed.      *
// The startpage will be returned if no arguments are given.                               *
// Non-IR commands are "debugon", "debugoff" and "test".                                   *
// An extra argument may be "version=<random number>", which will be ignored in order to   *
// prevent browsers like Edge and IE to use their cache.                                   *
// IR commands, examples:                                                                  *
//    "/?samsung&C2CA649B&power&version=0.9775479450590543" or                             *
//    "/?kpnitv&mSMSmsMSMSmsmsmsmsmsmsmsmsmsmsmsmsMSm&one&version=0.12"                    *
// The 1st argument tells us with protocol to use, the 2nd is the code to be transmitted.  *
// The 3rd argument is the command, like "power", "one" or "return". The command is not    *
// used in this sketch.                                                                    *
//******************************************************************************************
void handleCmd ( AsyncWebServerRequest *request )
{
  AsyncWebParameter* p ;                              // Points to parameter structure
  static char*       reply ;                          // Reply to client
  int                numargs ;                        // Number of arguments
  int                i ;                              // For loop through arguments 
  
  numargs = request->params() ;                       // Get number of arguments
  if ( numargs >  5 )                                 // Max. 5 arguments
  {
    numargs = 5 ;
  }
  if ( numargs == 0 )                                 // Any arguments?
  {
    request->send ( SPIFFS, "/index.html",            // No parameters, send the startpage
                    "text/html" ) ;
    return ;
  }
  if ( cmdreq )                                       // Check completion of previous command
  {
    dbgprint ( "Last IR command not finished!" ) ;    // Command coming in too fast...
  }
  for ( i = 0 ; i < numargs ; i++ )                   // Put parameters in array
  {
    p = request->getParam ( i ) ;                     // Get pointer to parameter structure
    argument[i] = p->name() ;                         // Get the argument
    dbgprint ( "Command[%d] is %s", i, argument[i].c_str() ) ;
  }
  argument[0].toLowerCase() ;                         // Force lowercase for first argument
  // Default reply
  reply = dbgprint ( "Command %s accepted", argument[0].c_str()  ) ;
  if ( argument[0] == "reset" )                       // Reset request?
  {
    ESP.restart() ;                                   // Reset all
    // No continuation here......
  }
  else if ( argument[0] == "test" )                   // Test command
  {
    reply = dbgprint ( "Free memory is %d", system_get_free_heap_size() ) ;
  }
  else if ( argument[0] =="debugon" )                 // debug on request?
  {
    myDEBUG = true ;                                  // Yes, set flag
  }
  else if ( argument[0] == "debugoff" )               // debug off request?
  {
    myDEBUG = false ;                                 // Yes, clear flag
  }
  else
  {
    // other commands (IR) will be handled in the main loop
    cmdreq = true ;
  }
  request->send ( 200, "text/plain", reply ) ;        // Send the reply
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
//******************************************************************************************
//                                   S E T U P                                             *
//******************************************************************************************
// Setup for the program.                                                                  *
//******************************************************************************************
void setup()
{
  FSInfo      fs_info ;                              // Info about SPIFFS
  Dir         dir ;                                  // Directory struct for SPIFFS
  File        f ;                                    // Filehandle
  String      filename ;                             // Name of file found in SPIFFS
  String      potSSID ;                              // Potential SSID if only 1 one password file
  int         numpwf = 0 ;                           // Number of password files 
  int         i ;                                    // Loop control

  irsend.begin() ;                                   // Set output pin for IR
  irrecv.enableIRIn();                // Start the receiver
  Serial.begin ( 115200 ) ;                          // For debugging
  Serial.println() ;
  pinMode ( RF_PIN, OUTPUT ) ;                       // Init I/O pin
  digitalWrite ( RF_PIN, LOW ) ;
  system_update_cpu_freq ( 80 ) ;                    // Set to 80/160 MHz
  SPIFFS.begin() ;                                   // Enable file system
  SPIFFS.info ( fs_info ) ;                          // Load info
  dbgprint ( "FS Total %d, used %d", fs_info.totalBytes, fs_info.usedBytes ) ;
  if ( fs_info.totalBytes == 0 )
  {
    dbgprint ( "No SPIFFS found!  See documentation." ) ;
  }
  dir = SPIFFS.openDir("/") ;                        // Show files in FS
  while ( dir.next() )                               // All files
  {
    f = dir.openFile ( "r" ) ;
    filename = dir.fileName() ;
    dbgprint ( "%-32s - %6d",                        // Show name and size
               filename.c_str(), f.size() ) ;
    if ( filename.endsWith ( ".pw" ) )               // If this a password file?
    {
      numpwf++ ;                                     // Yes, count number password files
      potSSID = filename.substring ( 1 ) ;           // Save filename (without starting "/") of potential SSID 
      potSSID.replace ( ".pw", "" ) ;                // Convert into SSID 
    }
  }
  WiFi.mode ( WIFI_STA ) ;                           // This ESP is a station
  wifi_station_set_hostname ( (char*)"ESP-IR" ) ;    // Set hostname
  // Print some memory and sketch info
  dbgprint ( "Starting ESP Version " VERSION "...  Free memory %d",
             system_get_free_heap_size() ) ;
  dbgprint ( "Sketch size %d, free size %d",
              ESP.getSketchSize(),
              ESP.getFreeSketchSpace() ) ;
  listNetworks() ;                                   // Search for strongest WiFi network  if ( numpwf == 1 )                                 // If there's only one pw-file...
  if ( numpwf == 1 )                                 // If there's only one pw-file...
  {
    dbgprint ( "Single (hidden) SSID found" ) ;
    ssid = potSSID ;                                 // Use this SSID (it may be hidden)
  }
  dbgprint ( "Selected network: %-25s", ssid.c_str() ) ;
  connectwifi() ;                                    // Connect to WiFi network
  dbgprint ( "Start server for commands" ) ;
  cmdserver.on ( "/", handleCmd ) ;                  // Handle startpage
  cmdserver.onNotFound ( handleFS ) ;                // Handle file from FS
  cmdserver.begin() ;
  ArduinoOTA.setHostname ( "ESP-IR" ) ;              // Set the hostname
  ArduinoOTA.onStart ( otastart ) ;
  ArduinoOTA.begin() ;                               // Allow update over the air


  udp.begin(localPort);

   ponNTP();
   
}


//******************************************************************************************
//                                   L O O P                                               *
//******************************************************************************************
// Main loop of the program.                                                               *
//******************************************************************************************
void loop()
{
  String req ;
  int    pos ;

  if ( cmdreq )                                      // New command waiting?
  {
    handleIRCmd() ;                                  // Yes, handle it
    cmdreq = false ;                                 // Clear flag
  }
  // Check de WiFi status.  Als die wegvalt zorgt de watchdog voor reset
  if ( WiFi.status() != WL_CONNECTED )
  {
    dbgprint ( "WiFi disconnected, going to restart..." ) ;
    ESP.restart() ;                                  // Reset the CPU, probably no return
  }


//*******************************************IR RECEIVER LOOP ******************************************
  if (irrecv.decode(&results)) {
    // print() & println() can't handle printing long longs. (uint64_t)

if (results.value == 0xFFB24D) { irsend.sendNEC(0x20DF10EF,32); } // on off

if (results.value == 0xFF48B7) { irsend.sendNEC(0x20DF08F7,32); } // 0
if (results.value == 0xFF906F) { irsend.sendNEC(0x20DF8877,32); } // 1
if (results.value == 0xFFB847) { irsend.sendNEC(0x20DF48B7,32); } // 2
if (results.value == 0xFFF807) { irsend.sendNEC(0x20DFC837,32); }
if (results.value == 0xFFB04F) { irsend.sendNEC(0x20DF28D7,32); }
if (results.value == 0xFF9867) { irsend.sendNEC(0x20DFA857,32); }
if (results.value == 0xFFD827) { irsend.sendNEC(0x20DF6897,32); }
if (results.value == 0xFF8877) { irsend.sendNEC(0x20DFE817,32); }
if (results.value == 0xFFA857) { irsend.sendNEC(0x20DF18E7,32); }
if (results.value == 0xFFE817) { irsend.sendNEC(0x20DF9867,32); }

if (results.value == 0xFF6897) { irsend.sendNEC(0x20DF906F,32); } // mute-> 


    
    irrecv.resume();  // Receive the next value
    
  }




 





  ArduinoOTA.handle() ;                              // Check for OTA 
}

