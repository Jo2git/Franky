#ifndef M5BTN_H
#define M5BTN_H

#include <M5Stack.h>
#include <Arduino.h>

#define btnBGColor TFT_BLUE
#define btnFGColor TFT_YELLOW
#define screenBgColor TFT_BLACK

// scheinbar verdreht
#define TFT_W TFT_HEIGHT
#define TFT_H TFT_WIDTH

// Adresse des M5Faces-Encoders
#define Faces_Encoder_I2C_ADDR     0X5E

#define NumButtons 11 // ohne Drehregler li/re/Knopf; diese werden getrennt behandelt
#define NumHwButtons 3 // A, B, C

class M5Btn {

    public:

        enum ButtonType {A, B, C, AA, BB, CC, AB, AC, BA, BC, CA, CB, RotaryKnob, RotaryKnobLong, RotaryKnobDouble, RotaryKnobDoubleLong, RotaryLeft, RotaryRight, extBtnChnR, extBtnChnL, extBtnFktUp, extBtnFktDown };
        
        static void led(int ledIndex, int red, int green, int blue);
        static void ledRing(int red, int green, int blue, int delay_ms);

        static void begin(TFT_eSPI* tft);
        static bool loop();

    private:

        static TFT_eSPI* tft;

        static bool encoderLoop();
};

#endif