#include <MIDI.h>
#include "Controller.h"

//
// Note to future self: 
// When you pull the code back down from git, 
// create a new project and just paste the code in
//

MIDI_CREATE_DEFAULT_INSTANCE();
//************************************************************
//***SET THE CONTROLS USED**************************
//************************************************************
byte NUMBER_MUX_BUTTONS = 12;
byte midi_channel = 10;
bool lightShowRun = false;
bool lightOn = true;
int lightCounter = 0;
int lightIteration = 0;
bool setInitialState = false;
//bool rhythmMode = false;

bool handleClockMeasureState = false;
bool handleClockKickState = false;

int rKitCount = 0;
int kits[] {0, 5, 13, 22, 30, 39, 47, 56, 64, 72, 81, 89, 98, 106, 115, 123};

int rPatternCount = 0;
int patterns[] {0, 2, 4, 6, 8, 11, 13, 15, 17, 19, 22, 24, 26, 28, 31, 33, 35, 37, 39, 42, 44, 46, 48, 51, 53, 55, 57, 60, 62, 64, 66, 68, 71, 73, 75, 77, 80, 82, 84, 86, 89, 91, 93, 95, 97, 100, 102, 104, 106, 109, 111, 113, 115, 117, 120, 122, 124, 126};

//************************************************************

//***ANY MULTIPLEXERS? (74HC4067)************************************
//*******************************************************************
//Mux NAME (Input PIN, S0, S1, S2, S3); 
Mux2 Mux_Button(12, 2,3,4,5); // S0-S3 must use 2-5
//*******************************************************************
// Creates a MUX74HC4067 instance
// 1st argument is the Arduino PIN to which the EN pin connects
// 2nd-5th arguments are the Arduino PINs to which the S0-S3 pins connect
MUX74HC4067 Mux_Led(11,6,7,8,9);

//***DEFINE BUTTONS CONNECTED TO MULTIPLEXER*************************
//Button::Button(Mux2 mux, byte muxpin,byte channel, byte debounce, int midiNumber, bool isMidiCC, bool defaultState)

Button MBU1 (Mux_Button, 0, 10, 5, 80, true, false); // track P/S
Button MBU2 (Mux_Button, 1, 10, 5, 81, true, false); // Rhythm P/S
Button MBU3 (Mux_Button, 2, 10, 5, 82, true, true);  // Track stop and rhythm start in time
Button MBU4 (Mux_Button, 3, 10, 5, 83, true, true);  // stop everything for one measure then turn back on
Button MBU5 (Mux_Button, 4, 10, 5, 84, true, false);  // stop everything and play kicks on quarter notes
Button MBU6 (Mux_Button, 5, 10, 5, 85, true, true);  // tap tempo
Button MBU7 (Mux_Button, 6, 10, 5, 86, true, false);
Button MBU8 (Mux_Button, 7, 10, 5, 87, true, false);
Button MBU9 (Mux_Button, 8, 10, 5, 80, true, false);
Button MBU10(Mux_Button, 9, 10, 5, 80, true, false);
Button MBU11(Mux_Button, 10, 10, 5, 80, true, false);
Button MBU12(Mux_Button, 11, 10, 5, 80, true, false);
//*******************************************************************
////Add multiplexed buttons
Button *MUXBUTTONS[] {&MBU1,&MBU2,&MBU3,&MBU4,&MBU5,&MBU6,&MBU7,&MBU8,&MBU9,&MBU10,&MBU11,&MBU12};

//*******************************************************************


void setup() {
  //Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Mux_Led.signalPin(10, OUTPUT, DIGITAL);
  //setUpStates();
}


void loop() {
  lightShowAndSetupInitialLightStates();
  readMuxButtons();  
}

void lightShowAndSetupInitialLightStates(){
  if(lightCounter > 16){
    return;
  }
  if(lightCounter == 16)
  {
    setUpStates();
    lightCounter = lightCounter + 1;
  }
  MUXBUTTONS[lightCounter]->LEDState = lightOn;
  delay(250);
  if(lightCounter == 15){
    MUXBUTTONS[lightCounter]->LEDState = false;
  }
  lightCounter = lightCounter + 1;
}

