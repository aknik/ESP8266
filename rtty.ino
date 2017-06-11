//45.45baud BAUDOT 

#include <stdio.h>

#define AFSK_OUT 12   //---- AFSK AUDIO OUTPUT en ESP8266 pin D6
#define FSK_OUT  13   //---- TTL LEVEL OUTPUT

#define SHIFT     170
#define MARK     2125
#define SPACE    MARK - SHIFT

/*      EU              USA
170 Hz  2125 / 1955 Hz  2295 / 2125 Hz
425 Hz  2125 / 1700 Hz  2295 / 1870 Hz
850 Hz  2125 / 1275 Hz  2295 / 1445 Hz
 85 Hz  2125 / 2040 Hz  2295 / 2210 Hz
255 Hz  2125 / 1870 Hz  2295 / 2040 Hz
340 Hz  2125 / 1785 Hz  2295 / 1955 Hz
680 Hz  2125 / 1445 Hz  2295 / 1615 Hz */

int baud = 22;   // 22ms = 45.5 bd -- 20ms = 50 bd 

/*Baud bit length  char length   char/sec  char/min  words/min
45.5 Bd   22 ms     0.165 s   6     364    60
  50 Bd   20 ms     0.15 s    6.67  400    66
  75 Bd   13.33 ms  0.1 s     10    600   100
 100 Bd   10 ms     0.075 s   13.33 800   133 */
 
boolean d1;
boolean d2;
boolean d3;
boolean d4;
boolean d5;
boolean sSq;
boolean sRd = 1;
boolean space;
boolean fig1;
int     fig2;
byte    i;
byte    j;
byte    idC;
char    ch;
char    sVal[9];

void setup()
{
        pinMode(FSK_OUT,  OUTPUT);
        pinMode(AFSK_OUT, OUTPUT);
        digitalWrite(FSK_OUT, HIGH);
        tone(AFSK_OUT, MARK);
        delay(700);

        Serial.begin(115200);
    

}

void sendFsk()
{
        //--start bit
        digitalWrite(FSK_OUT, LOW);
        tone(AFSK_OUT, SPACE);
        delay(baud);
        //--bit1
        if(d1 == 1){
                digitalWrite(FSK_OUT, HIGH);
                tone(AFSK_OUT, MARK);
        }
        else{
                digitalWrite(FSK_OUT, LOW);
                tone(AFSK_OUT, SPACE);
        }
        delay(baud);
        //--bit2
        if(d2 == 1){
                digitalWrite(FSK_OUT, HIGH);
                tone(AFSK_OUT, MARK);
        }
        else{
                digitalWrite(FSK_OUT, LOW);
                tone(AFSK_OUT, SPACE);
        }
        delay(baud);
        //--bit3
        if(d3 == 1){
                digitalWrite(FSK_OUT, HIGH);
                tone(AFSK_OUT, MARK);
        }
        else{
                digitalWrite(FSK_OUT, LOW);
                tone(AFSK_OUT, SPACE);
        }
        delay(baud);
        //--bit4
        if(d4 == 1){
                digitalWrite(FSK_OUT, HIGH);
                tone(AFSK_OUT, MARK);
        }
        else{
                digitalWrite(FSK_OUT, LOW);
                tone(AFSK_OUT, SPACE);
        }
        delay(baud);
        //--bit5
        if(d5 == 1){
                digitalWrite(FSK_OUT, HIGH);
                tone(AFSK_OUT, MARK);
        }
        else{
                digitalWrite(FSK_OUT, LOW);
                tone(AFSK_OUT, SPACE);
        }
        delay(baud);
        //--stop bit
        digitalWrite(FSK_OUT, HIGH);
        tone(AFSK_OUT, MARK);
        delay(33);
}

