#include <OctoWS2811.h>

#define NUMPIXELS  300
#define PIN    6
#define SEGLEN 7
#define SETCOLOR 0x0F0000

// States for the final state maschine
enum {cntdwn, settimer};
int state = cntdwn;

// States for the keypress
enum {noKey = 0, modMode = 1, resetTimer = 2, modDigit = 3, modValue = 4};
int key = noKey;

// Active digit to modefy
enum {secondDigit = 0; minuteDigit = 1, hourDigit = 2};
int activeDigit = secondDigit;

// Physische Keys am Teensy Board
int key1 = 0, old_key1 = 0; // IO 0
int key2 = 0, old_key2 = 0; // IO 1
int key3 = 0, old_key3 = 0; // IO 23

// Interrupt Timer setup
IntervalTimer myTimer, digitFlash;
volatile byte tick = 0;

// OctoWS expansion for Teensy board definitions
DMAMEM int displayMemory[NUMPIXELS*6];
int drawingMemory[NUMPIXELS*6];
OctoWS2811 pixels(NUMPIXELS, displayMemory, drawingMemory,  WS2811_GRB | WS2811_800kHz);

//Display Values
volatile int sectotal = 60*60;
volatile int h = 1;
volatile int m = 0;
volatile int s = 0;

//  --- 7-Segement Definitionen
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
  maxdigitnum = int(numdisp/(10*digitPos));

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


// Timer für lange gehaltene keys
void loop() {
// decondign welcher Key gedrückt wurde und wie lange
  // Übernehmen des aktuellen Status
  key1 = digitalRead(0);
  key2 = digitalRead(1);
  key3 = digitalRead(23);
  // Keydeaction
  if(key1 == 1 && old_key1 == 0) {   // Steigende Flanke Key 1
    key = modMode;
    //Start Timer
    timer1_val = 0;                  // wird Key1 lange gehalten?
    timer1_modus = 1;
  } else if(key1 == 0 && old_key1 == 1) {  // fallende Flanke
    // Stop Timer
    timer1_modus = 0;
    if(timer1_val > 400) { // 
      key = resetTimer;
    }
    timer1_val = 0;
  } else if(key2 && old_key2 == 0) { // geht in den bearbeitungs modus
    key = modDigit;
  } else if(key3 && old_key3 == 0) { // geht in den zählmodus
    key = modValue;
  }
  old_key1 = key1;
  old_key2 = key2;
  old_key3 = key3;

  // Timer for long key press
  if(timer1_modus == 1) {
    timer1_val++;
  }
  
  switch (state){
    case cntdwn: 
                 if(key ==  modMode){
                   state = settimer;
                 }

                 // Display number 
                 if(sectotal-1 > 0 )
                    sectotal = (sectotal -1);
                 
                 h = extractH(sectotal);
                 m = extractM(h, sectotal);
                 s = extractS(h, m, sectotal);
                 
                 break;
                  
    case settimer: 
                 if(key = modDigit){
                   switch(activeDigit) {
                     case secondDigit:  s = (s+1)%60; break;
                     case minuteDigit:  m = (m+1)%60; break;
                     case hourDigit:    h = (h+1)%99; break;
                     default : break;
                   }
                 }else if(key == modValue){
                   activeDigit = (activeDigit+1)%3;
                 }
                 }else if(key == modMode){
                   state = cntdwn;
                 }
                  
                 break;
    default: break;              
    }

  dislay(h, m, s);
  
  delay(10);
}
