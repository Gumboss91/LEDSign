#include <Adafruit_NeoPixel.h>
#include "TimerOne.h" 

#define NUMPIXELS  300
#define PIN    6
#define SEGLEN 7

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

volatile byte state = 0;
volatile byte prestate = 0;
volatile byte tick = 0;

//                      1  2  3  4  5  6  7
char number[10][7] = { {1, 1, 1, 0, 1, 1, 1}, // 0
                       {0, 0, 1, 0, 0, 0, 1}, // 1
                       {1, 1, 0, 1, 0, 1, 1}, // 2
                       {0, 1, 1, 1, 0, 1, 1}, // 3
                       {0, 0, 1, 1, 1, 0, 1}, // 4
                       {0, 1, 1, 1, 1, 1, 0}, // 5
                       {1, 1, 1, 1, 1, 1, 0}, // 6
                       {0, 0, 1, 0, 0, 1, 1}, // 7
                       {1, 1, 1, 1, 1, 1, 1}, // 8
                       {0, 1, 1, 1, 1, 1, 1}};// 9 

void setdigit(char num, char numOffset){
  int segOffset = numOffset*SEGLEN*7;
  int n,i =0;
  if( num <0 || num > 9)
    return;
    
  for (i=0;i<7;i++){
    for(n=segOffset; n< (segOffset + SEGLEN);n++){
      pixels.setPixelColor(n, pixels.Color(10,10,10)* number[num][i]);}
    segOffset = segOffset + SEGLEN;
  }
}

void setNum(int number){
  if(number < 0 || number > 999999)
    return;
//  Serial.println(String(number%10));
  setdigit(int(number%10),5);  
  

  number = int(number/10);
  setdigit(number%10,4);
  
//  Serial.print(String(number%10));
  number = int(number/10);
  setdigit(number%10,3);

//  Serial.print(String(number%10));
  number = int(number/10);
  setdigit(number%10,2);

//  Serial.print(String(number%10));  
  number = int(number/10);
  setdigit(number%10,1);

//  Serial.print(String(number%10));
  number = int(number/10);
  setdigit(number%10,0);

//  Serial.print("\n");
}

void clearsrip(){
   for (int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0));}
}

void ticktack(){
  tick = 1;
}

void isrbutton(){
  if(state)
    state = 0;  
   else
    state = 1;
}

void setup() {

 // Serial.begin(9600);
  Serial.println("Hello World");
 // INIT Button interrupt 
 pinMode(2, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(2), isrbutton, FALLING   );

 // INIT timer interrupt
 // Timer1.initialize(1*1000000);
 Timer1.initialize(1*100);
 Timer1.attachInterrupt(ticktack);

 // INIT LED Stripe
 pixels.begin(); // This initializes the NeoPixel library.
 pixels.show();
 setNum(0);
 pixels.show();
}

int i =90;
void loop() {
  int offset =0;

  if(tick) {
    tick = 0;
    i=(i+1)%100000;
//    clearsrip();
//    pixels.show();  

    setNum(i);
    pixels.show();
  }
  
  prestate = state;
  
  delay(200);
}