void chTable()
{
        fig2 = -1;
        if(ch == ' ')
        {
                d1 = 0; d2 = 0; d3 = 1; d4 = 0; d5 = 0;
                space = 1;
        }
        else if(ch == 'A'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'B'){d1 = 1; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'C'){d1 = 0; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'D'){d1 = 1; d2 = 0; d3 = 0; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'E'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'F'){d1 = 1; d2 = 0; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'G'){d1 = 0; d2 = 1; d3 = 0; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'H'){d1 = 0; d2 = 0; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'I'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'J'){d1 = 1; d2 = 1; d3 = 0; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'K'){d1 = 1; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'L'){d1 = 0; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'M'){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'N'){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'O'){d1 = 0; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'P'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'Q'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'R'){d1 = 0; d2 = 1; d3 = 0; d4 = 1; d5 = 0; fig2 = 0;}
        else if(ch == 'S'){d1 = 1; d2 = 0; d3 = 1; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'T'){d1 = 0; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'U'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 0;}
        else if(ch == 'V'){d1 = 0; d2 = 1; d3 = 1; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'W'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'X'){d1 = 1; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 0;}
        else if(ch == 'Y'){d1 = 1; d2 = 0; d3 = 1; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == 'Z'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 0;}
        else if(ch == '0'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '1'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '2'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '3'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '4'){d1 = 0; d2 = 1; d3 = 0; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == '5'){d1 = 0; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '6'){d1 = 1; d2 = 0; d3 = 1; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '7'){d1 = 1; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '8'){d1 = 0; d2 = 1; d3 = 1; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '9'){d1 = 0; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == '-'){d1 = 1; d2 = 1; d3 = 0; d4 = 0; d5 = 0; fig2 = 1;}
        else if(ch == '?'){d1 = 1; d2 = 0; d3 = 0; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == ':'){d1 = 0; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == '('){d1 = 1; d2 = 1; d3 = 1; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == ')'){d1 = 0; d2 = 1; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;}
        else if(ch == '.'){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == ','){d1 = 0; d2 = 0; d3 = 1; d4 = 1; d5 = 0; fig2 = 1;}
        else if(ch == '/'){d1 = 1; d2 = 0; d3 = 1; d4 = 1; d5 = 1; fig2 = 1;}
        else if(ch == '+'){d1 = 1; d2 = 0; d3 = 0; d4 = 0; d5 = 1; fig2 = 1;} //ITA2
        else
        {
                ch = ' ';
                d1 = 0; d2 = 0; d3 = 1; d4 = 0; d5 = 0;
                space = 1;
        }
}


void loop()
{
  
char* mensaje;

mensaje = "RY RY RY RY RY...... CQ CQ CQ CQ DE    CQ CQ CQ CQ CQ CQ DE    CQ CQ CQ   KKKKKK\0";
rtty(mensaje);


}



void rtty(char* Texto) 
{
  
 
  ch = Texto[j];
  
  if(ch != '\0')
      {
          chTable();
          if(fig1 == 0 && fig2 == 1)
          {
                  d1 = 1; d2 = 1; d3 = 0; d4 = 1; d5 = 1; //FIGURES
          }
          else if(fig1 == 1 && fig2 == 0)
          {
                  d1 = 1; d2 = 1; d3 = 1; d4 = 1; d5 = 1; //LETTERS
          }
          else if(space == 1 && fig2 == 1)
          {
                  d1 = 1; d2 = 1; d3 = 0; d4 = 1; d5 = 1; //FIGURES 
          }
          else
          {
                  j++;
          }
          if(fig2 == 0 || fig2 == 1)
          {
                  space = 0;
                  fig1 = fig2;
          }
          sendFsk();
  }
  if(ch == '\0')
  {
          d1 = 0; d2 = 0; d3 = 0; d4 = 1; d5 = 0; //CR
          sendFsk();
          d1 = 0; d2 = 1; d3 = 0; d4 = 0; d5 = 0; //LF
          sendFsk();
          sSq = 0;
          j = 0;
          noTone(AFSK_OUT);
          delay(60000);
          idC = 0;
                    
            }

  
}




