#include "ExtBtn.h"
#include "LocoPage.h"
#include "Pref.h"

// Parameter scheinen M5Stack-gerätespezifisch zu sein (wie schnell Tastendrücken erkannt wird)
#define SHORT 300 // Zeit in ms, bis zu der ein Kurzdruck erkannt wird
#define LONG 500  // Zeit in ms, ab der ein Langdruck erkannt wird

bool extBtnPressed;
bool WasPressed[NumExtHwButtons];
bool WasReleased[NumExtHwButtons];
bool IsPressed[NumExtHwButtons];
bool ExtbuttonState[NumExtHwButtons];
bool LongNotified[NumExtHwButtons];
bool Sl[NumExtHwButtons];
bool Sk[NumExtHwButtons];
bool S01[NumExtHwButtons];
bool S10[NumExtHwButtons];
long LastReleased[NumExtHwButtons];
long LastPressed[NumExtHwButtons];
int extFktS0;
int extFktS1;
int extFktS2;

// ----------------------------------------------------------------------------------------------------
//
extern void buttonPressed(M5Btn::ButtonType btn);

void setAllChnFst0()
{
    for (size_t i = 0; i < MAX_LOCO_CHANNELS; i++)
    {
        Loco::loco[i] = Loco::addLoco(Pref::get(prefNameLocoChannelAddr + String(i), String(i + 1)).toInt());
        Loco::loco[i]->fst = 0;
        Loco::loco[i]->targetFst = 0;
        Loco::loco[i]->speed = 0;
        Z21::LAN_X_SET_LOCO_DRIVE(
            Loco::loco[i]->getAddr(),
            Loco::loco[i]->forward ? Forward : Backward,
            Loco::loco[i]->fst);
    }
};

void ExtBtn::update()
{
    for (int i = 0; i < NumExtHwButtons; i++)
    {
        ExtBtn::extBtn01[i] = S01[i];
        ExtBtn::extBtn10[i] = S10[i];
        ExtBtn::extBtnL[i] = Sl[i];
        ExtBtn::extBtnK[i] = Sk[i];
        ExtBtn::extBtn = extBtnPressed;
    }
};

void ExtBtn::begin()
{
    pinMode(IN1, INPUT_PULLUP);
    pinMode(IN2, INPUT_PULLUP);
    pinMode(IN3, INPUT_PULLUP);
    pinMode(IN4, INPUT_PULLUP);
    pinMode(IN5, INPUT_PULLUP);
    pinMode(IN6, INPUT_PULLUP);
}

