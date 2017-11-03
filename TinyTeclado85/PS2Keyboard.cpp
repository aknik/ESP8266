#include "PS2Keyboard.h"

#define BUFFER_SIZE 45
static volatile uint8_t buffer[BUFFER_SIZE];
static volatile uint8_t head, tail;
static uint8_t DataPin;
static uint8_t CharBuffer=0;
static uint8_t UTF8next=0;
static uint8_t K[255];

// The ISR for the external interrupt
void ps2interrupt(void)
{
	static uint8_t bitcount=0;
	static uint8_t incoming=0;
	static uint32_t prev_ms=0;
	uint32_t now_ms;
	uint8_t n, val;

	val = digitalRead(DataPin);
	now_ms = millis();
	if (now_ms - prev_ms > 250) {
		bitcount = 0;
		incoming = 0;
	}
	prev_ms = now_ms;
	n = bitcount - 1;
	if (n <= 7) {
		incoming |= (val << n);
	}
	bitcount++;
	if (bitcount == 11) {
		uint8_t i = head + 1;
		if (i >= BUFFER_SIZE) i = 0;
		if (i != tail) {
			buffer[i] = incoming;
			head = i;
		}
		bitcount = 0;
		incoming = 0;
	}
}

static inline uint8_t get_scan_code(void)
{
	uint8_t c, i;

	i = tail;
	if (i == head) return 0;
	i++;
	if (i >= BUFFER_SIZE) i = 0;
	c = buffer[i];
	tail = i;
	return c;
}

// http://www.quadibloc.com/comp/scan.htm
// http://www.computer-engineering.org/ps2keyboard/scancodes2.html

static char get_iso8859_code(void)
{
	static uint8_t state=0;
	uint8_t s;

	while (1) {
		s = get_scan_code();

			if (s) return s;
			}
	
}

bool PS2Keyboard::available() {
	if (CharBuffer || UTF8next) return true;
	CharBuffer = get_iso8859_code();
	if (CharBuffer) return true;
	return false;
}

void PS2Keyboard::clear() {
	CharBuffer = 0;
	UTF8next = 0;
}

uint8_t PS2Keyboard::readScanCode(void)
{
	return get_scan_code();
}

int PS2Keyboard::read() {
	uint8_t result;
	result = UTF8next;
	if (result) {
		UTF8next = 0;
	} else {
		result = CharBuffer;
		if (result) {
			CharBuffer = 0;
		} else {
			result = get_scan_code();
		}
		
	}
	

		
		
		
	if (result == 0xE0 ) {
		
		result = get_iso8859_code();
		
		if (result == 0x11 ) { result = 0x40; }
		
		
		
		}
	
	
	
	
	
	
	
	
	result = K[result];	// Conversion pskeyboard codes a HID USB codes ...
	
	if (!result) return -1;
	return result;
}



PS2Keyboard::PS2Keyboard() {
  // nothing to do here, begin() does it all
}

