#include <MIDI.h>
#include <Bounce2.h>
#include <LinkedList.h>

int midiChannel = 10;
int buttonStartCCNumber = 80;
int bluePin = 10;
int orangePin = 11;

MIDI_CREATE_DEFAULT_INSTANCE();

class CustomButton {
  public:
    bool pressedState = false;
    bool shouldTrackState = false;
    Bounce2::Button *button = new Bounce2::Button();
};

LinkedList<CustomButton*> custButts = LinkedList<CustomButton*>();

void setupButtons(int totalButtons) {
  for (int pin = 2; pin < totalButtons + 2; pin++) {
    CustomButton *customButt = new CustomButton();
    customButt->button->attach(pin, INPUT_PULLUP);
    // DEBOUNCE INTERVAL IN MILLISECONDS
    customButt->button->interval(5);
    // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
    customButt->button->setPressedState(LOW);

    if(pin >= 4){
      customButt->shouldTrackState = true;
    }
    
    custButts.add(customButt);
  }

}


void setup() {
  //Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL_OFF);
  setupButtons(8);
  pinMode(bluePin, OUTPUT);
  pinMode(orangePin, OUTPUT);
  digitalWrite(bluePin, HIGH);
}

// the loop routine runs over and over again forever:
void loop() {

  CustomButton *butt;
  for (int i = 0; i < custButts.size(); i++) {
    butt = custButts.get(i);
    butt->button->update();

//    if(butt->button->pressed() && i==5){
//      MIDI.sendNoteOn(76, 127, 10);
//      continue;
//    }
//
//    if(butt->button->released() && i==5){
//      MIDI.sendNoteOff(76, 0, 10);
//      continue;
//    }

    if (butt->button->pressed() && butt->shouldTrackState) {
      if(butt->pressedState){
        butt->pressedState = false;
        //Serial.println("off sent for tracking");
        MIDI.sendControlChange(buttonStartCCNumber + i, 0, midiChannel); // 1 - studio 246 - 808, // patterns are 0 thru 74
        digitalWrite(orangePin, HIGH);
      }
      else if(!butt->pressedState){
//        Serial.println("button pressed");     
//        Serial.println(i);
//        Serial.println("on sent with tracking");
        MIDI.sendControlChange(buttonStartCCNumber + i, 74, midiChannel); 
        butt->pressedState = true;
        digitalWrite(orangePin, HIGH);
      }
    }

    if (butt->button->pressed() && !butt->shouldTrackState){
//         Serial.println("button pressed");     
//         Serial.println(i);
//         Serial.println("on sent no tracking");
         MIDI.sendControlChange(buttonStartCCNumber + i, 96, midiChannel);
         digitalWrite(orangePin, HIGH); 
    }

    if(butt->button->released() && !butt->shouldTrackState){
//      Serial.println("button released");     
//      Serial.println(i);
//      Serial.println("off sent no tracking");
      MIDI.sendControlChange(buttonStartCCNumber + i, 32, midiChannel);
    }

  }
  delay(10); // delay in between reads for stability
  digitalWrite(orangePin, LOW);
}