bool ExtBtn::loop()
{

    // externe Taster einlesen
    ExtbuttonState[0] = !digitalRead(IN1);
    ExtbuttonState[1] = !digitalRead(IN2);
    ExtbuttonState[2] = !digitalRead(IN3);
    ExtbuttonState[3] = !digitalRead(IN4);
    ExtbuttonState[4] = !digitalRead(IN5);
    ExtbuttonState[5] = !digitalRead(IN6);

    extBtnPressed = false;

    for (int i = 0; i < NumExtHwButtons; i++)
    {

        WasReleased[i] = ExtbuttonState[i] == LOW && IsPressed[i] == HIGH; // 10-Flanke
        WasPressed[i] = ExtbuttonState[i] == HIGH && IsPressed[i] == LOW;  // 01-Flanke
        IsPressed[i] = ExtbuttonState[i];

        if (WasReleased[i])
        {
            LastReleased[i] = millis();
            extBtnPressed = true;
        }

        if (WasPressed[i])
        {
            LastPressed[i] = millis();
            LongNotified[i] = false;
            extBtnPressed = true;
        }

        // 01/10-Flanken egal wie lang gedrückt

        S01[i] = WasPressed[i];
        S10[i] = WasReleased[i];

        /* SHORT?
        <-----> kleiner SHORT
        |~~~~~|
     ---|     |-----
              |
              v Event: short pressed
        */
        if (WasReleased[i])
        {
            if (millis() - LastPressed[i] <= SHORT)
                Sk[i] = true;
            WasReleased[i] = false;
            extBtnPressed = true;
        }
        else
            Sk[i] = false;

        /* LONG?
        <------------> größer SHORT
        |                  | > gleich LONG
        |~~~~~~~~~~~~~~~~~~|
     ---|             |    |-------
                        v Event: long pressed
        */
        if (IsPressed[i] && millis() - LastPressed[i] >= LONG && !LongNotified[i])
        {
            Sl[i] = true;
            LongNotified[i] = true;
            extBtnPressed = true;
        }
        else
            Sl[i] = false;
    }
    if (extBtnPressed)
    {
        int i = 0;
        Loco::loco[i] = LocoPage::currentLoco(); // aktuelle Lokdaten holen

        if(S01[0] || S01[1] || S01[2] || S10[0] || S10[1] || S10[2]) Loco::loco[i]->takenOver=false; // Steuerung übernehmen

        // Testweise S[0 .. 2] als Taster, Funktion aus locoData.csv
        if (S01[0]){
            extFktS0 = Loco::loco[i]->getS0();
            Serial.print("ExtFktS01[0] ");
            Serial.println(extFktS0);
            if (extFktS0 >=0 ) Z21::LAN_X_SET_LOCO_FUNCTION(Loco::loco[i]->getAddr(), extFktS0, 1);
        }
        if (S10[0]){
            extFktS0 = Loco::loco[i]->getS0();
            Serial.print("ExtFktS10[0] ");
            Serial.println(extFktS0);
            if (extFktS0 >=0 ) Z21::LAN_X_SET_LOCO_FUNCTION(Loco::loco[i]->getAddr(), extFktS0, 0);
        }

        if (S01[1]){
            extFktS1 = Loco::loco[i]->getS1();
            Serial.print("ExtFktS01[1] ");
            Serial.println(extFktS1);
            if (extFktS1 >=0 ) Z21::LAN_X_SET_LOCO_FUNCTION(Loco::loco[i]->getAddr(), extFktS1, 1);
        }
        if (S10[1]){
            extFktS1 = Loco::loco[i]->getS1();
            Serial.print("ExtFktS10[1] ");
            Serial.println(extFktS1);
            if (extFktS1 >=0 ) Z21::LAN_X_SET_LOCO_FUNCTION(Loco::loco[i]->getAddr(), extFktS1, 0);
        }

        if (S01[2]){
            extFktS2 = Loco::loco[i]->getS2();
            Serial.print("ExtFktS01[2] ");
            Serial.println(extFktS2);
            if (extFktS2 >=0 ) Z21::LAN_X_SET_LOCO_FUNCTION(Loco::loco[i]->getAddr(), extFktS2, 1);
        }
        if (S10[2]){
            extFktS2 = Loco::loco[i]->getS2();
            Serial.print("ExtFktS10[2] ");
            Serial.println(extFktS2);
            if (extFktS2 >=0 ) Z21::LAN_X_SET_LOCO_FUNCTION(Loco::loco[i]->getAddr(), extFktS2, 0);
        }

        if (Sl[4])
        {
            Z21::LAN_X_SET_TRACK_POWER(false);
            setAllChnFst0();
        }

        if (Sk[4])
        {
            if (Z21::getTrackPowerState() == BoolState::Off || Z21::getEmergencyStopState() == BoolState::On)
            {
                Z21::LAN_X_SET_TRACK_POWER(true);
                Z21::LAN_X_GET_STATUS();
            }
            else
            {
                Z21::LAN_X_SET_STOP();
                setAllChnFst0();
            }
        }
        if (Sk[3])  buttonPressed(M5Btn::extBtnChnR);
        if (Sl[3])  buttonPressed(M5Btn::extBtnChnL);
        if (Sk[5])  buttonPressed(M5Btn::extBtnFktUp);
        if (Sl[5])  buttonPressed(M5Btn::extBtnFktDown);
    }
    return extBtnPressed;
}