void PS2Keyboard::begin(uint8_t data_pin, uint8_t irq_pin ) {
  uint8_t irq_num=255;
  
  DataPin = data_pin;
    // keymap = &map;

  // initialize the pins
#ifdef INPUT_PULLUP
  pinMode(irq_pin, INPUT_PULLUP);
  pinMode(data_pin, INPUT_PULLUP);
#else
  pinMode(irq_pin, INPUT);
  digitalWrite(irq_pin, HIGH);
  pinMode(data_pin, INPUT);
  digitalWrite(data_pin, HIGH);
#endif

#ifdef CORE_INT_EVERY_PIN
  irq_num = irq_pin;

#else
  switch(irq_pin) {
    #ifdef CORE_INT0_PIN
    case CORE_INT0_PIN:
      irq_num = 0;
      break;
    #endif
    #ifdef CORE_INT1_PIN
    case CORE_INT1_PIN:
      irq_num = 1;
      break;
    #endif
    #ifdef CORE_INT2_PIN
    case CORE_INT2_PIN:
      irq_num = 2;
      break;
    #endif
    #ifdef CORE_INT3_PIN
    case CORE_INT3_PIN:
      irq_num = 3;
      break;
    #endif
    #ifdef CORE_INT4_PIN
    case CORE_INT4_PIN:
      irq_num = 4;
      break;
    #endif
    #ifdef CORE_INT5_PIN
    case CORE_INT5_PIN:
      irq_num = 5;
      break;
    #endif
    #ifdef CORE_INT6_PIN
    case CORE_INT6_PIN:
      irq_num = 6;
      break;
    #endif
    #ifdef CORE_INT7_PIN
    case CORE_INT7_PIN:
      irq_num = 7;
      break;
    #endif
    #ifdef CORE_INT8_PIN
    case CORE_INT8_PIN:
      irq_num = 8;
      break;
    #endif
    #ifdef CORE_INT9_PIN
    case CORE_INT9_PIN:
      irq_num = 9;
      break;
    #endif
    #ifdef CORE_INT10_PIN
    case CORE_INT10_PIN:
      irq_num = 10;
      break;
    #endif
    #ifdef CORE_INT11_PIN
    case CORE_INT11_PIN:
      irq_num = 11;
      break;
    #endif
    #ifdef CORE_INT12_PIN
    case CORE_INT12_PIN:
      irq_num = 12;
      break;
    #endif
    #ifdef CORE_INT13_PIN
    case CORE_INT13_PIN:
      irq_num = 13;
      break;
    #endif
    #ifdef CORE_INT14_PIN
    case CORE_INT14_PIN:
      irq_num = 14;
      break;
    #endif
    #ifdef CORE_INT15_PIN
    case CORE_INT15_PIN:
      irq_num = 15;
      break;
    #endif
    #ifdef CORE_INT16_PIN
    case CORE_INT16_PIN:
      irq_num = 16;
      break;
    #endif
    #ifdef CORE_INT17_PIN
    case CORE_INT17_PIN:
      irq_num = 17;
      break;
    #endif
    #ifdef CORE_INT18_PIN
    case CORE_INT18_PIN:
      irq_num = 18;
      break;
    #endif
    #ifdef CORE_INT19_PIN
    case CORE_INT19_PIN:
      irq_num = 19;
      break;
    #endif
    #ifdef CORE_INT20_PIN
    case CORE_INT20_PIN:
      irq_num = 20;
      break;
    #endif
    #ifdef CORE_INT21_PIN
    case CORE_INT21_PIN:
      irq_num = 21;
      break;
    #endif
    #ifdef CORE_INT22_PIN
    case CORE_INT22_PIN:
      irq_num = 22;
      break;
    #endif
    #ifdef CORE_INT23_PIN
    case CORE_INT23_PIN:
      irq_num = 23;
      break;
    #endif
  }
#endif

  head = 0;
  tail = 0;
  if (irq_num < 255) {
    attachInterrupt(irq_num, ps2interrupt, FALLING);
  }
  


  K[0x1C] = 4;
  K[0x32] = 5;
  K[0x21] = 6;
  K[0x23] = 7;
  K[0x24] = 8;
  K[0x2B] = 9;
  K[0x34] = 10;
  K[0x33] = 11;
  K[0x43] = 12;
  K[0x3B] = 13;
  K[0x42] = 14;
  K[0x4B] = 15;
  K[0x3A] = 16;
  K[0x31] = 17;
  K[0x44] = 18;
  K[0x4D] = 19;
  K[0x15] = 0x14;
  K[0x2D] = 21;
  K[0x1B] = 22;
  K[0x2C] = 23;
  K[0x3C] = 24;
  K[0x2A] = 25;
  K[0x1D] = 26;
  K[0x22] = 27;
  K[0x35] = 28;
  K[0x1A] = 29;
  K[0x45] = 39;
  K[0x16] = 30;
  K[0x1E] = 31;
  K[0x26] = 32;
  K[0x25] = 33;
  K[0x2E] = 34;
  K[0x36] = 35;
  K[0x3D] = 36;
  K[0x3E] = 37;
  K[0x46] = 38;
  K[0x0E] = 53;
  K[0x4E] = 45;
  K[0x55] = 46;
  K[0x5D] = 49;
  K[0x66] = 42;
  K[0x29] = 44;
  K[0x0D] = 43;
  K[0x5A] = 40;
  K[0x76] = 41;
  K[0x05] = 58;
  K[0x06] = 59;
  K[0x04] = 60;
  K[0x0C] = 61;
  K[0x03] = 62;
  K[0x0B] = 63;
  K[0x83] = 64;
  K[0x0A] = 65;
  K[0x01] = 66;
  K[0x09] = 67;
  K[0x78] = 68;
  K[0x07] = 69;
  K[0x7E] = 57;
  K[0x54] = 47;
  K[0x77] = 83;
  K[0x7C] = 85;
  K[0x7B] = 86;
  K[0x79] = 87;
  K[0x71] = 99;
  K[0x70] = 98;
  K[0x69] = 89;
  K[0x72] = 90;
  K[0x7A] = 91;
  K[0x6B] = 92;
  K[0x73] = 93;
  K[0x74] = 94;
  K[0x6C] = 95;
  K[0x75] = 96;
  K[0x7D] = 97;
  K[0x5B] = 48;
  K[0x4C] = 51;
  K[0x52] = 52;
  K[0x41] = 54;
  K[0x49] = 55;
  K[0x4A] = 56;
  K[0x61] = 0x64; // <>
    
  K[0x58] = 0x39; // KEY_CAPS_LOCK; 
  
  K[0x12] = 102; // MOD_SHIFT_LEFT
  K[0x14] = 101; // MOD_CONTROL_LEFT;
  K[0x11] = 104; //MOD_ALT_LEFT;
  K[0x59] = 102; // MOD_SHIFT_RIGHT;
  K[0x40] = 140; //MOD_ALT_RIGHT;

  
  K[0xF0] = 0xF0;
  K[0xE0] = 0xE0;
  
  
  
  
  
  
}

