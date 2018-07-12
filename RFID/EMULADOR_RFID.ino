
#include <ESP8266WiFi.h>

ADC_MODE(ADC_VCC);

#define RF_PIN D1
#define ACTIVITY_LED D4

static char DEBUGMODE=0;
//helpers for debugging timings
int32_t starttime, stoptime;

static inline int32_t asm_ccount(void) {
    int32_t r; asm volatile ("rsr %0, ccount" : "=r"(r)); return r; }

static inline void asm_nop() {
    asm volatile ("nop"); }

// Variables used for temporary storage
static uint8_t data[64];
static uint8_t value[10];
static String webResponse;

// Variables used for state keeping
static uint8_t delayed_setup = 1;
static uint8_t state=1;
static uint8_t active_id=1;
static uint8_t network_count=0;

uint8_t  vendor_active=255;
uint32_t ID_active= 4294967295;
//uint32_t ID_active=  268435455;

void prepare_data(uint32_t ID, uint32_t VENDOR){
  value[0] = (VENDOR>>4) & 0XF;
  value[1] = VENDOR & 0XF;
  for (int i=0; i<8; i++){
    value[i+2] = (ID>>(28-i*4)) &0xF;
  }

  for (int i=0; i<9; i++) data[i]=1; //header
  for (int i=0; i<10; i++) {         //data
    for (int j=0; j<4; j++) {
      data[9 + i*5 +j] = value[i] >> (3-j) & 1;
    }
    data[9 + i*5 + 4] = ( data[9 + i*5 + 0]
                        + data[9 + i*5 + 1]
                        + data[9 + i*5 + 2]
                        + data[9 + i*5 + 3]) % 2;
  }
  for (int i=0; i<4; i++) {          //checksum
    int checksum=0;
    for (int j=0; j<10; j++) {
      checksum += data[9 + i + j*5];
    }
    data[i+59] = checksum%2;
  }
  data[63] = 0;                      //footer

  /*
  delay(10);
  Serial.println();
  for (int i=0; i<64; i++) {
    Serial.printf("%d", data[i]);
    if (i>=8 && (i+2)%5==0) Serial.printf("\r\n    ");
  }
  Serial.println();
  delay(10);
  */
}


void setup() {
  
  WiFi.mode( WIFI_OFF );
  
  Serial.begin(115200);
  
  prepare_data(ID_active, vendor_active);

}

void loop() {

  digitalWrite (ACTIVITY_LED, LOW);

  delay(10);
  int i=0, j=0;
  //Manchester
  if (active_id){
    for (i=0; i<15; i++){
      for (j=0; j<64; j++){

//        data[j]? pinMode(RF_PIN, OUTPUT):pinMode(RF_PIN, INPUT);
//        data[j]? (GPE |= 0b00000100):(GPE &= ~0b00000100);
        data[j]? (GPE |= (1<<RF_PIN)):(GPE &= ~(1<<RF_PIN));
        delayMicroseconds(255);
        for (int k=0; k<14; k++) asm_nop(); //fine tuning

//        data[j]? pinMode(RF_PIN, INPUT):pinMode(RF_PIN, OUTPUT);
//        data[j]? (GPE &= ~0b00000100):(GPE |= 0b00000100);
        data[j]? (GPE &= ~(1<<RF_PIN)):(GPE |= (1<<RF_PIN));
        delayMicroseconds(255);
        for (int k=0; k<13; k++) asm_nop(); //fine tuning

      }
    }
    digitalWrite (ACTIVITY_LED, HIGH);
  }
}
