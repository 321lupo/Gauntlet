
//  OR SHIFT? OR 2nd BUTTON FOR CONTROL/NOTE CHANGE ON LONG SENSORS??
//or control mode and playmode..


#include <Bounce.h>
#include <MIDI.h>
#define MIDI_CHAN 2

#define BLUE1 5                      //LEDS
#define GREEN1 4
#define RED1 3
#define BLUE2 10
#define GREEN2 9
#define RED2 6

#define FSRLONG_N 6                  //FSR AND POS SENSOR VARIABLES AND THRESHOLD LEVELS
#define POSLONG_N 6
#define FSRSHORT_N 8 
#define THRESH_SHORT 850
#define THRESH_LONG 930

int fsrlongPins[] = {A0, A1, A2, A3, A4, A5}; //the pins for the long fsr sensors
int poslongPins[] = {A6, A7, A8, A9, A10, A11}; //the pins for the pos sensors
int fsrshortPins[] = {A12, A13, A16, A19, A14, A15, A17, A18}; //the pins for the short fsr sensors
int fsrlongReadings[FSRLONG_N];
int poslongReadings[POSLONG_N];
int fsrshortReadings[FSRSHORT_N];
int poslongMidi[POSLONG_N];
int poslongPitch [POSLONG_N];
int posnoteLevel[POSLONG_N];
int posnotePos [POSLONG_N];
int posChange [POSLONG_N];
int fsrshortmidiVol[FSRSHORT_N];
bool fsrshortPress[FSRSHORT_N];
bool fsrlongPress [FSRLONG_N];


int controlVal = 31;                              //smallest control value - 1 (make sure it doesnt conflict with the other midi inst)

int scalelong[] = {60,62,65,67,69,72};
int scaleshort[] = {60, 62, 64, 65, 67, 69, 71, 72};   //C MAJOR SCALE

#define BUTTON1 1                                 //BUTTONS AND BOUNCE
#define BUTTON2 0
Bounce ButtonUp = Bounce(BUTTON1, 10);
Bounce ButtonDown = Bounce(BUTTON2, 10);
int bank = 0;
#define NUM_BANKS 3

void setup() {

  Serial.begin(9600);       // use the serial port
  Serial.print(" START ");   

  initFsrlongs(); 
  initPoslongs();
  initFsrshorts();
  initButtons();
  initLeds(); //might not need the init for analogWrites
  bankLeds(); 

  for(int i=0; i<FSRSHORT_N; i++){ 
    fsrshortPress[i]=false;
  }
  for(int i=0; i<FSRLONG_N; i++){ 
    fsrlongPress[i]=false; 
  }
  for (int i=0; i<FSRSHORT_N; i++){
    usbMIDI.sendNoteOff(scaleshort[i], 127, MIDI_CHAN);  
  }
    for (int i=0; i<FSRLONG_N; i++){
    usbMIDI.sendNoteOff(scalelong[i], 127, MIDI_CHAN);  
  }
}

