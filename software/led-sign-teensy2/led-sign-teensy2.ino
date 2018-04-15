//     --6--
//   |       |
//   5       7
//   |       |
//     --4--     
//   |       |
//   1       3
//   |       |
//     --2--  
//
//  Anzeigeformat:  88:88:88
//  Digitnum:       54 32 10

#include <OctoWS2811.h>
   

// -------------------------
// ------ Definitions ------
// -------------------------

// ------ Definitions ------                          
#define PINSTRIP1    6  // Sekundenanzeige Strip 1
#define PINSTRIP2    6  // Minutenanzeige  Strip 2
#define PINSTRIP3    6  // Stundenanzeige  Strip 3
#define NUMPIXELS  300  // LEDs per Stripe         
#define SEGLEN 7        // LEDs per Segment
#define STDCOLOR 0x0F0F0F
#define HIGCOLOR 0x0F0000
#define ALET1COL 0x0F0F00
#define ALET2COL 0x0F000F
#define ALET3COL 0x050005

// Pins an denen die Buttons angeschlossen sind
#define PINMODEKEY 0
#define PINSETEKEY 1
#define PININCEKEY 23

// ------ Main Final State Maschine -----
enum {cntdwn, settimer};            // cmtdwn - Display zaehlt vom eingestellten wert runter
volatile int mainstate = cntdwn;    // settimer - Anfangswert laesst sich einstellen

// ------ Keys-----
enum {noKey = 0, modMode = 1, resetTimer = 2, modDigit = 3, modValue = 4};
volatile int activeKey = noKey;
//  KeyStates
volatile int keyMod = 0, old_keyMod = 0;
volatile int keySet = 0, old_keySet = 0;
volatile int keyInc = 0, old_keyInc = 0;

// ------ Active mod digit-----
volatile int activeDigit = 0;

// ------ 7 Segment Lookup definition -----
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

int digitColor[6] = {STDCOLOR, STDCOLOR, STDCOLOR, STDCOLOR, STDCOLOR, STDCOLOR};
                       
// ------  Interrupt Timer setup -----
IntervalTimer myTimer, digitFlash;
volatile byte tick = 0;
volatile int timer1_modus = 0;
volatile int timer1_val = 0;

// ------  IOctoWS expansion for Teensy board definitions -----
DMAMEM int displayMemory[NUMPIXELS*6];
int drawingMemory[NUMPIXELS*6];
OctoWS2811 pixels(NUMPIXELS, displayMemory, drawingMemory,  WS2811_GRB | WS2811_800kHz);


// ------ Display Values -----
volatile int sectotal = 0;
volatile int h = 00;
volatile int m = 11;
volatile int s = 00;


// -----------------------
// ------ Functions ------
// -----------------------
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
      pixels.setPixel(n, digitColor[digitPos]* number[num][i]);}
    segOffset = segOffset + SEGLEN;
  }
}

void showTime(){
  setdigit(int(h/10),5);
  setdigit(h%10,4);

  setdigit(int(m/10),3);
  setdigit(m%10,2);

  setdigit(int(s/10),1);
  setdigit(s%10,0);
  pixels.show();
}

void decrementTime(){
  sectotal = h*60*60 + m*60 + s;
  if( sectotal >= 1)
    sectotal--;
  else
    sectotal = 0;
  h = int (sectotal /(60*60));
  m = int ((sectotal- h*60*60)/60);
  s = int ((sectotal- h*60*60 - m*60));
}

void incrementDigit(){

  if( activeDigit == 0){
    if( s%10 != 9)
      s++;
    else
      s = int(s/10)*10;
  } else if( activeDigit == 1){
    if( s < 50)
      s = s +10;
    else
      s = s - 50;
  } else if( activeDigit == 2){
    if( m%10 != 9)
      m++;
    else
      m = int(m/10)*10;
  } else if( activeDigit == 3){
    if( m < 50)
      m = m +10;
    else
      m = m - 50;
  } else if( activeDigit == 4){
    if( h%10 != 9)
      h++;
    else
      h = int(h/10)*10;
  } else if( activeDigit == 5){
    if( h < 50)
      h = h +10;
    else
      h = h - 50;
  }
  
  sectotal = h*60*60 + m*60 + s;
}

void highlightActiveDigit() {
  for(int i = 0; i<6;i++){
    digitColor[i]= STDCOLOR;
  }
  digitColor[activeDigit] = HIGCOLOR;
}

void resetDigitColor() {
  for(int i = 0; i<6;i++){
    digitColor[i]= STDCOLOR;
  }
}

int blinktimer = 0;
void coloriceDigits(){
  int tempColor= ALET2COL;
  
  if( m < 10)
    for(int i = 0; i<6;i++){
    digitColor[i]= ALET1COL;
  }
  if(m == 0 && s < 30){
    blinktimer = (blinktimer+1) %100;
    if(blinktimer >50)
      tempColor = ALET2COL;
    else
      tempColor = ALET3COL;
    for(int i = 0; i<6;i++){
      digitColor[i]= tempColor;
    }
  }
  
}

void ticktack(){
  tick = 1;
}


void setup() {
  Serial.begin(9600);
  Serial.println("Hello World");
  pinMode(PINMODEKEY, INPUT_PULLUP); 
  pinMode(PINSETEKEY, INPUT_PULLUP); 
  pinMode(PININCEKEY, INPUT_PULLUP); 

  // INIT LED Stripe
  pixels.begin();
  pixels.show();
  
  // INIT timer interrupt
  myTimer.begin(ticktack, 1000000);  // setzt jede Sekunde tick auf 1
}

int i = 0, n =0;
void loop() {
// decondign welcher Key gedrückt wurde und wie lange
  // Übernehmen des aktuellen Status
  keyMod = digitalRead(PINMODEKEY);
  keySet = digitalRead(PINSETEKEY);
  keyInc = digitalRead(PININCEKEY);
  
  // Keydeaction
  if(keyMod == 1 && old_keyMod == 0) {   // Steigende Flanke Mode Key
    activeKey = modMode;
  } else if(keySet == 1 && old_keySet == 0) { // geht in den bearbeitungs modus
    activeKey = modDigit;  
  } else if(keyInc == 1 && old_keyInc == 0) { // geht in den bearbeitungs modus
    activeKey = modValue;
  } else {
    activeKey = noKey;
  }
  
  old_keyMod = keyMod;
  old_keySet = keySet;
  old_keyInc = keyInc;
//  Serial.print("mainstate: ");
//  Serial.println(mainstate);

  //  ---  State machine  ---
  switch (mainstate){
    case cntdwn: 
                  coloriceDigits();
                  if( activeKey == modMode) {
                    mainstate = settimer;
                    highlightActiveDigit();
                  }
                  
                  if(tick){
                    tick = 0;
                    decrementTime();
                  }
                  
                  break;
    case settimer:
                  if( activeKey == modMode) {
                    mainstate = cntdwn;
                    resetDigitColor();
                  } else if(activeKey == modDigit){
                    activeDigit = (activeDigit+1)%6;
                    highlightActiveDigit();
                  } else if(activeKey == modValue){
                    incrementDigit();
                  }
                  
                  break;                  
    default: 
                  break;
  }
  
  showTime();
  delay(10);              
}
