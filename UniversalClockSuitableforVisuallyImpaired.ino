#define encoderPinA 2
#define encoderPinB 0
#define buttonpin 2
#define loudspeakerpin 1
#include "Time.h"//I did NOT make this library
#include "PinChangeInterruptSimple.h"//And also this library wasn't made by me
volatile  int encoderPos = 0;//position of the encoder, volatile because it changes on interrupt
unsigned int lastReportedPos = 1;   // change management
static boolean rotating=false;      // debounce management
int stepstochangeitem=10; //how many encoder steps are needed for menu item to be changed
boolean A_set = false;              //some debouncing related vars, who knows, I haven't done this part of code
boolean B_set = false;
int items=4;//number of items in menu
int selecteditem=0;
int R=512;//307;//telling which button is pressed-512=pullup resistor has the same value as the button one
int numbertype[]={//roman numerals; if you change this, please also change the numtypesound array abd typecount variable
  50, 10,5,1};
int numbertypesound[]={//pitches of different numerals
  100, 300, 500, 700};
int typecount=4;//number of numerals alltogether
int delaytime=150;
boolean inmenu=true;// are we in menu now?
unsigned long presslength=0;//buttons-for how long has a button been pressed
//time management vars below
long diff=0;//difference between time syncs in millis
long realdiff=0;//same as above, but rounded to half hours
const unsigned long hh=1800000;//half hour in milliseconds
long drift=0;//by how much the realdiff differs from diff
unsigned long lastmillis=0;
unsigned long lastcheckedtime=0;
int onemetervalue=300;//how long (in encoder steps) is one meter
boolean driftenabled=false;//are we correcting time drift?
void setup() {
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(buttonpin, INPUT);
  pinMode(loudspeakerpin, OUTPUT);
  // encoder pin on interrupt 0 (pin 2)
  attachPcInterrupt(2, doEncoderA, CHANGE);
  // encoder pin on interrupt 1 (pin 3)
  attachPcInterrupt(0, doEncoderB, CHANGE); 
  // Serial.begin (9600);
  beepnumber(35);//I just love the sound of that
  /*int tempR=analogRead(buttonpin);
   if (tempR>5)
   {
   R=tempR;
   playSound(1);
   //beepnumber(tempR);
   encoderPos=0;
   selecteditem=0;
   delay(500);
   }*/
}

int setvalue(boolean minutes)//set value; used for clock and egg timer routines
{
  //wrapvalues[]={0,2,1,4};
  encoderPos=0;//reset encoder position
  selecteditem=0;//reset menu position
  int returnvalue=0; //the value to be returned at the end of the function
  int stepnumber=0;//number of current step. each numeral setting=1 step
  int laststepnumber=0;

  while(1)//infinite loop to be broken out of using break command
  {
    inmenu=true;//so that it beeps out menu item, which is here used to determine how many times you want each numeral to be in your desired number
    wrapEncValues();
    inmenu=false;//resets back to appropriate value
    if (stepnumber==0&&minutes==false)//setting hours? Then skip setting 50-value (step 0)
    {
      stepnumber=1;
      laststepnumber=1; 
    }
    if ( laststepnumber==stepnumber)
    {
      if (minutes==false&&stepnumber==2&&returnvalue>=20)//skipping value 5 setting if user is setting hours and has set them to be >20 (setting 25:00 as time wouldn't make sense)
        stepnumber=3;
      if (minutes==true&&stepnumber==1&&returnvalue>=50)//skipping value 10 setting if user is setting minutes and has set them to be >50 (59 is the max value for minutes)
        stepnumber=2;
      beepnumber(numbertype[stepnumber]);
      laststepnumber=-1; 
    }
    if (minutes==false)
    {
      switch (stepnumber)
      {
      case 1://10s  
        items=2;
        break;
      case 2://5s
        if (returnvalue>=20)//20-something value
          stepnumber=3;//skip 5s     
        items=1;
        break;
      case 3://1s
        if (returnvalue>=20)//20-something value
        {
          items=3;
        }
        else
          items=4;
        break;
      }
    }
    else
    {
      switch (stepnumber)
      {
      case 0://50s  
        items=1;
        break;
      case 1://10s
        if (returnvalue>=50)//50-something value
        {
          stepnumber=2;//skip 10s     
          break; 
        }
        items=4;
        break;
      case 2://5s
        items=1;
        break;
      case 3://1s
        items=4;
        break;
      }
    }

    if (buttonpressed()==1)
    {
      returnvalue+=numbertype[stepnumber]*selecteditem;
      stepnumber+=1;
      selecteditem=0;
      laststepnumber=stepnumber;
      if (stepnumber>3)
      {

        inmenu=true;
        stepstochangeitem=10;
        selecteditem=1;//back to menu
        items=4;//not really necessary since "items" allready will have this value by coincidence
        return returnvalue;
        break;
      } 
    }
  }
}
void wrapEncValues()
{
  int beepnum=-1;
  while (encoderPos>stepstochangeitem)
  {
    encoderPos=encoderPos-stepstochangeitem;
    selecteditem+=1;
    beepnum=selecteditem;

  }
  while (encoderPos<0)
  {
    encoderPos=stepstochangeitem+encoderPos;
    selecteditem-=1;
    beepnum=selecteditem;
  }

  if (selecteditem>items)
  {
    selecteditem=0;
    beepnum=selecteditem; 
  }
  if (selecteditem<0)
  {
    selecteditem=items;
    beepnum=selecteditem;
  }
  if (beepnum!=-1&&inmenu==true)
    beepnumber(beepnum);

}
void playSound(int soundtype)//routine for beeping out specific codes (for zero, entering program, alarm and "OK beep" which is also used for separating hours and minutes when displaying time
{
  switch (soundtype)
  {
  case 0://enter
    /* for (int i=0; i<=100; i++)
     {
     tone(loudspeakerpin, i*5);
     delay(5);
     }
     noTone(loudspeakerpin);
     */
    tone(loudspeakerpin, 500,250);
    delay(100);
    tone(loudspeakerpin, 800,250);
    delay(500);
    break;
  case 1://melody
    tone(loudspeakerpin, 200,250);
    delay(100);
    tone(loudspeakerpin, 500,500);
    delay(50);
    break;
  case 2://zero
    tone(loudspeakerpin, 950,250);
    delay(50);
    tone(loudspeakerpin, 250,250);
    break;
  case 3://dot
    delay(100);
    tone(loudspeakerpin, 950,250);
    delay(100);
    noTone(loudspeakerpin);
    tone(loudspeakerpin, 950,250);
    delay(400);
    break;
  }
}