void loop() {

  while (usbMIDI.read()) ; // read and discard any incoming MIDI messages
   
  readFsrlongs();
  readPoslongs();
  readFsrshorts();
  readButtons();

  Serial.println("");    

  if(ButtonUp.fallingEdge()) {              //3 banks and the buttons switch through them BLUE==MODE0, TURKOISE==MODE1, GREEN=MODE2
    bank++;                                 //Leds show bank except when control or midi notes are being sent                                
    if (bank>=3) bank=NUM_BANKS-3;;
    bankLeds();   
    resetPress();                           //resets the press booleans on every bank change  
    resetScales();                          //sends midi off for both scales
    usbMIDI.sendPitchBend(8192, MIDI_CHAN)
  }
  else if(ButtonDown.fallingEdge()) {
    bank=bank-1;
    if (bank<=-1) bank=NUM_BANKS-1;
    bankLeds();
    resetPress();                           //resets the press booleans on every bank change
    resetScales();                         //sends midi off for both scales
    usbMIDI.sendPitchBend(8192, MIDI_CHAN);
  } 
  
  Serial.println(bank); 

  
  if (bank==0) {                          //MODE0 BLUE long sensors act as cntrl short as midi notes (pos on volume faders, notes for more exact instrument than is possible with glove 
                                          // alternatively, if guitar is synth sufficient use the midi buttons as solo/active buttons in ableton 

    Serial.println("MODE0");
    for(int i = 0; i< FSRLONG_N; i++){                                      //LONG SENSORS 
      if (fsrlongReadings[i] <= THRESH_LONG){
        poslongMidi[i] = map (poslongReadings[i], 0, 1023, 0, 127);
        usbMIDI.sendControlChange((i+controlVal), poslongMidi[i], MIDI_CHAN);
        fsrlongPress[i]=true;
        int ledcntrlblue = map (poslongReadings[i], 0, 1023, 0, 255);
        int ledcntrlred = map (poslongReadings[i], 0, 1023, 0, 255);
        analogWrite(BLUE1, ledcntrlblue);                                       //led animations
        analogWrite(BLUE2, ledcntrlblue);
        analogWrite(RED1, ledcntrlred);
        analogWrite(RED2, ledcntrlred);
      }
      else if (fsrlongReadings[i] > THRESH_LONG && fsrlongPress[i]==true) {           //this function is only there to shut the Leds off
        fsrlongPress[i] = false;
        clearLeds();
        bankLeds();
      }
    }  
  
    for(int i=0; i<FSRSHORT_N; i++){                                        //SHORT SENSORS
      if (fsrshortReadings[i]<=THRESH_SHORT){
        if(!fsrshortPress[i]) {
          fsrshortmidiVol[i] = map (fsrshortReadings[i], 1024, 0, 0, 127);
          usbMIDI.sendNoteOn(scaleshort[i], fsrshortmidiVol[i], MIDI_CHAN);
        } 
        fsrshortPress[i]=true;
        analogWrite(BLUE1, 255);
        analogWrite(BLUE2, 255);
      }
      else if (fsrshortReadings[i] > THRESH_SHORT && fsrshortPress[i]==true) {
        fsrshortPress[i] = false;
        usbMIDI.sendNoteOff(scaleshort[i], 127, MIDI_CHAN);
        clearLeds();
        bankLeds();
        Serial.println(String(" Note ")+(scaleshort[i])+(" LedOff "));
      }
    }
  }

    
  
  else if (bank==1) {                                                 //MODE0=1 BLUE/GREEN send notes plus bend on pos sensors. fsr send control values
    Serial.println("MODE1");   

    for(int i=0; i<FSRLONG_N; i++) {
      if (fsrlongReadings[i]<=THRESH_LONG) { 
        if (fsrlongPress[i]==false){   
          delay(20);
          poslongReadings[i] = analogRead(poslongPins[i]);
          posnoteLevel[i] = map (fsrlongReadings[i], THRESH_LONG, 0, 0, 127); 
          usbMIDI.sendNoteOn(scalelong[i], posnoteLevel[i], MIDI_CHAN);     
          }
        fsrlongPress[i] = true;
        poslongPitch[i] = map (poslongReadings[i], 0, 1023, 0, 16383); 
        usbMIDI.sendPitchBend(poslongPitch[i], MIDI_CHAN);
        int lednoteblue = map (poslongReadings[i], 0, 1023, 255, 255);
        int lednotegreen = map (poslongReadings[i], 0, 1023, 0, 255);
        analogWrite(BLUE1, lednoteblue);                                       //led animations
        analogWrite(BLUE2, lednoteblue);
        analogWrite(GREEN1, lednotegreen);
        analogWrite(GREEN2, lednotegreen);
      } 
      else if (fsrlongReadings[i]>THRESH_LONG && fsrlongPress[i] == true) {
        usbMIDI.sendNoteOff(scalelong[i], 127, MIDI_CHAN);   
        usbMIDI.sendPitchBend(8192, MIDI_CHAN); 
        fsrlongPress[i] = false;
        clearLeds();
        bankLeds();      
      }      
    }
  
    
    for (int i=0; i<FSRSHORT_N; i++){             //SHORT SENSORS send control values
      if (fsrshortReadings[i]<=THRESH_SHORT){
        if(!fsrshortPress[i]) {
          usbMIDI.sendControlChange((i+FSRLONG_N+controlVal), 127, MIDI_CHAN);
        } 
        fsrshortPress[i]=true;
        analogWrite(BLUE1, 255);
        analogWrite(BLUE2, 255);
        analogWrite(GREEN1, 255);
        analogWrite(GREEN2, 255);
      }
      else if (fsrshortReadings[i] > THRESH_SHORT && fsrshortPress[i]==true) {
        fsrshortPress[i] = false;
        clearLeds();
        bankLeds();
        Serial.println(String(" CTRL ")+(i+FSRLONG_N+controlVal)+(" LedOff "));
      }
    }
  }
  
  else if (bank==2) {             //GREEN maybe discard and only have one switch button
    Serial.println("Sooooo?");    
    for(int i=0; i<FSRLONG_N; i++) {          
          if (fsrlongReadings[i]<=THRESH_LONG) { 
            if (fsrlongPress[i]==false){   
              posnoteLevel[i] = map (fsrlongReadings[i], THRESH_LONG, 0, 0, 127); 
              posnotePos[i] = poslongReadings[i];
              usbMIDI.sendNoteOn(scalelong[i], posnoteLevel[i], MIDI_CHAN);  
             // delay (20);                                 //THIS IN MILLIS
            }
            fsrlongPress[i]=true;
            posChange[i] = (poslongReadings[i] - posnotePos[i]) * 16; 
            poslongPitch[i] = 8192 + posChange[i];
            poslongPitch[i] = constrain(poslongPitch[i],0,16383);
            usbMIDI.sendPitchBend(poslongPitch[i], MIDI_CHAN);
            int lednoteblue = map (poslongPitch[i], 0, 16383, 255, 255);        //led animations
            int lednotegreen = map (poslongPitch[i], 0, 16383, 0, 255);
            analogWrite(BLUE1, lednoteblue);                                        
            analogWrite(BLUE2, lednoteblue);
            analogWrite(GREEN1, lednotegreen);
            analogWrite(GREEN2, lednotegreen);
            
          }
          else if (fsrlongReadings[i]>THRESH_LONG && fsrlongPress[i] == true) {
            fsrlongPress[i] = false;
            usbMIDI.sendNoteOff(scalelong[i], 127, MIDI_CHAN);    
            clearLeds();
            bankLeds();      
            //delay(20)
          }    
    }
  }

     delay(20);
}
  


