
#include <FastLED.h>
#include <LinkedList.h>


//Declare Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1
#define NUM_LEDS 60

// Data pin that led data will be written out over (green wire)
#define DATA_PIN 11

// Clock pin (blue wire)
#define CLOCK_PIN 13


#define interruptPin 2
volatile int counter = 0;
const int modes = 3; 

//declare LED array
CRGB leds[NUM_LEDS];
enum State {
  music,
  standby,
};
//Define spectrum variables
int freq_amp;
int frequency[7];

//Define variables used for ranking function
int freqRank[7];
int litLEDS[7];
int rank[7];
int cmpfunc (const void * a, const void * b)        //function used in qsort
{
  return ( *(int*)a - * (int*)b );
}

int averageAmp;
int tempo = 200;
int avgBeatLength;
LinkedList<int> tempoList = LinkedList<int>();
boolean musicOn = false;
unsigned long loopEndTime1 = 0;
unsigned long loopEndTime2 = 0;
int max;
boolean check0 = false;
unsigned long avgBandThree = 0;
unsigned long avgBandFour = 0;
unsigned long avgBandFive = 0;
unsigned long avgMax = 0;


//variables for standby pattern
int r[] = {75, 0, 0};       //red value is an array, one value for each LED, first LED initiall red
int b = 0;      //blue color value
int g = 0;     //green color value
int n = 0;

//variables for stripe pattern
int rStripe[] = {0, 0, 0, 0, 0, 0};
int gStripe[] = {0, 0, 0, 0, 0, 0};
int bStripe[] = {0, 0, 0, 0, 0, 0};
int horLine[6];
int advancePattern = 0;
unsigned long intervalStripe = 130;

//variables for ring pattern
int averagePitch = 0;
int r1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int g1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int g2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int b3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int g3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int b4[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int r5[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int b5[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int r6[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long intervalRing = 60;         //determines speed of animation
int tempoCount = 0;
int tempoStripeCount = 0;

//tempo variables
unsigned long intervalRandom = 30;
unsigned long previousMillis = 0;


//case variable
boolean isRandom = false;
boolean isStripes = false;
boolean isRing = false;


/********************Setup Loop*************************/
void setup() {
  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);
  pinMode(2, OUTPUT);
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);


  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(STROBE, HIGH);
  delay(1);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, LOW);
  
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), interrupted, CHANGE);

  //adds LEDs
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);

  //turns off LEDs on strip
  for (int j = 0; j < NUM_LEDS; j++) {
    leds[j] = CRGB(0, 0, 0);
    FastLED.show();
  }
}



/**************************Main Function Loop*****************************/
State mode = standby;
int deltaTime = 3000;
unsigned long timeMusicCheck = 0;
void loop() {
  //Also gathers data for this run through
  determineAmpRange();


  if (millis() > loopEndTime2) {
    check0 = isMusicOn();
    loopEndTime2 += deltaTime;
    timeMusicCheck += deltaTime + 2000;
  }
  calculateTempo();
  mode = musicOrStandby(mode);

}
void calculateTempo() {
  for (int i = 0; i < 60; i++) {
    noteTiming(i);
  }

  calculateBeatLength();
  if (tempoList.size() > 5) {
    int a = tempoAvg();
    tempo = 60000.0 / a;
  }
}


/*
   Lighting Patterns
*/

