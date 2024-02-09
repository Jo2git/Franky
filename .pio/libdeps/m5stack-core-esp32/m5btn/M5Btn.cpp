#include "M5Btn.h"

// Parameter scheinen M5Stack-gerätespezifisch zu sein (wie schnell Tastendrücken erkannt wird)
#define SHORT 300   // Zeit in ms, bis zu der ein Kurzdruck erkannt wird
#define LONG  500   // Zeit in ms, ab der ein Langdruck erkannt wird
#define DOUBLE 400  // Zeit für Doppeldruck

TFT_eSPI* M5Btn::tft;

extern void buttonPressed(M5Btn::ButtonType btn);

bool wasPressed[NumHwButtons];
bool wasReleased[NumHwButtons];
bool isPressed[NumHwButtons];
bool longNotified[NumHwButtons];
long lastReleased[NumHwButtons];
long lastPressed[NumHwButtons];
bool doublePress = false;

// ----------------------------------------------------------------------------------------------------
// 

void M5Btn::begin(TFT_eSPI* tft) {
    M5Btn::tft = tft;
}

// ----------------------------------------------------------------------------------------------------
//
// Alternative: via FastLED:
// http://community.m5stack.com/topic/665/unexpected-behavior-on-m5stack-fire-leds/2

void M5Btn::led(int ledIndex, int red, int green, int blue) {
	Wire.beginTransmission(Faces_Encoder_I2C_ADDR);
	Wire.write(ledIndex);
	Wire.write(red);
	Wire.write(green);
	Wire.write(blue);
	Wire.endTransmission();
}

// ----------------------------------------------------------------------------------------------------
// 

void M5Btn::ledRing(int red, int green, int blue, int delay_ms) {
	for(int ledIndex=0;ledIndex<12;ledIndex++) {
        led(ledIndex, red, green, blue);
		delay(delay_ms);
    }
}

// ----------------------------------------------------------------------------------------------------
// Wichtig: Enumreihenfolge wird angenommen!

bool doubleButton(int first, int second) {
    switch ((M5Btn::ButtonType)first) {
        case M5Btn::ButtonType::A: buttonPressed(static_cast<M5Btn::ButtonType>(5+second)); break;
        case M5Btn::ButtonType::B: buttonPressed(static_cast<M5Btn::ButtonType>(second == M5Btn::ButtonType::A ? M5Btn::ButtonType::BA : M5Btn::ButtonType::BC)); break;
        case M5Btn::ButtonType::C: buttonPressed(static_cast<M5Btn::ButtonType>(10+second)); break;
    }
}

// ----------------------------------------------------------------------------------------------------
// Prüft, ob andere als Button <pressedButtonIndex> (0..NumHwButtons-1) gedrückt wurden und liefert dann
// false, sonst true

bool checkOthers(int pressedButtonIndex) {
    int prev = (pressedButtonIndex -1 ) % NumHwButtons; if (prev < 0) prev = NumHwButtons - 1;
    int next = (pressedButtonIndex + 1) % NumHwButtons;

    if (!isPressed[prev] && !isPressed[next]) return true;

    if (isPressed[prev] && isPressed[next]) return false; // ignoriere gleichzeitiges ABC

    if (isPressed[prev]) { 
        doubleButton(prev, pressedButtonIndex);
        longNotified[prev] = true; 
        doublePress = true;
        return false; 
    }
    if (isPressed[next]) { 
        doubleButton(next, pressedButtonIndex);
        longNotified[next] = true;
        doublePress = true;
        return false; 
    }

}

// ----------------------------------------------------------------------------------------------------
//

bool M5Btn::loop() {

    M5.update(); // Buttonzustand einlesen

    // Zustände speichern (weil z.B. M5.BtnA nur einmal wirkt und das Event dann verbraucht ist)
    wasPressed [0] = M5.BtnA.wasPressed();  wasPressed [1] = M5.BtnB.wasPressed();  wasPressed [2] = M5.BtnC.wasPressed();
    wasReleased[0] = M5.BtnA.wasReleased(); wasReleased[1] = M5.BtnB.wasReleased(); wasReleased[2] = M5.BtnC.wasReleased();
    isPressed  [0] = M5.BtnA.isPressed();   isPressed  [1] = M5.BtnB.isPressed();   isPressed  [2] = M5.BtnC.isPressed();

    for (int i=0; i<NumHwButtons; i++) {
    
        if (wasReleased[i]) {
            lastReleased[i] = millis();
        }

        if (wasPressed[i]) {
            lastPressed[i] = millis();
            longNotified[i] = false;
        }

        /* SHORT?
        <-----> kleiner SHORT
        |~~~~~|
     ---|     |-----
              |
              v Event: short pressed
        */
        if (wasReleased[i]) {
            if (millis() - lastPressed[i] <= SHORT)
                if (checkOthers(i)) {
                    if (!doublePress) buttonPressed((ButtonType)i); else doublePress = false;
                }
            wasReleased[i] = false;
        }

        /* LONG?
        <------------> größer SHORT 
        |                  | > gleich LONG
        |~~~~~~~~~~~~~~~~~~|
     ---|             |    |------- 
                        v Event: long pressed
        */
        if (isPressed[i] && millis() - lastPressed[i] >= LONG && !longNotified[i]) {
            buttonPressed((ButtonType)(3 + i));
            longNotified[i] = true;
        }
    }

    return (encoderLoop() || isPressed[0] || isPressed[1] || isPressed[2]);
}