void setUpStates(){
  MIDI.sendControlChange(83, 74, midi_channel);
  MIDI.sendControlChange(84, 0, midi_channel);
  MIDI.sendControlChange(85, 0, midi_channel);
  MIDI.sendControlChange(86, 0, midi_channel);
  MIDI.sendControlChange(87, 0, midi_channel);
  MIDI.sendControlChange(84, 74, midi_channel);
  MIDI.sendControlChange(85, 74, midi_channel);
  MIDI.sendControlChange(86, 74, midi_channel);
  MIDI.sendControlChange(87, 74, midi_channel);
  for(int i = 0; i <= 15; i++){
    if(i == 5 || i == 11) {
      MUXBUTTONS[i]->LEDState = true;
      continue;
    }
    MUXBUTTONS[i]->LEDState = false;
  }
  setInitialState = true;
  
}

//*****************************************************************

void readMuxButtons() {
  MIDI.read();
  // Cycle through Mux Button array
  for (int i = 0; i < NUMBER_MUX_BUTTONS; i = i + 1) {

    MUXBUTTONS[i]->muxRead();
    byte message = MUXBUTTONS[i]->getValue();

    //  Button is pressed
    if (message == 0) {
      MUXBUTTONS[i]->updatePressedState(!MUXBUTTONS[i]->getPressedState());
      switch(i){ 
        // rhythm P/S
        case 0:{ // CC 81
          sendCCOnOff(81);
          MUXBUTTONS[0]->LEDState = MUXBUTTONS[0]->getPressedState();
          break;
        }
        // druma parts on/off
        case 1: case 2: case 3: case 4:{     // CC 84, 85, 86, 87      
          if(MUXBUTTONS[5]->getPressedState()){
            switch(i){
              case 1:{ //84 - 87
                MIDI.sendControlChange(84, 74, midi_channel);
                sendCC(new int[3]{85,86,87}, 3, 0);
                MUXBUTTONS[2]->updateState(false); 
                MUXBUTTONS[3]->updateState(false);
                MUXBUTTONS[4]->updateState(false);

                MUXBUTTONS[1]->LEDState = true;
                break;
              }
              case 2:{
                MIDI.sendControlChange(85, 74, midi_channel);
                sendCC(new int[3]{84,86,87}, 3, 0);
                MUXBUTTONS[1]->updateState(false);
                MUXBUTTONS[3]->updateState(false);
                MUXBUTTONS[4]->updateState(false);

                MUXBUTTONS[2]->LEDState = true;
                break;
              }
              case 3:{
                MIDI.sendControlChange(86, 74, midi_channel);
                sendCC(new int[3]{84,85,87}, 3, 0);
                MUXBUTTONS[1]->updateState(false);
                MUXBUTTONS[2]->updateState(false);
                MUXBUTTONS[4]->updateState(false);
                
                MUXBUTTONS[3]->LEDState = true;                             
                break;
              }
              case 4:{
                MIDI.sendControlChange(87, 74, midi_channel);
                sendCC(new int[3]{84,85,86}, 3, 0);
                MUXBUTTONS[1]->updateState(false);
                MUXBUTTONS[2]->updateState(false);
                MUXBUTTONS[3]->updateState(false);
                
                MUXBUTTONS[4]->LEDState = true;                
                break;
              }
            }
          }
          else{
            if(MUXBUTTONS[i]->getPressedState()){
              MIDI.sendControlChange(80 + i + 3, 74, midi_channel);
              MUXBUTTONS[i]->LEDState = true;
            }
            else{
              MIDI.sendControlChange(80 + i + 3, 0, midi_channel);
              MUXBUTTONS[i]->LEDState = false;
            }
          }
          break;
        }
        // function button
        case 5:{ // NO UNIQUE CC
          if(MUXBUTTONS[i]->getPressedState()){
            MUXBUTTONS[i]->LEDState = true;
          }
          else{
              sendCC(new int[4]{84, 85, 86, 87}, 4, 74);
              MUXBUTTONS[i]->LEDState = false;

              MUXBUTTONS[1]->updateState(true);
              MUXBUTTONS[2]->updateState(true);
              MUXBUTTONS[3]->updateState(true);
              MUXBUTTONS[4]->updateState(true);
          }
          break;
        }
        // variation of rythm
        case 6:{ // CC 83
          if(MUXBUTTONS[5]->getPressedState()){
              if(MUXBUTTONS[i]->getPressedState()){
                MIDI.sendControlChange(83, 74, midi_channel);
                MIDI.sendControlChange(83, 0, midi_channel);
                MUXBUTTONS[6]->updatePressedState(false);
              }
              else{
                MIDI.sendControlChange(83, 0, midi_channel);
                MIDI.sendControlChange(83, 74, midi_channel);
                MUXBUTTONS[6]->updatePressedState(true);
              }
              break;
          }
          if(MUXBUTTONS[i]->getPressedState()){
            MIDI.sendControlChange(83, 74, midi_channel);
            MUXBUTTONS[i]->LEDState = true;
          }
          else{
            MIDI.sendControlChange(83, 0, midi_channel);
            MUXBUTTONS[i]->LEDState = false;
          }
          break;
        }
        // kit increment
        case 7:case 8:{ // CC 80 Goes from 0 - 190
          //studio 0
          //live 5
          //light 13
          //heavy 22
          //rock 30
          // metal 39
          // jazz 47
          // brush 56
          // cajon 64
          // drum and bass 72
          // rb 81
          // dance 89
          // techno 98
          // dance beats 106
          // hip hop 115
          // 808 123
          // kits are as follow (16 total) {0, 5, 13, 22, 30, 39, 47, 56, 64, 72, 81, 89, 98, 106, 115, 123}
          
            // patterns are as follow (58 total) {0, 2, 4, 6, 8, 11, 13, 15, 17, 19, 22, 24, 26, 28, 31, 33, 35, 37, 39, 42, 44, 46, 48, 51, 53, 55, 57, 60, 62, 64, 66, 68, 71, 73, 75, 77, 80, 82, 84, 86, 89, 91, 93, 95, 97, 100, 102, 104, 106, 109, 111, 113, 115, 117, 120, 122, 124, 126}
          // increase/decrease
          switch(i){
            case 7:{
              rKitCount = rKitCount - 1;
              if(rKitCount < 0){
                rKitCount = 0;
              }
              MIDI.sendControlChange(80, kits[rKitCount], midi_channel);
              MUXBUTTONS[i]->LEDState = true;
              MUXBUTTONS[i+1]->LEDState = false;
              break;
            }
            case 8:{
              rKitCount = rKitCount + 1;
              if(rKitCount > 15){
                rKitCount = 15;
              }
              MIDI.sendControlChange(80, kits[rKitCount], midi_channel);
              MUXBUTTONS[i]->LEDState = true;
              MUXBUTTONS[i-1]->LEDState = false;
              break;
            }
          }        
           
           break;
        }
        // pattern increase/decrease
        case 9:case 10:{ // CC 82
         switch(i){
            case 9:{
             rPatternCount = rPatternCount - 1;
              if(rPatternCount < 0){
                rPatternCount = 0;
              }
              MIDI.sendControlChange(82, patterns[rPatternCount], midi_channel);
              MUXBUTTONS[i]->LEDState = true;
              MUXBUTTONS[i+1]->LEDState = false;
              break;
            }
            case 10:{
              rPatternCount = rPatternCount + 1;
              if(rPatternCount > 57){
                rPatternCount = 57;
              }
              MIDI.sendControlChange(82, patterns[rPatternCount], midi_channel);
              MUXBUTTONS[i]->LEDState = true;
              MUXBUTTONS[i-1]->LEDState = false;
              break;
            }
          }        
           
           break;
        } 
        // tap tempo
        case 11:{
          sendCCOnOff(82);
          break;
        }

        
        
      }
      
    }
    //  Button is released
    if (message == 1) {
      
    }
    setLedState();
  }
}


void sendCCOnOff(int ccNumber){
      MIDI.sendControlChange(ccNumber, 96, midi_channel); 
      MIDI.sendControlChange(ccNumber, 32, midi_channel);
}

void sendCC(int arr[], int size, int ccValue){
  for(int i = 0; i< size; i++){
      MIDI.sendControlChange(arr[i], ccValue, midi_channel);
  }
}

void setLedState(){
  for (int i = 0; i < NUMBER_MUX_BUTTONS; i = i + 1) {   
      if(MUXBUTTONS[i]->LEDState){
        Mux_Led.write(MUXBUTTONS[i]->getMuxPin(), HIGH);
      }
      else{
        Mux_Led.write(MUXBUTTONS[i]->getMuxPin(), LOW);
      }
  }
}
