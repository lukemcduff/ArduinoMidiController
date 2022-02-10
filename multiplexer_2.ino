#include <MIDI.h>
#include "Controller.h"

MIDI_CREATE_DEFAULT_INSTANCE();

//************************************************************
//***SET THE CONTROLS USED**************************
//************************************************************
byte NUMBER_MUX_BUTTONS = 12;
byte midi_channel = 10;
int measureCount = 0;
int kickCount = 0;
int tempoCount = 0;
bool beatOn = false;
bool setInitialState = false;
bool rhythmMode = false;

bool handleClockMeasureState = false;
bool handleClockKickState = false;

int rPatternCount = 0;

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

  MUXBUTTONS[5]->LEDState = true;
  setUpStates();
  MIDI.setHandleClock(handleClock);
}


void loop() {
  readMuxButtons();  
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
  MUXBUTTONS[7]->LEDState = true;
  MUXBUTTONS[8]->LEDState = true;
  MUXBUTTONS[9]->LEDState = true;
  MUXBUTTONS[10]->LEDState = true;
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
      if(setInitialState == false){
        setUpStates();
      }
      switch(i){
        case 0:{
          handleClockKickState = false;
          MUXBUTTONS[4]->updateState(false);
          MIDI.sendControlChange(80, 96, midi_channel); 
          MIDI.sendControlChange(80, 32, midi_channel);
          if(!MUXBUTTONS[0]->getPressedState()){ 
            MUXBUTTONS[0]->LEDState = false;
            MUXBUTTONS[1]->updateState(false);
          }

          else if(MUXBUTTONS[0]->getPressedState()){
            MUXBUTTONS[0]->LEDState = true;
            MUXBUTTONS[1]->LEDState = false;
            MUXBUTTONS[2]->LEDState = false;
            
          }
          break;
        }
        case 1:{
          handleClockKickState = false;
          MUXBUTTONS[4]->updateState(false);
          sendCCOnOff(81);
          if(!MUXBUTTONS[0]->getPressedState()){ 
            MUXBUTTONS[1]->LEDState = MUXBUTTONS[1]->getPressedState();
          }
          MUXBUTTONS[2]->LEDState = false;
          MUXBUTTONS[4]->LEDState = false;
          break;
        }
        case 2:{
          handleClockKickState = false;
          MUXBUTTONS[4]->updateState(false);
          if(MUXBUTTONS[0]->getPressedState()){
            sendCCOnOff(80);
            MUXBUTTONS[0]->updateState(false);
            MUXBUTTONS[1]->LEDState = false;
          }
          sendCCOnOff(81);
          MUXBUTTONS[1]->updatePressedState(true);
          MUXBUTTONS[2]->LEDState = true;
          MUXBUTTONS[4]->LEDState = false;
          break;
        }
        case 3:{
          if(MUXBUTTONS[0]->getPressedState() && measureCount == 0){
            MUXBUTTONS[3]->LEDState = true;
            sendCCOnOff(80);
            MUXBUTTONS[0]->LEDState = false;
            handleClockMeasureState = true;
          }
          else if(MUXBUTTONS[1]->getPressedState() && measureCount == 0){
            MUXBUTTONS[3]->LEDState = true;
            sendCCOnOff(81);
            MUXBUTTONS[1]->LEDState = false;
            handleClockMeasureState = true;
          }
          break;
        }
        case 4:{
          // lets make our best effort to turn the track and drums off if we can
          if(MUXBUTTONS[0]->getPressedState()){
              sendCCOnOff(80);
              MUXBUTTONS[0]->updateState(false);
           }
           else if(!MUXBUTTONS[0]->getPressedState() && MUXBUTTONS[1]->getPressedState()){
              sendCCOnOff(81);
              MUXBUTTONS[1]->updateState(false);
              MUXBUTTONS[2]->LEDState = false;
           }
           MUXBUTTONS[2]->updatePressedState(false);
           handleClockKickState = MUXBUTTONS[4]->getPressedState();
           MUXBUTTONS[4]->LEDState = MUXBUTTONS[4]->getPressedState();
           break;
        }
        case 5:{
           sendCCOnOff(82);
          break;
          
        }
        case 6:{
          if(MUXBUTTONS[11]->getPressedState()){
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
        case 7: case 8: case 9: case 10:{          
          if(MUXBUTTONS[11]->getPressedState() and rhythmMode){
            switch(i){
              case 7:{ //84 - 87
                MIDI.sendControlChange(84, 74, midi_channel);
                sendCC(new int[3]{85,86,87}, 3, 0);
                MUXBUTTONS[8]->updateState(false); 
                MUXBUTTONS[9]->updateState(false);
                MUXBUTTONS[10]->updateState(false);

                MUXBUTTONS[7]->LEDState = true;
                break;
              }
              case 8:{
                MIDI.sendControlChange(85, 74, midi_channel);
                sendCC(new int[3]{84,86,87}, 3, 0);
                MUXBUTTONS[7]->updateState(false);
                MUXBUTTONS[9]->updateState(false);
                MUXBUTTONS[10]->updateState(false);

                MUXBUTTONS[8]->LEDState = true;
                break;
              }
              case 9:{
                MIDI.sendControlChange(86, 74, midi_channel);
                sendCC(new int[3]{84,85,87}, 3, 0);
                MUXBUTTONS[7]->updateState(false);
                MUXBUTTONS[8]->updateState(false);
                MUXBUTTONS[10]->updateState(false);
                
                MUXBUTTONS[9]->LEDState = true;                             
                break;
              }
              case 10:{
                MIDI.sendControlChange(87, 74, midi_channel);
                sendCC(new int[3]{84,85,86}, 3, 0);
                MUXBUTTONS[7]->updateState(false);
                MUXBUTTONS[8]->updateState(false);
                MUXBUTTONS[9]->updateState(false);
                
                MUXBUTTONS[10]->LEDState = true;                
                break;
              }
            }
            rhythmMode = false;
          }
          else{
            if(MUXBUTTONS[i]->getPressedState()){
              MIDI.sendControlChange(80 + i - 3, 74, midi_channel);
              MUXBUTTONS[i]->LEDState = true;
            }
            else{
              MIDI.sendControlChange(80 + i - 3, 0, midi_channel);
              MUXBUTTONS[i]->LEDState = false;
            }
          }
          break;
        } 
        case 11:{
          if(MUXBUTTONS[i]->getPressedState()){
            MUXBUTTONS[i]->LEDState = true;
            rhythmMode = true;
          }
          else{
              sendCC(new int[4]{84, 85, 86, 87}, 4, 74);
              MUXBUTTONS[i]->LEDState = false;

              MUXBUTTONS[7]->updateState(true);
              MUXBUTTONS[8]->updateState(true);
              MUXBUTTONS[9]->updateState(true);
              MUXBUTTONS[10]->updateState(true);
              rhythmMode = false;
          }
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

void handleClock(void){
  tempoCount += 1;
  if(tempoCount - 24 == 0){
    MUXBUTTONS[5]->LEDState = beatOn;
    beatOn = !beatOn;
    tempoCount = 0;
  }

  if(handleClockKickState){
      kickCount += 1;
      if(MUXBUTTONS[11]->getPressedState() && kickCount - 12 == 0){
        MIDI.sendNoteOn(54, 65, midi_channel); // tamborine
      }
      if(kickCount - 24 == 0){
        MIDI.sendNoteOn(36, 100, midi_channel); // kick drum
        MIDI.sendNoteOn(42, 90, midi_channel); // high hat
        if(MUXBUTTONS[11]->getPressedState()){
          MIDI.sendNoteOn(54, 90, midi_channel); // tamborine          
        }
        kickCount = 0;
      }
  }
  if(handleClockMeasureState){
    measureCount += 1;
    // one measure or 4 quarter notes has elapsed
    if(measureCount % 24 == 0){
      MIDI.sendNoteOn(42, 100, midi_channel); // high hat
    }
    int measureLength = 96;
    if(MUXBUTTONS[11]->getPressedState()){
      measureLength = 192;
    }
    
    if(measureCount - measureLength == 0){
        if(MUXBUTTONS[0]->getPressedState()){
          sendCCOnOff(80);
          MUXBUTTONS[0]->LEDState = true;
          MUXBUTTONS[1]->LEDState = false;
          MUXBUTTONS[0]->updatePressedState(true);
        }
        else{
          sendCCOnOff(81);
          MUXBUTTONS[1]->LEDState = true;
          MUXBUTTONS[1]->updatePressedState(true);
        }
        //MUXBUTTONS[0]->LEDState = true;
        //MUXBUTTONS[1]->LEDState = false;  
        MUXBUTTONS[2]->LEDState = false;
        MUXBUTTONS[3]->LEDState = false;
        //MUXBUTTONS[0]->updatePressedState(true);
        setLedState();
        measureCount = 0;
        handleClockMeasureState = false;
    }
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