void loop()
{ //Do stuff here
  /*pinMode(13,OUTPUT);
   digitalWrite(13,HIGH);
   delayMicroseconds(100000);
   digitalWrite(13,LOW);*/


  if (driftenabled==true)//auto time correction enabled
  {
    boolean setlastcheckedtime=false;
    while (now()-lastcheckedtime>3600)//more than hour difference since last sync time
    {
      setlastcheckedtime=true;//sync happened
      lastcheckedtime+=3600;
      adjustTime(drift/1000);
    }
    if (setlastcheckedtime== true)//set the time of last sync only if the sync actually happened
   lastcheckedtime=now()-(drift/1000);
  }
  //moved code
  wrapEncValues();
  //if (Serial.read()=='B')
  // Serial.println (encoderPos, DEC);    


  if (inmenu==true&&buttonpressed()==1)
  {
    inmenu=false;
    playSound(0);
    switch (selecteditem)
    {

    case 0://clock
      if (presslength<2000)//press wasn't longer than 2 secs
      {
        beepnumber(hour());
        delay(200);
        playSound(3);
        delay(200);
        beepnumber(minute());
        inmenu=true;
        break;
      }
      else //longer than 2 secs
      {
        if (presslength>5000)//longer than 5 secs
        {
          if (lastmillis!=0)
            driftenabled=true;
          playSound(3);
          //code below calculates drift of internal oscillator by comparing expected delay
          //(which can be easily estimated, since user has to make the sync at :30 or :00) to gained value.
         // Then calculates drift, nominates it to one hour and saves to "drift" variable
          diff=millis()-lastmillis;
          unsigned long compval=hh/2;
          int i;
          for (i=1; i<=10000; i++)
          {
            compval+=hh;
            if (diff<=compval)
              break;
          }
          realdiff=i*hh;
          drift=realdiff-diff;
          drift=(2*drift)/(realdiff/hh);
          lastmillis=millis();
          int hrz=hour();
          int minz;
          if (minute()>15&&minute()<45)//assume the clock isn't off by more than 15 minutes
            minz=30;
          else
          {
            if (minute()>45)
              hrz+=1;
            minz=0;
          }
          setTime(hrz,minz,0,1,1,1);

        } 

        else
        {

          int hrz=setvalue(false);
         
          int minz=setvalue(true);
          setTime(hrz,minz,0,1,1,1);
          driftenabled=false;
          lastmillis=0;
      lastcheckedtime=now(); 
      }
        inmenu=true;
        break;
      }
      break;
    case 1://tape measure
      encoderPos=0;
      while(1)
      {
        // wrapEncValues();
        if (buttonpressed()==1)
        {
          if (presslength<2000)
          {
            int cms=round(map(abs(encoderPos),0,onemetervalue,0,100));
            beepnumber(cms);
            inmenu=true;
            stepstochangeitem=10;
            selecteditem=1;//back to menu
            items=4;
            encoderPos=0;
            break;
          }
          else//assume user has measured exactly one meter and wants to calibrate the tape measure
          {
            playSound(3);
            onemetervalue=abs(encoderPos);
            encoderPos=0;
            inmenu=true;
            break;
          } 
        } 
      }
      break;
    case 2://water detector
      {
        stepstochangeitem=4;
        items=10;
        selecteditem=5;
        while(1){
          wrapEncValues();
          if (abs(selecteditem-5)>2)//the probe uses same input pin as buttons; therefore, to exit the water detector, user is obliged to twist the encoder wheel
          {
            inmenu=true;
            stepstochangeitem=10;
            encoderPos=0;
            selecteditem=2;//back to menu
            items=4;
            noTone(loudspeakerpin);
            break;
          }
          tone(loudspeakerpin,map(analogRead(buttonpin),0,512,100,1000));//0-256;100-500
        }
        break;
      }
    case 3://stopwatch
      {
        int startingsecs;
        boolean watchrunning=false;
        while (1)
        {
          if(buttonpressed()==1)
          {
            playSound(3);
            if (watchrunning==false)
            {
              watchrunning=true;
              startingsecs=millis()/1000;
            }
            else
            {
              watchrunning=false;
              beepnumber(millis()/1000-startingsecs);
              inmenu=true;
              selecteditem=3;//back to menu
              break;

            }
          }
        }
        break;
      }
    case 4://egg timer
      int minutemins=99;
      while (1)
      {
        int buttoncode=buttonpressed();//so that it doesn't have to be sampled twice
        if (buttoncode==0)
          break;
        if (buttoncode==1)
        {
          minutemins=setvalue(true);
          minutemins=(minute()+minutemins)%60;//to ensure it will work correctly even if hour changes while the egg timer is running (which isn't really possible now when I think about it...)
          playSound(3);
        }
        if (minute()==minutemins)
        {
          playSound(1);
          if (buttoncode==0)
            break;
        } 
      }

      break;

    }
  }
}

