
#ifndef PS2Keyboard_h
#define PS2Keyboard_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h" // for attachInterrupt, FALLING
#else
#include "WProgram.h"
#endif

#include "utility/int_pins.h"

// Every call to read() returns a single byte for each
// keystroke.  These configure what byte will be returned
// for each "special" key.  To ignore a key, use zero.


#define PS2_KEYMAP_SIZE 0

typedef struct {
	uint8_t noshift[PS2_KEYMAP_SIZE];
	uint8_t shift[PS2_KEYMAP_SIZE];
	unsigned int uses_altgr;
    /*
     * "uint8_t uses_altgr;" makes the ESP8266 - NodeMCU modules crash.
     * So, I replaced it with an int and... It works!
     * I think it's because of the 32-bit architecture of the ESP8266
     * and the use of the flash memory to store the keymaps.
     * Maybe I'm wrong, it remains a hypothesis.
     */
	uint8_t altgr[PS2_KEYMAP_SIZE];
} PS2Keymap_t;


extern const PROGMEM PS2Keymap_t PS2Keymap_US;

/**
 * Purpose: Provides an easy access to PS2 keyboards
 * Author:  Christian Weichel
 */
class PS2Keyboard {
  public:
  	/**
  	 * This constructor does basically nothing. Please call the begin(int,int)
  	 * method before using any other method of this class.
  	 */
    PS2Keyboard();

    /**
     * Starts the keyboard "service" by registering the external interrupt.
     * setting the pin modes correctly and driving those needed to high.
     * The propably best place to call this method is in the setup routine.
     */
    static void begin(uint8_t dataPin, uint8_t irq_pin );

    /**
     * Returns true if there is a char to be read, false if not.
     */
    static bool available();

    /* Discards any received data, sets available() to false without a call to read()
    */
    static void clear();

    /**
     * Retutns ps2 scan code.
     */
    static uint8_t readScanCode(void);

    /**
     * Returns the char last read from the keyboard.
     * If there is no char available, -1 is returned.
     */
    static int read();
    static int readUnicode();
};

#endif
