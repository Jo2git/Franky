#ifndef EXTBTN_H
#define EXTBTN_H

#include <Arduino.h>
#include "LocoPage.h"
#include "Pref.h"

#define IN1 5
#define IN2 2
#define IN3 17
#define IN4 16
#define IN5 19
#define IN6 26

#define NumExtHwButtons 6

struct ExtBtn
{
    static void begin();
    static bool loop();
    static void update();
    static bool extBtnL[NumExtHwButtons];
    static bool extBtnK[NumExtHwButtons];
    static bool extBtn01[NumExtHwButtons];
    static bool extBtn10[NumExtHwButtons];
    static bool extBtn;
};

//void setFst0 ();

#endif