//ADDITIONAL FUNCTIONS

void initFsrlongs(){
  pinMode(A0, INPUT_PULLUP); 
  pinMode(A1, INPUT_PULLUP); 
  pinMode(A2, INPUT_PULLUP); 
  pinMode(A3, INPUT_PULLUP); 
  pinMode(A4, INPUT_PULLUP); 
  pinMode(A5, INPUT_PULLUP); 
}

void initPoslongs(){            //there are external pull_up resistors on the remaining A pins
  pinMode(A6, INPUT_PULLUP); 
  pinMode(A7, INPUT_PULLUP); 
  pinMode(A8, INPUT_PULLUP); 
  pinMode(A9, INPUT_PULLUP); 
}

void initFsrshorts(){
  pinMode(A15, INPUT_PULLUP); 
  pinMode(A16, INPUT_PULLUP); 
  pinMode(A17, INPUT_PULLUP); 
  pinMode(A18, INPUT_PULLUP); 
  pinMode(A19, INPUT_PULLUP); 
  pinMode(A20, INPUT_PULLUP); 
}

void initButtons() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
} 

void initLeds() {
  pinMode(BLUE1, OUTPUT);
  pinMode(GREEN1, OUTPUT);
  pinMode(RED1, OUTPUT);
  pinMode(BLUE2, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(RED2, OUTPUT);
}

void clearLeds() {
  analogWrite(BLUE1, 0); 
  analogWrite(BLUE2, 0);
  analogWrite(RED1, 0);
  analogWrite(RED2, 0);
  analogWrite(GREEN1, 0);
  analogWrite(GREEN2, 0);
}

void bankLeds() {
    if (bank==0) {
      clearLeds();
      analogWrite(BLUE1, 10);
      analogWrite(BLUE2, 10);
    }
    else if (bank==1) {
      clearLeds();
      analogWrite(BLUE1, 7);
      analogWrite(BLUE2, 7);
      analogWrite(GREEN1, 7);
      analogWrite(GREEN2, 7);
    }
    else if (bank==2) {
      clearLeds();
      analogWrite(GREEN1, 10);
      analogWrite(GREEN2, 10);
    }
}


void readFsrlongs(){
  for(int i = 0; i< FSRLONG_N; i++){
    fsrlongReadings[i] = analogRead(fsrlongPins[i]);
  }
  for(int i = 0; i< FSRLONG_N; i++){
    Serial.print (fsrlongReadings[i]);
    Serial.print (" ");
  }
}

void readPoslongs(){
  for(int i = 0; i< POSLONG_N; i++){
    poslongReadings[i] = analogRead(poslongPins[i]);
  }
  for(int i = 0; i< POSLONG_N; i++){
    Serial.print (poslongReadings[i]);
    Serial.print (" ");
  }
}

void readFsrshorts(){
  for(int i=0; i<FSRSHORT_N; i++){
    fsrshortReadings[i] = analogRead(fsrshortPins[i]);
  }
  for(int i=0;i<FSRSHORT_N; i++){
    Serial.print (fsrshortReadings[i]);
    Serial.print (" ");
  }
}


void readButtons(){
  ButtonUp.update();
  ButtonDown.update();
}

void resetPress(){
  for(int i=0; i<FSRSHORT_N; i++){ 
    fsrshortPress[i]=false;
  }
  for(int i=0; i<FSRLONG_N; i++){ 
    fsrlongPress[i]=false;
  }
}

void resetScales(){
    for (int i=0; i<FSRSHORT_N; i++){
    usbMIDI.sendNoteOff(scaleshort[i], 127, MIDI_CHAN);  
  }
    for (int i=0; i<FSRLONG_N; i++){
    usbMIDI.sendNoteOff(scalelong[i], 127, MIDI_CHAN);  
  }
}