/*****************Random dot pattern*****************/
void Random_Dots() {
  Rank_Frequencies(freqRank);    //ranks the frequencies
  int lit = 0;                   //used to index which led is lit
  //resets all LEDs to off each loop
  for (int off = 0; off < NUM_LEDS; off++) {
    leds[off] = CRGB(0, 0, 0);
  }
  //first frequency, generates a proportional number of LEDs to its frequency rank
  for (int freq0 = 0; freq0 < (litLEDS[0] * ((frequency[0] - averagePitch)/35)); freq0++) {
    lit = (int)random(30, 60);
    leds[lit] = CRGB(130, 130, 0);
  }
  for (int freq1 = 0; freq1 < (litLEDS[1] * ((frequency[1] - averagePitch)/35)); freq1++) {
    lit = (int)random(25, 60);
    leds[lit] = CRGB(0, 150, 0);
  }
  for (int freq2 = 0; freq2 < (litLEDS[2] * ((frequency[2] - averagePitch)/35)); freq2++) {
    lit = (int)random(20, 50);
    leds[lit] = CRGB(0, 130, 130);
  }
  for (int freq3 = 0; freq3 < (litLEDS[3] * ((frequency[3] - averagePitch)/35)); freq3++) {
    lit = (int)random(15, 45);
    leds[lit] = CRGB(0, 0, 120);
  }
  for (int freq4 = 0; freq4 < (litLEDS[4] * ((frequency[4] - averagePitch)/35)); freq4++) {
    lit = (int)random(10, 40);
    leds[lit] = CRGB(69, 0, 130);
  }
  for (int freq5 = 0; freq5 < (litLEDS[5] * ((frequency[5] - averagePitch)/35)); freq5++) {
    lit = (int)random(0, 35);
    leds[lit] = CRGB(150, 0, 0);
  }
  for (int freq6 = 0; freq6 < (litLEDS[6] * ((frequency[6] - averagePitch)/35)); freq6++) {
    lit = (int)random(0, 30);
    leds[lit] = CRGB(150, 60, 0);
  }
}

/******************Ring Pattern**************************/

//assigns color values to variables for ring pattern
void Ring_Dots() {
  //if highest frequency is above threshold, light that ring
  if (frequency[0] > averagePitch + 75){
    for (int light = 0; light < 10; light++){
       r1[light] = 130;
       g1[light] = 130;
    }
   }
    else {
      //turn off the LEDs on that ring
      for (int light = 0; light < 10; light++){
       r1[light] = 0;
       g1[light] = 0;
      }
    }
    if (frequency[1] > averagePitch + 75){ 
      for (int light = 0; light < 10; light++){
        g2[light] = 150;
      }
    }
    else{
      for (int light = 0; light < 10; light++){
        g2[light] = 0;
      }
    }
    //higher threshold for the lower bands, because they are more prone to giving larger amplitude values at similar perceived volume
    if (frequency[2] > (averagePitch + 150)){
      for (int light = 0; light < 10; light++){
        g3[light] = 130;
        b3[light] = 130;
      }
    }
    else {
      for (int light = 0; light < 10; light++){
        g3[light] = 0;
        b3[light] = 0;
      } 
    }
    if (frequency[3] > (averagePitch + 150)){ 
      for (int light = 0; light < 10; light++){
        b4[light] = 120;
      }         
    }
    else {
       for (int light = 0; light < 10; light++){
        b4[light] = 0;
      }  
    }
    if (frequency[4] > (averagePitch + 150)){
      for (int light = 0; light < 10; light++){
        r5[light] = 69;
        b5[light] = 130;
      }  
    }
    else{
      for (int light = 0; light < 10; light++){
        r5[light] = 0;
        b5[light] = 0;
      } 
    }
   if (frequency[5] > (averagePitch + 150)){
    for (int light = 0; light < 10; light++){
        r6[light] = 150;
      }  
    }
    else {
      for (int light = 0; light < 10; light++){
        r6[light] = 0;
      }
    }
  Assign_LEDs();

}
//puts the right color value, defined in Ring_Dots() to the right LED 
void Assign_LEDs() {
  int lit = 0;
  //first stripe, bottom 10 LEDs on the cylinder
   for (int stripe6 = 0; stripe6 < 10; stripe6++){
    lit = stripe6;
    leds[lit] = CRGB(r6[stripe6], 0, 0); //color variable is r6, the color that corresponds with the lowest frequency
   }
   //second stripe
   for (int stripe5 = 0; stripe5 < 10; stripe5++){
    lit = stripe5 + 10;
    leds[lit] = CRGB(r5[stripe5], 0, b5[stripe5]); 
   }
   for (int stripe4 = 0; stripe4 < 10; stripe4++){
    lit = stripe4 + 20;
    leds[lit] = CRGB(0, 0, b4[stripe4]); 
   }
   for (int stripe3 = 0; stripe3 < 10; stripe3++){
    lit = stripe3 + 30;
    leds[lit] = CRGB(0, g3[stripe3], b3[stripe3]); 
   }
   for (int stripe2 = 0; stripe2 < 10; stripe2++){
    lit = stripe2 + 40;
    leds[lit] = CRGB(0, g2[stripe2], 0); 
   }
   //last stripe, highest 10 LEDs on the cylinder 
   for (int stripe1 = 0; stripe1 < 10; stripe1++){
      lit = stripe1 + 50;   
      leds[lit] = CRGB(r1[stripe1], g1[stripe1], 0);
    }
}

