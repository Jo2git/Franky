#ifndef LOCO_H
#define LOCO_H

#include <Z21.h>
#include "configuration.h"
#include "Locofunction.h"

#define MaxFst 126
#define Fst1 MaxFst/4 + 1
#define Fst2 Fst1 + Fst1
#define Fst3 Fst2 + Fst1
#define LOCO_FILE_NAME "/locoData.csv"

class Loco {

  public:
    int fst = 0;
    int targetFst = 0;
    float speed = fst;

    String name = ""; // Betriebsnummer
    String decoder = "";
    String fctSpec = "";

    boolean forward = true;
    boolean takenOver = false;
    
    int lastFunction = 5;

    int getAddr() { return addr; }
    Locofunction* getFct(int index) { if (index>=0 && index< MaxFct + 1) return fct[index]; }
    Locofunction* getFctMappedTo(int index) ; //{ if (mappedTo>=0 && mappedTo< MaxFct + 1) return fct[mappedTo]; }
    Locofunction* getFctByName(String shortName);
    int getNumFct() { return numFct; }
    void setNumFct(int numFct) { this->numFct = numFct; }

    int getAcc() { return acc; }
    int getDec() { return dec; }
    int getVmax() { return vmax; }
    int getS0() { return (s0 >= 0 && s0 <= MaxFct) ? s0 : -1; }
    int getS1() { return (s1 >= 0 && s1 <= MaxFct) ? s1 : -1; }
    int getS2() { return (s2 >= 0 && s2 <= MaxFct) ? s2 : -1; }

    static Loco* loco[MAX_LOCOS];

    static void begin();

    static Loco* addLoco(int addr);

    static Loco* getLoco(int addr);
    static Loco* existsLoco(int addr);
    static void setLoco();

    static int getMinAddress() { return minAddress; }
    static int getMaxAddress() { return maxAddress; }


    char setTargetFst(int value);
    char increaseAcceleration(int8_t delta);
    char increaseDeceleration(int8_t delta);

    void increaseAddress(int8_t delta);

    bool isAccelerating();
    bool isDecelerating();
    bool isCoasting();

    // Regelmäßig von Scheduler zu rufende Loksteuerung (bremst/beschleunigt alle Loks)
    static void drive();

    // Regelmäßig von Scheduler zu rufende Auffrischen der Lokinfo, damit Notifikationen noch ankommen
    static void refresh();  

    static void dumpLocos(bool all);

  private:
    Loco(int itsAddr);
    ~Loco();
    
    int addr = 3;
    int acc = 5;
    int dec =5;
    int vmax = 0;
    int numFct = MaxFct;
    Locofunction* fct[MaxFct+1];
    int s0 = -1;
    int s1 = -1;
    int s2 = -1;
    static int minAddress, maxAddress;
    static void readLocoData();
};

#endif
