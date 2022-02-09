#include "Controller.h"

//****************************************************************************************
Mux2::Mux2(byte outpin_, int pin0, int pin1, int pin2, int pin3)
{
  outpin = outpin_;
  pinMode(outpin, INPUT_PULLUP);
  pinMode(pin0, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
}

Button::Button(Mux2 mux, byte muxpin, byte channel, byte debounce, int midiNumber, bool isMidiCC, bool defaultState)
{
  _pin = mux.outpin;
  _muxpin = muxpin;
  _debounce = debounce;
  _time = 0;
  _busy = false;
  _status = 0b00000010;
  _last = 1;
  Bchannel = channel;
  Btoggle = 0;
  
  updatePressedState(defaultState);
  midiCCNumber = midiNumber;
  isCC = isMidiCC;

}

byte Button::getMuxPin(){
  return _muxpin;
}

bool Button::getPressedState(){
  return _pressedState;
}

void Button::updatePressedState(bool state){
  if(state != _pressedState){
    _pressedState = state;
  }
}

void Button::updateState(bool state){
  if(state != _pressedState){
    _pressedState = state;
  }
  if(state != LEDState){
    LEDState = state;
  }
}

void Button::muxRead()
{
  byte temp = _muxpin;
  temp = temp << 2;
  PORTD = PORTD & B11000011;
  PORTD = PORTD | temp;
}

byte Button::getValue()
{
  // If BUSY bit not set - read button
  if (bitRead(_status, 0) == false) { // If busy false
    if (digitalRead(_pin) == _last) return 2; // If same as last state - exit
  }

  // If NEW Bit set - Key just pressed, record time
  if (bitRead(_status, 1) == true) { // If new is true
    bitSet(_status, 0); // Set busy TRUE
    bitClear(_status, 1); // Set New FALSE
    _time = millis();
    return 255;
  }

  // Check if debounce time has passed - If no, exit
  if (millis() - _time < _debounce) return 255;

  // Debounce time has passed. Read pin to see if still set the same
  // If it has changed back - assume false alarm
  if (digitalRead(_pin) == _last) {
    bitClear(_status, 0); // Set busy false
    bitSet(_status, 1); // Set new true
    return 255;
  }

  // If this point is reached, event is valid. return event type
  else {
    bitClear(_status, 0); // Set busy false
    bitSet(_status, 1); // Set new true
    _last = ((~_last) & 0b00000001); // invert _last
    return _last;
  }
}