/***********Frequency Ranking************/
void Rank_Frequencies (int a[]) {
  qsort(a, 7, sizeof(int), cmpfunc);      //sorts the freqRank[], the array of frequency values
  for (int i = 0; i < 7; i++) {
    //if there is minimal sound in frequency band, litLEDs is equal to 0 
    if (frequency[i] < 100) {
      litLEDS[i] = 0;
    }
    //compares frequency[] to freqRank[], if it is the maximum value, litLEDs for that index is equal to 6
    else if (frequency[i] == freqRank[6]) {
      litLEDS[i] = 6;
    }
    else if (frequency[i] == freqRank[5]) {
      litLEDS[i] = 5;
    }
    else if (frequency[i] == freqRank[4]) {
      litLEDS[i] = 4;
    }
    else if (frequency[i] == freqRank[3]) {
      litLEDS[i] = 3;
    }
    else if (frequency[i] == freqRank[2]) {
      litLEDS[i] = 2;
    }
    else if (frequency[i] == freqRank[1]) {
      litLEDS[i] = 1;
    }
    //if that frequency band has the smallest value, litLEDs for that band is equal to 0
    else {
      litLEDS[i] = 0;
    }
  }
}

/****************Stripe Pattern*******************/
void Stripes(){
    Rank_Frequencies(freqRank);   //rank the frequncies
    Stripe_Color();               //call the function that defines the color variables
      
   int lit = 0;

    //Stripe at top of the cube, gets color from index 5 for each color variable
   for (int stripe6 = 0; stripe6 < 10; stripe6++){
    lit = stripe6 + 50;
    leds[lit] = CRGB(rStripe[5], gStripe[5], bStripe[5]); 
   }
   for (int stripe5 = 0; stripe5 < 10; stripe5++){
    lit = stripe5 + 40;
    leds[lit] = CRGB(rStripe[4], gStripe[4], bStripe[4]); 
   }
   for (int stripe4 = 0; stripe4 < 10; stripe4++){
    lit = stripe4 + 30;
    leds[lit] = CRGB(rStripe[3], gStripe[3], bStripe[3]); 
   }
   for (int stripe3 = 0; stripe3 < 10; stripe3++){
    lit = stripe3 + 20;
    leds[lit] = CRGB(rStripe[2], gStripe[2], bStripe[2]); 
   }
   for (int stripe2 = 0; stripe2 < 10; stripe2++){
    lit = stripe2 + 10;
    leds[lit] = CRGB(rStripe[1], gStripe[1], bStripe[1]); 
   }
   //stripe at bottom of the cube, gets the color from index 0 from color variables 
   for (int stripe1 = 0; stripe1 < 10; stripe1++){
      lit = stripe1;   
      leds[lit] = CRGB(rStripe[0], gStripe[0], bStripe[0]);
    }
    
 }
 
 void Stripe_Color(){

  //every beat, the stripe color variable takes its value from the index before it. This gives the appearance of stripes advancing up the cube
    if (tempoStripeCount % 8 == 0){
      for (int c = 5; c > 0; c--){
        rStripe[c] = rStripe[c-1];
        bStripe[c] = bStripe[c-1];
        gStripe[c] = gStripe[c-1];
      }
    }

  
  //for strongest note, defining color

  //if the highest frequency band is the loudest, the color is set to yellow 
    if (litLEDS[0] == 6){
       rStripe[0] = 100;
       gStripe[0] = 100; 
       bStripe[0] = 0;
    }
    // if second highest frequency band is loudest, the color is set to green
    else if (litLEDS[1] == 6){
       rStripe[0] = 0; 
       gStripe[0] = 120; 
       bStripe[0] = 0; 
    }
    else if (litLEDS[2] == 6){
       rStripe[0] = 0; 
       gStripe[0] = 100; 
       bStripe[0] = 100; 
    }
    else if (litLEDS[3] == 6){
       rStripe[0] = 0; 
       gStripe[0] = 0; 
       bStripe[0] = 90; 
    }
    else if (litLEDS[4] == 6){
        rStripe[0] = 50; 
        gStripe[0] = 0; 
        bStripe[0] = 100; 
    }
   else if (litLEDS[5] == 6){
        rStripe[0] = 120; 
        gStripe[0] = 0; 
        bStripe[0] = 0;
    }
   else if (litLEDS[6] == 6){
        rStripe[0] = 120; 
        gStripe[0] = 30; 
        bStripe[0] = 0;
   }
   else {
    rStripe[0] = 0; 
    gStripe[0] = 0;
    bStripe[0] = 0;
   }
   
   //second strongest note has small affect on color of the stripe 
   //if the second strongest note is the highest frequency, then band is essentially made a little more yellow
   if (litLEDS[0] == 5){
       rStripe[0] += 15;
       gStripe[0] += 15; 
    }
    else if (litLEDS[1] == 5){ 
       gStripe[0] += 20; 
    }
    else if (litLEDS[2] == 5){
       gStripe[0] += 15; 
       bStripe[0] += 15; 
    }
    else if (litLEDS[3] == 5){
       bStripe[0] += 1; 
    }
    else if (litLEDS[4] == 5){
        rStripe[0] += 5;  
        bStripe[0] += 15; 
    }
   else if (litLEDS[5] == 5){
        rStripe[0] += 5;
        gStripe[0] -= 10; 
        bStripe[0] -= 10; 
    }
    //if lowest frequency band is second highest, the stripe is more dim 
   else if (litLEDS[6] == 5){
        rStripe[0] -= 20; 
        gStripe[0] -= 20;
        bStripe[0] -= 20; 
   }
 }