// ----------------------------------------------------------------------------------------------------
//

// Drehreglerknopf
int buttonState = HIGH;
int oldButtonState = buttonState;
boolean _buttonDown = false;
long rotaryKnobLastPressed;
bool rotaryKnobLongNotified = false;
bool rotaryKnobPressedNotified = false; // wurde gedrückt merken um Doppeldruck zu erkennen  
bool rotaryKnobPressedNotified2 = false; // wurde 2. mal gedrückt  

// Drehreglerstellung
int _min = 0;
int _max = 0;
int _incr = 1;
int _value = 0;
int _oldValue = _value;
int _direction = 0;

// Drehgeschwindigkeit
long lastTurned = 0;
int _fast;
int _veryFast;

bool M5Btn::encoderLoop() {
    
  int tmp_increment, increment;
  boolean turnedFast, turnedVeryFast;

  Wire.requestFrom(Faces_Encoder_I2C_ADDR, 3);
  if (Wire.available()) {
    tmp_increment = Wire.read();
    buttonState = Wire.read();
  }/* else {
      return;
  }*/

  if (tmp_increment > 127) { // nach links gedreht
    _direction = 1;
    increment = -_incr;
    buttonPressed(M5Btn::RotaryLeft);
  } else {
    _direction = 0;
    increment = tmp_increment * _incr; // 0: nicht gedreht, 1: rechts gedreht
    if (increment) buttonPressed(M5Btn::RotaryRight);
  }

  turnedFast = turnedVeryFast = false;
  if (increment != 0) { // wurde gedreht
    if (millis() - lastTurned < _veryFast) {
      turnedVeryFast = true;
    } else if (millis() - lastTurned < _fast) {
      turnedFast = true;
    }
    lastTurned = millis();
  }

  if (turnedVeryFast) increment = increment * 10;
  if (turnedFast) increment = increment * 5;

  _value += increment;

  if (turnedVeryFast) _value = _value - _value % 10;
  if (turnedFast) _value = _value - _value % 5;

  if (_value < _min) _value = _min;
  if (_value > _max) _value = _max;

  // Button gedrückt?
  bool wasReleased = buttonState == HIGH && oldButtonState == LOW;  // 10-Flanke
  bool wasPressed =  buttonState == LOW  && oldButtonState == HIGH; // 01-Flanke
  bool isPressed =   buttonState == LOW;                            // gedrückt
  oldButtonState =   buttonState;

  if (wasPressed) {
    rotaryKnobLastPressed = millis();
    rotaryKnobLongNotified = false;
    rotaryKnobPressedNotified2 = rotaryKnobPressedNotified;
  }

  if (wasReleased) rotaryKnobPressedNotified2 = false; // beim Loslassen immer rücksetzen


  // Doppeldruckerkennung rückstellen - Kurzdruck ausgeben, falls kein Doppeldruck erkannt wurde
  if (rotaryKnobPressedNotified && !rotaryKnobPressedNotified2)
  {
    if ((millis() - rotaryKnobLastPressed) > DOUBLE) {
       buttonPressed(ButtonType::RotaryKnob);
       rotaryKnobPressedNotified = false;
    }
  }
  
  // Drehknopf kurz oder doppelt gedrückt?
  if (wasReleased && millis() - rotaryKnobLastPressed <= SHORT && millis() - rotaryKnobLastPressed > 20) // Entprellung
  {
    if (!rotaryKnobPressedNotified)
    {
      rotaryKnobPressedNotified = true;
    }
    else
    {
      buttonPressed(ButtonType::RotaryKnobDouble);
      rotaryKnobPressedNotified = false;  
    }
  }
  
  // Drehknopf lang gedrückt?
  if (isPressed && millis() - rotaryKnobLastPressed >= LONG && !rotaryKnobLongNotified)
  {
    if (!rotaryKnobPressedNotified)
    { 
      buttonPressed(ButtonType::RotaryKnobLong);
      rotaryKnobLongNotified = true;
    }
    else
    {
      buttonPressed(ButtonType::RotaryKnobDoubleLong);
      rotaryKnobPressedNotified = false;
      rotaryKnobLongNotified = true;
    }
  }
  return isPressed || increment || _direction; 
}
