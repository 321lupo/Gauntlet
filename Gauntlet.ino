
// TODO MAKE SCALE SWITCH UP/DOWN BUTTON
//  OR SHIFT? OR THIRD BUTTON FOR CONTROL/NOTE CHANGE ON LONG SENSORS??


#include <Bounce.h>
#include <MIDI.h>
#define MIDI_CHAN 2

#define BLUE1 5                      //LEDS
#define GREEN1 4
#define RED1 3
#define BLUE2 10
#define GREEN2 9
#define RED2 6

#define FSRLONG_N 6                   //FSR AND POS SENSOR VARIABLES
#define POSLONG_N 6
#define FSRSHORT_N 8 
int fsrlongPins[] = {A0, A1, A2, A3, A4, A5}; //the pins for the long fsr sensors
int poslongPins[] = {A6, A7, A8, A9, A10, A11}; //the pins for the pos sensors
int fsrshortPins[] = {A12, A13, A16, A19, A14, A15, A17, A18}; //the pins for the short fsr sensors
int fsrlongReadings[FSRLONG_N];
int poslongReadings[POSLONG_N];
int fsrshortReadings[FSRSHORT_N];
int poslongMidi[POSLONG_N];
int fsrshortmidiVol[FSRSHORT_N];
bool fsrshortPress[FSRSHORT_N];


int controlVal = 49; //smallest control value - 1 (make sure it doesnt conflict with the other midi inst)

int scale[] = {60, 62, 64, 65, 67, 69, 71, 72};  //C MAJOR SCALE

#define BUTTON1 1                     //BUTTONS AND BOUNCE
#define BUTTON2 2
Bounce ButtonUp = Bounce(BUTTON1, 10);
Bounce ButtonDown = Bounce(BUTTON2, 10);
#define NUM_BANKS 3
int bank = 0;


void setup() {

  Serial.begin(9600);       // use the serial port
  Serial.print(" START ");   

  initFsrlongs(); 
  initPoslongs();
  initFsrshorts();
  initButtons();
  initLeds();

}

void loop() {

  while (usbMIDI.read()) ; // read and discard any incoming MIDI messages
   
  readFsrlongs();
  readPoslongs();
  readFsrshorts();
  readButtons();

  Serial.println("");    

  if(ButtonUp.fallingEdge()) {     //3 banks and the buttons switch through them
    bank++;
    bank %= NUM_BANKS;
  }
  if(ButtonDown.fallingEdge()) {
    bank-1;
    bank %= NUM_BANKS;
  }
  
  if (bank = 0) {
    
    for(int i = 0; i< FSRLONG_N; i++){             //SHOULD SEND CORRECT CONTROL CHANGES TRY OUT AFTER HARDWARE REP
      if (fsrlongReadings[i] <= 850){
      poslongMidi[i] = map (poslongReadings[i], 0, 1023, 0, 127);
      usbMIDI.sendControlChange((i+controlVal), poslongMidi[i], MIDI_CHAN);  
    
      }
    }  
  
    for(int i = 0; i< FSRSHORT_N; i++){             //SHOULD SEND CORRECT NOTE VALUES according to scale TRY OUT AFTER HARDWARE REP
      if (fsrshortReadings[i] <= 920){
        if(!fsrshortPress[i]){
        fsrshortmidiVol[i] = map (fsrshortReadings[i], 1024, 0, 0, 127);
        usbMIDI.sendNoteOn(scale[i], fsrshortmidiVol[i], MIDI_CHAN);
        analogWrite(BLUE1, 255);
        analogWrite(BLUE2, 255);
        } 
      fsrshortPress[i] = true;
      }
      else if (fsrshortReadings[i] > 920) {
        fsrshortPress[i] = false;
        usbMIDI.sendNoteOff(scale[i], 127, MIDI_CHAN);
      }
    }  
  }
  
  else if (bank = 1) {
    Serial.println("Now What..");   
  }
  
  else if (bank = 2) {
    Serial.println("Sooooo?");
  }
     
     
     
  Serial.print("Bank is ");
  Serial.println(bank);
  delay (30);
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
  for(int i = 0; i< FSRSHORT_N; i++){
    fsrshortReadings[i] = analogRead(fsrshortPins[i]);
  }
  for(int i = 0; i< FSRSHORT_N; i++){
    Serial.print (fsrshortReadings[i]);
    Serial.print (" ");
  }
}


void readButtons(){
  ButtonUp.update();
  ButtonDown.update();
}