void standbyPattern() {
  for (int j = 0; j < 3; j++) {      //cycles through 3 LEDs
    if (j < 2) n = j + 1;            //advances to next LED 0 -> 1 -> 2 -> 0
    else n = 0;
    for (int i = 0; i < 45; i++) {   //cycles through 60 color changes
      if ( i / 15 == 0) {    //second 15 loops, red is decreasing and green is increasing
        r[j] = r[j] - 5;
        g = g + 10;
      }
      else if (i / 15 == 1) {     //third 15 loops, green is decreasing and blue is increasing
        g = g - 10;
        b = b + 10;
      }
      else {
        b = b - 10;       //last 15 loops, blue is decreasing, the red of the next LED is increasing
        r[n] = r[n] + 5;
      }
      for (int k = 0; k < 20; k++) {
        leds[j + 3 * k] = CRGB( r[j], g, b);
        leds[n + 3 * k] = CRGB(r[n], 0, 0);
        FastLED.show();
      }
    }
  }
}

/*
   FSM
*/


State musicOrStandby(State state) {

  switch (state) {
    case music:
      if (millis() > loopEndTime1) {
        if (counter % modes == 0) {
          Random_Dots();
          loopEndTime1 += (tempo/4);
        } 
        else if (counter % modes == 1) {
          Stripes(); 
          loopEndTime1 += (tempo / 4);
          //loopEndTime1 += tempo;
          tempoStripeCount++;
        }
        else {
          Ring_Dots();
          //loopEndTime1 += (tempo / 20);
          loopEndTime1 += tempo/4;
          tempoCount++; 
        }
        FastLED.show();
      }
      if (millis() > timeMusicCheck) {
        if (isMusicOn() || check0) {
          state = music;
        }
        else if (!isMusicOn() && !check0) {
          state = standby;
        }
      }
      break;
    case standby:
      standbyPattern();
      if (!isMusicOn() && !check0) {
        state = standby;
      }
      else if (isMusicOn() || check0) {
        state = music;
      }
      break;
  }
  return state;
}

/*
   MUSIC DETECTION
*/

int detectAmp = 80;
int detectBassAmp = 100;
boolean isMusicOn() {
  boolean shouldChange = false;
  if (((avgBandFour >= detectAmp && avgBandFive >= detectAmp)||(avgBandThree >= 4*detectAmp && avgMax >= 2*detectAmp)) || averageAmp >= detectBassAmp) {
    musicOn = true;
  }
  int it = 0;
  while (it < 5) {
    it++;
  }
  if ((((avgBandFour >= detectAmp || avgBandFive >= detectAmp) || (avgBandThree >= 4*detectAmp || avgMax >= 2*detectAmp)) || averageAmp >= detectBassAmp) && musicOn) {
    shouldChange = true;
    musicOn = false;
  }
  else {
    musicOn = false;
  }
  return shouldChange;
}