//Until further notice, the code below is NOT mine
void doEncoderA(){
  // debounce
  if ( rotating ) delay (1);  // wait a little until the bouncing is done

  // Test transition, did things really change?
  if( digitalRead(encoderPinA) != A_set ) {  // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if ( A_set && !B_set )
      encoderPos += 1;

    rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB(){
  if ( rotating ) delay (1);
  if( digitalRead(encoderPinB) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if( B_set && !A_set )
      encoderPos -= 1;

    rotating = false;
  }
}
//from now on, the code is mine again

int buttonpressed()
{
  if (analogRead(buttonpin)>5)//is anything going on at all?
  {
    int returnvalue=-1;
    unsigned long beginms=millis();//variable for later comparation to tell for how long has the button been pressed
    while(1)
    {
      unsigned long lastmillis;

      int value=analogRead(buttonpin);
      if (value>5)//should be 0, but some hysteresion never hurts
      {
        lastmillis=millis();
        if (abs(value-((2*R)/3))<10)//button 2
          returnvalue=0;
        else
          if (abs(value-R)<10)//button 1
          {
            returnvalue=1;
            // R=value;
          }
          else
            if (abs(value-((2*R)/5)*3)<10)//both buttons, doesn't really work
              returnvalue=2;
      }
      else
        if (millis()-lastmillis>20)//to do the debouncing; count button as released once it has been released for more than 20ms
        {
          presslength=millis()-beginms;
          return returnvalue;
          break;
        }
    }
  }
  else
    return -1;
}

int beepnumber(int num) //beeping out the number. Basically works pseudorecursively by dividing the argument by numerals from highest to lowest
//and then feeds the modulo of division back to itself and repeats the whole process
{
  if (num==0)
  {
    playSound(2);//special code for zero
    return 0;
  }
  int initpos=encoderPos;
  int numbertypevalue[]={
    -1, -1, -1 ,-1                                             };
  for(int i=0;i<typecount;i++)
  {
    if (num/numbertype[i]>=1)
    {
      numbertypevalue[i]=floor(num/numbertype[i]);
      num=num%numbertype[i];
    }

    encoderPos=initpos;//basically ignore user rotating the encoder if beeping out the value
    if (numbertypevalue[i]>-1)
    {
      for(int i2=0;i2<numbertypevalue[i];i2++)
      {
        for (int i3=0;i3<4-i;i3++)
        {
          delay(delaytime/2);
          tone(loudspeakerpin, numbertypesound[i],delaytime);
          delay(delaytime);
        }
        delay(delaytime*2); 
      }
    }

  }

}
