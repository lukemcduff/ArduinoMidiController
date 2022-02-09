#ifndef Controller_h
#define Controller_h
#include <MUX74HC4067.h>
#include <Arduino.h>

//***********************************************************************
class Mux2
{
  public:
    Mux2(byte outpin_, int pin0, int pin1, int pin2, int pin3);
    byte outpin;
    int pin0; 
    int pin1; 
    int pin2; 
    int pin3;
};
//************************************************************************
//Button (Pin Number, Command, Note Number, Channel, Debounce Time)
class Button
{
  public:
    Button(Mux2 mux, byte muxpin, byte channel, byte debounce, int midiNumber, bool isCC, bool defaultState);
    byte getValue();
    void muxRead();
    void updatePressedState(bool state);
    bool getPressedState();
    byte getMuxPin();
    void updateState(bool state);
    byte Bchannel;
    byte Btoggle;
    int midiCCNumber;
    bool isCC;
    bool LEDState;

  private:
    byte _previous;
    byte _current;
    unsigned long _time;
    int _debounce;
    byte _pin;
    byte _muxpin;
    byte _numMuxPins;
    byte _value;
    byte _command;
    bool _busy;
    byte _status;
    byte _last;
    byte _enablepin;

    bool _pressedState;
};
#endif
