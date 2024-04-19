#ifndef LOCOPAGE_H
#define LOCOPAGE_H

#include "Page.h"
#include <Z21Observer.h>
#include "Symbolbox.h"
#include "Numberbox.h"
#include "Textbox.h"
#include "Loco.h"
#include "configuration.h"

class LocoPage: public Page, public Z21Observer {

  public:

    LocoPage(char navigable);
    virtual void setVisible(bool visible, bool clearScreen) override;
    virtual bool isBeta() override { return false; }
    virtual void buttonPressed(M5Btn::ButtonType btn) override;
    virtual void focusChanged() override;

    // Zuletzt bediente Lok
    static Loco* currentLoco() { return selectedLoco; }

    // Von Z21 erhaltene Notifikationen
    virtual void locoInfoChanged(int addr, Direction dir, int fst, bool takenOver, int numSpeedSteps, bool f[]) override;

    // Durch automatisches Bremsen/Beschleunigen geänderte Geschwindigkeit
    virtual void locoWasDriven(int addr);

  private:

    // Lok ansteuern gemäß angeforderter Werte
    void driveLoco();

    // Anzeige/Refresh der Daten der aktuellen Lok im Display
    void update();

    // Gewählter Lokkanal hat sich geändert
    void locoChannelChanged(int oldChannel, int channel);

    // Adresse wurde geändert
    void locoAddressChanged();

    // Min- und Maxwerte für Lokadresse (in Abhängigkeit vom durch selectKnownAddresses bestimmten Modus) setzen
    void setMinMaxAddr();

    // Im Adressmodus?
    bool inAddressChangeMode = false;
    // void handleDeltaButton();

    // Nur bekannte Loks wählen ("Bibliotheksmodus")?
    static bool libMode;

    // Anzahl Layer
    int numLayers;

    // Besonderheit der Taste handeln
    void handleAddrStepKey();

    // Mit jeder Lok und deren unterschiedlicher Funktionszahl betr. Softkeys neu setzen
    void setFunctionSoftkeys();

    // Gespeicherte Loks je Lokkanal
    Loco* loco[MAX_LOCO_CHANNELS];

    // Aktueller Lokkanal, anfangs der in der Mitte
    int channel = MAX_LOCO_CHANNELS/2;

    // Zuletzt bediente Lok
    static Loco* selectedLoco;

    // Widgets für Bedienung
    Numberbox* addr[MAX_LOCO_CHANNELS]; 
    Textbox* locoInfo;
    Symbolbox* direction;
    Symbolbox* headLights;
    Symbolbox* rearLights;
    Numberbox* speed;
    Numberbox* targetSpeed;
    Numberbox* tachoSpeed;

    // Modi
    bool driveManually = false; // Fahrmodus. true: manuell fahren, false: automatisch bremsen/beschleunigen
    bool inLibMode = false; // Librarymodus
    bool useExtBtn = false; // Navigation mit ext. Tasten (seitlich)
    bool useAdvLigths = false; // Erweiterte Lichttanzeige (FS1/FS2 - ws/rt )

    // Bestimmte Softkeys, auf die gesondert zugegriffen wird
    int firstFunctionSoftkeyIndex; // Softkeyindex der ersten Funktionstaste 
    Softkey* addrStepsSoftKey; // Softkey, mit der Adressschrittweiten ausgewählt werden kann. Wird nur angezeigt, wenn selectKnownAddresses = false
    Softkey* drivingModeSoftkey; // manuell/automatisch
    Softkey* libModeSoftkey; // Bibliotheksmodus 
    Softkey* headLightsSoftkey; // Frontlichter
    int firstDeltaKey, lastDeltaKey; // Index erste und letzte Softkey für Adressdeltaänderung

    void dump(char* message);

};

#endif