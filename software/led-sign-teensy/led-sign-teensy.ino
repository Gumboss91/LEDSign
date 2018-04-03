#include <OctoWS2811.h>


#define NUMPIXELS  300
#define PIN    6
#define SEGLEN 7
#define SETCOLOR 0x0F0000

enum {cntdwn, settimer};
int state = cntdwn;

IntervalTimer myTimer, digitFlash;

DMAMEM int displayMemory[NUMPIXELS*6];
int drawingMemory[NUMPIXELS*6];

OctoWS2811 pixels(NUMPIXELS, displayMemory, drawingMemory,  WS2811_GRB | WS2811_800kHz);

volatile byte prestate = 0;
volatile byte tick = 0;
volatile int numdisp = 55;

//                      1  2  3  4  5  6  7
char number[11][7] = { {1, 1, 1, 0, 1, 1, 1}, // 0
                       {0, 0, 1, 0, 0, 0, 1}, // 1
                       {1, 1, 0, 1, 0, 1, 1}, // 2
                       {0, 1, 1, 1, 0, 1, 1}, // 3
                       {0, 0, 1, 1, 1, 0, 1}, // 4
                       {0, 1, 1, 1, 1, 1, 0}, // 5
                       {1, 1, 1, 1, 1, 1, 0}, // 6 
                       {0, 0, 1, 0, 0, 1, 1}, // 7
                       {1, 1, 1, 1, 1, 1, 1}, // 8
                       {0, 1, 1, 1, 1, 1, 1}, // 9 
                       {0, 0, 0, 0, 0, 0, 0}};// 10 clear

int digitColor[6] = {0x0F0F0F, 0x0F0F0F, 0x0F0F0F, 0x0F0F0F, 0x0F0F0F, 0x0F0F0F};

//
//  digitPos  54 32 10
//  Display   88:88:88
void setdigit(char num, char digitPos){
  int segOffset = 0;
  int n,i =0;

  // 
  if( num <0 || num > 9)
    return;
  if( digitPos <0 || digitPos > 5)
    return;

 
  segOffset = (5-digitPos)*SEGLEN*7;
  
  for (i=0;i<7;i++){
    for(n=segOffset; n< (segOffset + SEGLEN);n++){
      pixels.setPixel(n, digitColor[5-digitPos]* number[num][i]);}
    segOffset = segOffset + SEGLEN;
  }
}

void setDigitColor(char digitPos, int newColor){
  digitColor[5-digitPos] = newColor;
}
int getDigitColor(char digitPos){
  return digitColor[5-digitPos];
}

void setNum(int number){
  if(number < 0 || number > 999999)
    return;
  for(int i = 0; i<6; i++){
    setdigit(int(number%10),i);  
    number = int(number/10);
  }
}

void incSingleDigit(int digitPos){
  int maxdigitnum = 0;
  if( digitPos <0 || digitPos > 5)
    return;
  Serial.print("int(pow(10,digitPos+1)): ");
  Serial.println(int(pow(10,digitPos+1)),DEC);
  
  Serial.print("int(pow(10,digitPos)): ");
  Serial.println(int(pow(10,digitPos)),DEC);

  Serial.print("numdisp: ");
  Serial.println(numdisp,DEC);

  Serial.print("digitPos: ");
  Serial.println(digitPos,DEC);
  
  Serial.print("numdisp + int(pow(10,digitPos)): ");
  Serial.println((numdisp + int(pow(10,digitPos))),DEC);
  
  Serial.print("numdisp+1: ");
  Serial.println(numdisp+1,DEC);
  
  
  maxdigitnum = int(numdisp/(10*digitPos));

  Serial.print("maxdigitnum: ");
  Serial.println(maxdigitnum,DEC);
  
  if( (int(pow(10,digitPos+1)) ) == numdisp+int(pow(10,digitPos+1)) )
    numdisp = numdisp - int(pow(10,digitPos+1));
  else
    numdisp = numdisp + int(pow(10,digitPos));
  
}

void clearsrip(){
   for (int i=0;i<NUMPIXELS;i++){
      pixels.setPixel(i,0x00000);}
}

void ticktack(){
  if(state == cntdwn && numdisp>=0)
    numdisp=(numdisp-1);
}

void blinkdigit(){
  
}

void setup() {
 pinMode(0, INPUT_PULLUP);       // LED
 pinMode(1, INPUT_PULLUP);       // LED
 pinMode(23, INPUT_PULLUP); // Pushbutton
 
 Serial.begin(9600);
 Serial.println("Hello World");
 
 // INIT timer interrupt
 myTimer.begin(ticktack, 1000000);
 digitFlash.begin(blinkdigit, 200000);
 
 // INIT LED Stripe
 pixels.begin(); // This initializes the NeoPixel library.
 pixels.show();
 setNum(0);
 pixels.show();
}


int preButton0State = 1;
int preButton1State = 1;
int preButton23State = 1;
int oldDigitColor = 0x0F0F0F;
int digitSetPos = 0;

void loop() {
//  Serial.println(state,DEC);
  switch (state){
    case cntdwn: 
                  if( preButton0State == 1 &&  digitalRead(0) == 0){
                    state = settimer;
                    digitSetPos = 0;
                    oldDigitColor = getDigitColor(digitSetPos);
                    setDigitColor(digitSetPos, SETCOLOR);
                  }
                  
                  setNum(numdisp);
                  pixels.show();
                  break;
                  
     case settimer: 
                  if( preButton0State == 1 &&  digitalRead(0) == 0){
                    state = cntdwn;
                    setDigitColor(digitSetPos, oldDigitColor);
                  }
                    
                  if( preButton1State == 1 &&  digitalRead(1) == 0)
                   // numdisp = (numdisp + int(pow(10,digitSetPos)))%int(pow(10,digitSetPos+1));
                    incSingleDigit(digitSetPos);
                    
                  
                  if( preButton23State == 1 &&  digitalRead(23) == 0){
                     setDigitColor(digitSetPos, oldDigitColor);
                     digitSetPos = (digitSetPos + 1)%6;
                     oldDigitColor = getDigitColor(digitSetPos);
                     setDigitColor(digitSetPos, SETCOLOR);
                  }
                  
                  setNum(numdisp);
                  pixels.show();
                  break;
    default: break;              
    }
    
    preButton0State = digitalRead(0);
    preButton1State = digitalRead(1);
    preButton23State = digitalRead(23);
   
  
  delay(10);
}