int Read_Frequencies() {
  averagePitch = 0;
  max = 0;
  for (freq_amp = 0; freq_amp < 7; freq_amp++)
  {
    frequency[freq_amp] = analogRead(DC_Two);
    averagePitch = averagePitch + (frequency[freq_amp] / 7.0);  //adds value to averagePitch
    
    freqRank[freq_amp] = frequency[freq_amp];
    if (max < frequency[freq_amp]) max = frequency[freq_amp];
    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
  avgMax += max;
  avgBandThree += frequency[3];
  avgBandFour += frequency[4];
  avgBandFive += frequency[5];
  int a = frequency[1];
  return a;
}
int bassValues[60];
int bassValuesMillis[60];
unsigned long t0;
unsigned long t1;
int filteredFrequencies(int a) {
  int avg = 0;
  for (int b = 0; b < 10; b++) {
    bassValues[b + (10 * a)] = Read_Frequencies();
    bassValuesMillis[b + (10 * a)] = millis();
    avg += bassValues[b + (10 * a)];
  }
  double b = (double)avg / 10.0;
  return (int)b;
}
void determineAmpRange() {
  double avg = 0;
  
  for (int a = 0; a < 6; ++a) {
    double f = filteredFrequencies(a);
    if (f > max) max = f;
    avg += f;
  }
  avgMax = ((double)avgMax/60.0);
  avgBandThree = ((double)avgBandThree / 60.0);
  avgBandFive = ((double)avgBandFive / 60.0);
  avgBandFour = ((double)avgBandFour / 60.0);
  averageAmp = ((double)avg / 6.0);
}
boolean prePeak = true;
boolean notPeak = false;
boolean inPeak = false;
LinkedList<int> beatLengths = LinkedList<int>();
void noteTiming(int b) {
  if (bassValues[b] < averageAmp && prePeak) {
    notPeak = true;
    prePeak = false;
    t0 = bassValuesMillis[b];
  }
  else if (bassValues[b] > averageAmp && notPeak) {
    prePeak = false;
    inPeak = true;
    notPeak = false;
  }
  else  {
    if (bassValues[b] < averageAmp && inPeak) {
      t1 = bassValuesMillis[b];
      int a = abs(t1 - t0);
      if (a > 0) {
        beatLengths.add(a);
      }
      inPeak = false;
      notPeak = false;
      prePeak = true;
    }
  }
}
void calculateBeatLength() {
  double long avg = 0;
  int len = beatLengths.size();
  if (len > 50) {
    for (int b = 0; b < (len - 50); ++b) {
      beatLengths.remove(0);
    }
  }
  for (int b = 0; b < beatLengths.size(); b++) {
    avg += abs(beatLengths.get(b));
  }
  avg = avg / beatLengths.size();
  avgBeatLength = avg;
  if (avg < 70) avg *= 48;
  long a = abs((double)60000.0 / avg);
  //if (averageAmp<a) a*=(a/averageAmp);
  if (a >= 0 && a <= 220) {
    tempoList.add(a);
  }
}
int tempoAvg() {
  double avg = 0;
  int len = tempoList.size();
  if (len > 20) {
    for (int a = 0; a < (len - 20); ++a) {
      tempoList.remove(0);
    }
  }
  for (int a = 0; a < tempoList.size(); ++a) {
    avg += tempoList.get(a);
  }
  int tempTempo = (220 - (avg / tempoList.size()));
  if (tempTempo <= 70) {
    return (int)((double)avg / tempoList.size());
  }
  else return tempTempo;
}

/*
   Button

*/

void interrupted() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // if the interrupt comes faster than 200 ms, assume it is a bounce and ignore
  if (interrupt_time - last_interrupt_time > 100) {
    counting();
  }
  last_interrupt_time = interrupt_time;
}
int lastButtonState = 1;
void counting() {
  int reading = digitalRead(interruptPin);
  if (reading != lastButtonState) {
    counter++;
  }
}
