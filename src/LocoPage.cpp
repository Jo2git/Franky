#include <LocoPage.h>
#include <SPIFFS.h>
#include <M5Btn.h>
#include "functions.h"
#include <Pref.h>

// Ränder
#define BORDER_TOP 15
#define BORDER_LEFT 8
#define BORDER_RIGHT 8

#define FN_SET_ADR "set Adr"

void LocoPage::dump(char* message) {
  Serial.printf("%s (%s)\n", message, driveManually ? "manuell" : "aut. bremsen/beschl");
  for (int i=0; i<MAX_LOCO_CHANNELS; i++) {
    Serial.printf("%cCh %d Loco @%d=%d GUI @%d=%d\n",
      (i==channel ? '>':' '),
      i+1,
      loco[i]->getAddr(), loco[i]->fst,
      addr[i]->getValue(), loco[i]->targetFst
    );
  }
  Serial.println();
}

Loco* LocoPage::selectedLoco;

// ----------------------------------------------------------------------------------------------------
// Konstruktor: Oberfläche initialisieren

#define BTNSIZE 30
#define DIAMETER 20

LocoPage::LocoPage(char navigable) : Page(navigable) {

  for (int i=0; i<MAX_LOCO_CHANNELS; i++) {
    loco[i] = Loco::addLoco(Pref::get(prefNameLocoChannelAddr + String(i), String(i+1)).toInt());
  }

  selectedLoco = Loco::addLoco(Pref::get(prefNameLocoChannelAddr + String("2"), "3").toInt());

  // Widgets

  widgets[numWidgets++] = locoInfo = new Textbox(tft, 0, 60, selectedLoco->name, TFT_W/2, 60, CC_DATUM, 4);
  widgets[numWidgets++] = speed = new Numberbox(tft, 0, "%d", 7, 0, 0, 126, TFT_W/2, TFT_H/2, CC_DATUM, 8);
  speed->setIncrement(1);
  widgets[numWidgets++] = targetSpeed = new Numberbox(tft, 0, "%d", 8, 0, 0, 126, TFT_W - BORDER_RIGHT, TFT_H/2, MR_DATUM, 4);
  targetSpeed->setIncrement(10);
  widgets[numWidgets++] = tachoSpeed = new Numberbox(tft, 0, "%d", 8, 0, 0, 600, TFT_W - BORDER_RIGHT, TFT_H/2 + 32, MR_DATUM, 4);
  widgets[numWidgets++] = direction = new Symbolbox(tft, TRIANGLE_UPDOWN | NOAUTOFOCUS, false, BTNSIZE, BTNSIZE, BORDER_LEFT + BTNSIZE, TFT_H/2);
  widgets[numWidgets++] = headLights = new Symbolbox(tft, CIRCLE_FILLEDEMPTY | NOAUTOFOCUS, true, DIAMETER, DIAMETER, BORDER_LEFT + BTNSIZE, TFT_H/2 - 40);
  widgets[numWidgets++] = rearLights = new Symbolbox(tft, CIRCLE_FILLEDEMPTY | NOAUTOFOCUS, true, DIAMETER, DIAMETER, BORDER_LEFT + BTNSIZE, TFT_H/2 + 40);

  for (int i=0; i<MAX_LOCO_CHANNELS; i++) {
    widgets[numWidgets++] = addr[i] = new Numberbox(tft, NOAUTOFOCUS, "%d", 12, Pref::get(prefNameLocoChannelAddr + String(i), String(i+1)).toInt(), 1, MaxLocoAddr, TFT_W*(2*i+1)/10, BORDER_TOP, TC_DATUM, 4);
  }

  for (int i=numSoftkeys; i<MAX_SOFT_KEYS; i++) softkeys[i] = 0;

  useExtBtn = Pref::get(prefNameExtButton, "off") == "on";
  useAdvLigths = Pref::get(prefNameAdvLigth, "off") == "on";

if(useExtBtn) {
  // Softkeys Ebene 0
  softkeys[numSoftkeys++] = drivingModeSoftkey = new Softkey(tft, 0, FN_DRIVE_AUTO, M5Btn::C, LAYER0, TFT_WHITE, TFT_DARKGREY, TFT_BLACK);
  softkeys[numSoftkeys++] = libModeSoftkey = new Softkey(tft, 0, FN_CHANGE_ADDR, M5Btn::CC, LAYER0, TFT_WHITE, TFT_DARKGREY, TFT_BLACK);
} else {
  // Softkeys Ebene 0
  softkeys[numSoftkeys++] = new Softkey(tft, 0, FN_HEADLIGHTS, M5Btn::A, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = drivingModeSoftkey = new Softkey(tft, 0, FN_DRIVE_AUTO, M5Btn::B, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, FN_CHANNELS_PLUS, M5Btn::C, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = addrStepsSoftKey = new Softkey(tft, 0, CAPTION_UP, M5Btn::AA, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = libModeSoftkey = new Softkey(tft, 0, FN_CHANGE_ADDR, M5Btn::BB, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, FN_CHANNELS_MINUS, M5Btn::CC, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
}
  // Softkeys Ebene 1 - nur für Adressschritte
  softkeys[firstDeltaKey = numSoftkeys++] = new Softkey(tft, 0, "1", M5Btn::A, LAYER1, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, "10", M5Btn::B, LAYER1, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, "100", M5Btn::C, LAYER1, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, "set Adr", M5Btn::AA, LAYER1, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, "50", M5Btn::BB, LAYER1, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[lastDeltaKey = numSoftkeys++] = new Softkey(tft, 0, "500", M5Btn::CC, LAYER1, TFT_WHITE, TFT_BLUE, TFT_BLACK);

  firstFunctionSoftkeyIndex = numSoftkeys;

  // Modi
  inLibMode = Pref::get(prefNameLocoLib, "off") == "on";
  libModeSoftkey->setActivated(inLibMode);
  driveManually = Pref::get(prefNameDriveAutomatically, "off") == "off";
  drivingModeSoftkey->setActivated(!driveManually);
  setMinMaxAddr();

  if (driveManually) {
    speed->setFocus(true);
    targetSpeed->setVisible(false);
  } else {
    targetSpeed->setFocus(true);
  }
  addr[channel]->setSelected(true);

  setFunctionSoftkeys();

  // Etc

  Z21::addObserver(static_cast<Page*>(this));

}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::setFunctionSoftkeys() {

  numSoftkeys = firstFunctionSoftkeyIndex;

  for (int i=numSoftkeys; i<MAX_SOFT_KEYS; i++) {
    if (softkeys[i] != 0) {
      delete(softkeys[i]); // da ja dynamisch - siehe unten - je Lok neu erzeugt wird: alte Softkeys abräumen
      softkeys[i]=0;
    }
  }

  int l=1;

if (useExtBtn){
  softkeys[numSoftkeys++] = headLightsSoftkey = new Softkey(tft, 0, FN_HEADLIGHTS, M5Btn::AA, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(1)->getShortName(), M5Btn::A, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(2)->getShortName(), M5Btn::B, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(3)->getShortName(), M5Btn::BB, LAYER0, TFT_WHITE, TFT_BLUE, TFT_BLACK);
  // Ab Ebene 2 Funktionstasten
  for (int f=4; f <= loco[channel]->getNumFct(); f++) {
    switch(f%6) {
      case 4: // neue Seite beginnen (++) Funktionstaste auf A
        l++;
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::A, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 5:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::AA, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 0:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::B, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 1:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::BB, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 2:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::C, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 3:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::CC, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
    }
  }
} else {
  // Ab Ebene 2 Funktionstasten
  for (int f=1; f <= loco[channel]->getNumFct(); f++) {
    switch(f%4) {
      case 1: // neue Seite beginnen (++) -> Navigationstasten und Funktionstaste auf BB
        l++;
        softkeys[numSoftkeys++] = new Softkey(tft, 0, CAPTION_DOWN, M5Btn::A, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        softkeys[numSoftkeys++] = new Softkey(tft, 0, CAPTION_UP, M5Btn::AA, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::B, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 2:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::C, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 3:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::BB, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
      case 0:
        softkeys[numSoftkeys++] = new Softkey(tft, 0, loco[channel]->getFct(f)->getShortName(), M5Btn::CC, l, TFT_WHITE, TFT_BLUE, TFT_BLACK);
        break;
    }
  }
}
  numLayers=l+1;
  if(!useExtBtn)  handleAddrStepKey();
}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::handleAddrStepKey() {
  String oldCaption = addrStepsSoftKey->getCaption();
  String newCaption;

  // In Ebene 0 keine Umsschaltfunktion zu weiteren F-Seiten nötig, wenn nur F0 benötigt werden
  if (layer == LAYER0  && loco[channel]->getNumFct() == 0) {
    newCaption = "";

  // in Layer 1 nur Deltaschritte, von dort Zurückschalten abieten
  } else if (layer == LAYER1) {
    newCaption = CAPTION_DOWN;

  // Ansonsten Umschalttaste für weitere F-Seiten nötig
  } else {
    newCaption = CAPTION_UP;
  }

  if (newCaption != oldCaption) {
      addrStepsSoftKey->setVisible(false);
      if (newCaption == "") return;
      addrStepsSoftKey->setCaption(newCaption)->setVisible(true);
  }

}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::setMinMaxAddr() {
  for (int c=0; c<MAX_LOCO_CHANNELS; c++) {
    addr[c]->setMinValue(inLibMode ? Loco::getMinAddress() : 1);
    addr[c]->setMaxValue(inLibMode ? Loco::getMaxAddress() : MaxLocoAddr);
  }
}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::setVisible(bool visible, bool clearScreen) {
  Page::setVisible(visible, clearScreen);
  if (visible) {
    navigationHint();
    Z21::LAN_X_GET_LOCO_INFO(addr[channel]->getValue());
  }
  setButtons(layer);
  if (!useExtBtn) addrStepsSoftKey->setVisible(inAddressChangeMode || loco[channel]->getNumFct() != 0);
}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::driveLoco() {
  loco[channel]->takenOver=false;
  Z21::LAN_X_SET_LOCO_DRIVE(
    loco[channel]->getAddr(),
    loco[channel]->forward ? Forward : Backward,
    loco[channel]->fst);
}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::focusChanged() {}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::buttonPressed(M5Btn::ButtonType button) {

  loco[channel]->takenOver=false; // Steuerung übernehmen

  // Not-Halt / Gleisspannung AUS mit Doppeldruck (2-stufig)
  if (!useExtBtn && (button == M5Btn::RotaryKnobDouble || button == M5Btn::RotaryKnobDoubleLong)) {
    if (button == M5Btn::RotaryKnobDoubleLong || Z21::getEmergencyStopState() == BoolState::On) {
      Z21::LAN_X_SET_TRACK_POWER(false);
    } else {
      Z21::LAN_X_SET_STOP();
      }
    loco[channel]->fst  = 0;
    loco[channel]->targetFst = 0;
    loco[channel]->speed = 0;
    driveLoco();
  }

  // ===================== Fahren

   // Links-/Rechtsdreh -> Werterniedrigung/-erhöhung (Geschwindigkeit)
  if ((button == M5Btn::RotaryLeft || button == M5Btn::RotaryRight) && ((Z21::getTrackPowerState() == BoolState::On && Z21::getEmergencyStopState() == BoolState::Off) && focussedWidget() != addr[channel])) {
      focussedWidget()->buttonPressed(button);
    if (focussedWidget() == speed) {
      targetSpeed->setValue(speed->getValue())->setVisible(false);
      if (speed->getValue() <= MaxFst){               // erstmal als Bastellösung f. H.-J.Rothkötter
        loco[channel]->fst = loco[channel]->targetFst = speed->getValue();
        loco[channel]->speed = speed->getValue();
      }
      driveLoco();
    }

    if (focussedWidget() == targetSpeed) {
      loco[channel]->targetFst = targetSpeed->getValue();
    }

  // Licht
  } else  if (getFunction(button) == FN_HEADLIGHTS) {
      headLights->setValue(direction->getValue()||loco[channel]->getFct(3)->isActive());
      headLights->setVisible(!loco[channel]->getFct(0)->isActive());
      loco[channel]->getFct(0)->setActive(!loco[channel]->getFct(0)->isActive());

    if (useAdvLigths && useExtBtn) {
      headLightsSoftkey->setVisible(true);
      headLightsSoftkey->setActivated(loco[channel]->getFct(0)->isActive());
    }
    Z21::LAN_X_SET_LOCO_FUNCTION(loco[channel]->getAddr(), 0, loco[channel]->getFct(0)->isActive());

  // Funktionen
  } else if (layer != 1 && !inAddressChangeMode && loco[channel]->getFctByName(getFunction(button)) != 0) {
    Softkey* softkey = getSoftkey(getFunction(button));
    softkey->setActivated(!softkey->isActivated())->setVisible(true);
    Locofunction* fct = loco[channel]->getFctByName(getFunction(button));
    fct->setActive(softkey->isActivated());
    Z21::LAN_X_SET_LOCO_FUNCTION(addr[channel]->getValue(), fct->getMappedTo(), softkey->isActivated());

  // Gleisspannung EIN (Stopp auflösen), wenn notwendig
  } else if (button == M5Btn::RotaryKnob && !useExtBtn && (Z21::getTrackPowerState() == BoolState::Off || Z21::getEmergencyStopState() == BoolState::On)) {
    Z21::LAN_X_SET_TRACK_POWER(true);
    Z21::LAN_X_GET_STATUS();

  // Manueller Betrieb: Fahrstufe 0 setzen oder Richtungswechsel
  } else if (button == M5Btn::RotaryKnob && focussedWidget() == speed) {
    // Wenn schon bei 0 -> Richtungswechsel
    if (speed->getValue() == 0) {
      direction->toggle();
      loco[channel]->forward = direction->getValue();
    } else {
      speed->setMinValue(); loco[channel]->fst  = speed->getValue();
      targetSpeed->setMinValue();  loco[channel]->targetFst = targetSpeed->getValue();
      loco[channel]->speed = speed->getValue();
    }
    driveLoco();

  // Fahrstufenschalter
  } else if (button == M5Btn::RotaryKnobLong && focussedWidget() == speed && Z21::getTrackPowerState() == BoolState::On && Z21::getEmergencyStopState() == BoolState::Off) {
    if (loco[channel]->fst >= Fst3) {
        speed->setValue(MaxFst);
    }
    if (loco[channel]->fst >= Fst2 and loco[channel]->fst < Fst3) {
        speed->setValue(Fst3);
    }
    if (loco[channel]->fst >= Fst1 and loco[channel]->fst < Fst2) {
        speed->setValue(Fst2);
    }
    if (loco[channel]->fst < Fst1) {
        speed->setValue(Fst1);
    }
    loco[channel]->fst = speed->getValue();
    loco[channel]->speed = speed->getValue();
    driveLoco();

  // Automatischer Betrieb: Fahrstufe 0 setzen oder Richtungswechsel
  } else if (button == M5Btn::RotaryKnob && focussedWidget() == targetSpeed) {
    // Wenn schon bei 0 -> Richtungswechsel
    if (speed->getValue() == 0) {
      direction->toggle();
      loco[channel]->forward = direction->getValue();
    } else if (targetSpeed->getValue() > 10) {
      // solange Zielgeschwindigkeit > 10, alles wie gehabt
      targetSpeed->setMinValue();  loco[channel]->targetFst = targetSpeed->getValue();
    } else {
      // sonst Schnellbremsung (wegen Bremsquietschen)
      speed->setMinValue(); loco[channel]->fst  = speed->getValue();
      targetSpeed->setMinValue();  loco[channel]->targetFst = targetSpeed->getValue();
      loco[channel]->speed = speed->getValue();
    }
    driveLoco();

  // Maximalfahrstufe
  } else if (button == M5Btn::RotaryKnobLong && (focussedWidget() == speed || focussedWidget() == targetSpeed) && (Z21::getTrackPowerState() == BoolState::On && Z21::getEmergencyStopState() == BoolState::Off)) {
    if (focussedWidget() == speed) { speed->setMaxValue(); loco[channel]->fst = speed->getValue(); }
    targetSpeed->setMaxValue(); loco[channel]->targetFst = targetSpeed->getValue();
    driveLoco();

  // ===================== Ebenenwechsel
  } else if (getFunction(button) == CAPTION_UP || button == M5Btn::extBtnFktUp) {
    if(inAddressChangeMode) {
      if (layer == 0 && !inLibMode) layer = 1;
      else layer = 0;
    }
    else {
      if (layer == 0) layer +=2;
      else layer++;
    }
    layer = layer % numLayers;
    setButtons(layer);

  } else if (getFunction(button) == CAPTION_DOWN || button == M5Btn::extBtnFktDown) {
    if(inAddressChangeMode) {
      if (layer == 0 && !inLibMode) layer = 1;
      else layer = 0;
    }
    else {
      if (layer == 2) layer = 0;
      else layer = layer - 1 + numLayers ;
      if (layer == 1) layer = 0;
    }
    layer = layer % numLayers;
    setButtons(layer);

  // ===================== Lokauswahl
  // im LibMode
  } else if ((button == M5Btn::RotaryLeft || button == M5Btn::RotaryRight) && (focussedWidget() == addr[channel] && inLibMode)) {
    focussedWidget()->buttonPressed(button);
    if (button == M5Btn::RotaryLeft) while (Loco::existsLoco(addr[channel]->getValue()) == 0 && addr[channel]->getValue() > Loco::getMinAddress()) addr[channel]->increase(-1);
    else while (Loco::existsLoco(addr[channel]->getValue()) == 0 && addr[channel]->getValue() < Loco::getMaxAddress()) addr[channel]->increase(+1);
    loco[channel] = Loco::addLoco(addr[channel]->getValue()); // falls noch nicht im Zugriff
    locoAddressChanged();

  // mit Adresseingabe
  } else if (
      getFunction(button) ==   "1" ||
      getFunction(button) ==  "10" ||
      getFunction(button) ==  "50" ||
      getFunction(button) == "100" ||
      getFunction(button) == "500") {
    int incr = getFunction(button).toInt();
    for (int i=0; i<MAX_LOCO_CHANNELS; i++) addr[i]->setIncrement(incr);
    addr[channel]->increase(incr);

  // ===================== andere Tasten
  } else if (getFunction(button) == FN_CHANGE_ADDR) {
    // Sonderfall: wenn bereits im Adressmodus wird Umschalten des Bibliotheksmodus vollzogen ...
    if (inAddressChangeMode) {
      inLibMode = !inLibMode;
      setMinMaxAddr();
      libModeSoftkey->setActivated(inLibMode)->setVisible(true);
      if (!inLibMode) {   // Wechsel zur Adresseingabe
        layer = 1;
        setButtons(layer);
      }
    } else {
      inAddressChangeMode = true;
      if(!useExtBtn) addrStepsSoftKey->setVisible(true);
      focussedWidget()->setFocus(false)->setVisible(true);
      addr[channel]->setFocus(true)->setVisible(true);
    }

  // Adresskanäle nach rechts durchschalten
  } else if (getFunction(button) == FN_CHANNELS_PLUS || button == M5Btn::extBtnChnR) {
    setButtons(0);
    Pref::set(prefNameLocoChannelAddr + String(channel), String(addr[channel]->getValue()));
    int oldChannel = channel;
    channel = (channel + 1) % MAX_LOCO_CHANNELS;
    locoChannelChanged(oldChannel, channel);

  // Adresskanäle nach links durchschalten
  } else if (getFunction(button) == FN_CHANNELS_MINUS || button == M5Btn::extBtnChnL) {
    setButtons(0);
    Pref::set(prefNameLocoChannelAddr + String(channel), String(addr[channel]->getValue()));
    int oldChannel = channel;
    channel = (channel - 1 + MAX_LOCO_CHANNELS) % MAX_LOCO_CHANNELS;
    locoChannelChanged(oldChannel, channel);

  // Minimaladresse
  } else if (button == M5Btn::RotaryKnob && focussedWidget() == addr[channel]) {

    if (inLibMode) {
      addr[channel]->setValue(Loco::getMinAddress())->setVisible(true);
    } else {
      addr[channel]->setMinValue();
    }
    locoAddressChanged();

  // Maximaladresse
  } else if (button == M5Btn::RotaryKnobLong && focussedWidget() == addr[channel]) {
    if (inLibMode) addr[channel]->setValue(Loco::getMaxAddress())->setVisible(true);
    locoAddressChanged();

  // Nur Zurückschalten vom Adressmodus -> alten driveManually-State berücksichtigen, nicht umschalten!
  } else if (inAddressChangeMode && (getFunction(button) == FN_DRIVE_AUTO || getFunction(button) == FN_SET_ADR)) {
    loco[channel] = Loco::addLoco(addr[channel]->getValue()); // falls noch nicht im Zugriff
    locoAddressChanged();
    inAddressChangeMode = false;
    focussedWidget()->setFocus(false)->setVisible(true);
    if (getFunction(button) == FN_SET_ADR) {
      layer = 0;
      setButtons(layer);
      for (int i=0; i<MAX_LOCO_CHANNELS; i++) addr[i]->setIncrement(1);
    }
    if (driveManually) speed->setFocus(true)->setVisible(true);
    else targetSpeed->setFocus(true)->setVisible(true);

  // Fahrmodus manuell -> automatisch
  } else if (getFunction(button) == FN_DRIVE_AUTO && driveManually) {
    inAddressChangeMode = false;
    focussedWidget()->setFocus(false)->setVisible(true);
    speed->setFocus(false)->setVisible(true);
    targetSpeed->setFocus(true)->setVisible(true);
    driveManually = false;
    drivingModeSoftkey->setActivated(!driveManually)->setVisible(true);
    setButtons(LAYER0);
    if(!useExtBtn) addrStepsSoftKey->setVisible(loco[channel]->getNumFct() != 0);

  // Fahrmodus automatisch -> manuell
  } else if (getFunction(button) == FN_DRIVE_AUTO && !driveManually) {
    inAddressChangeMode = false;
    if (focussedWidget() == targetSpeed) {
      targetSpeed->setFocus(false)->setVisible(false);
    } else focussedWidget()->setFocus(false)->setVisible(true);
    speed->setFocus(true)->setVisible(true);
    targetSpeed->setFocus(false)->setVisible(false);
    driveManually = true;
    drivingModeSoftkey->setActivated(!driveManually)->setVisible(true);
    setButtons(LAYER0);
    if(!useExtBtn) addrStepsSoftKey->setVisible(loco[channel]->getNumFct() != 0);
  }

}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::locoChannelChanged(int oldChannel, int channel) {

  bool hadFocus = addr[oldChannel]->hasFocus();

  // gewähltes Adressfeld gemäß Channel selektieren
  for (int i=0; i<MAX_LOCO_CHANNELS; i++) {
    addr[i]->setSelected(i==channel);
    addr[i]->setFocus(false)->setVisible(true);
  }
  addr[channel]->setFocus(hadFocus)->setVisible(true);

  selectedLoco = loco[channel];
  locoAddressChanged();
  update();
}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::locoAddressChanged() {
  int locoAddr = addr[channel]->getValue();

  Loco* loc = Loco::existsLoco(locoAddr);
  if (loc != 0) {
    locoInfo->setValue(loc->name)->setVisible(true);
  } else  {
    locoInfo->setVisible(false);
  }

  Z21::LAN_X_GET_LOCO_INFO(locoAddr);

  // mit neuer Adresse auch die Softkeys für Funktionen anpassen
  setFunctionSoftkeys();
}

// ----------------------------------------------------------------------------------------------------
//

void LocoPage::update() {

  speed->setValue(loco[channel]->fst);
  targetSpeed->setValue(loco[channel]->targetFst);
  if (!driveManually) targetSpeed->setVisible(true);
  else targetSpeed->setVisible(true, TFT_BLACK); //(false) funktioniert z.Zt. nicht richtig

  // Geschwindigkeit (Tacho) berechnen und anzeigen, falls vmax bekannt
  if (loco[channel]->getVmax() > 0) {
    tachoSpeed->setValue(loco[channel]->fst * loco[channel]->getVmax() / MaxFst);
    tachoSpeed->setVisible(true, TFT_LIGHTGREY);
  } else tachoSpeed->setVisible(true, TFT_BLACK); //(false) funktioniert z.Zt. nicht richtig

  direction->setValue(loco[channel]->forward)->setVisible(true);
  if (useAdvLigths) {
    headLights->setValue(direction->getValue()||loco[channel]->getFct(3)->isActive());
    rearLights->setValue(!direction->getValue()||loco[channel]->getFct(3)->isActive());
    headLights->setVisible(loco[channel]->getFct(0)->isActive()||(loco[channel]->getFct(3)->isActive()&&loco[channel]->getFct(1)->isActive()));
    rearLights->setVisible(loco[channel]->getFct(1)->isActive()||(loco[channel]->getFct(3)->isActive()&&loco[channel]->getFct(0)->isActive()));
  } else {
    headLights->setValue(true);
    headLights->setVisible(loco[channel]->getFct(0)->isActive());
    rearLights->setVisible(false);
  }

  // Funktionszustände auf zugehörige Buttons übertragen
  for (int i=1; i<=loco[channel]->getNumFct(); i++) {
    for (int j=0; j<numSoftkeys; j++) {
      if (loco[channel]->getFct(i)->getShortName() == softkeys[j]->getCaption()) {
        softkeys[j]->setActivated(loco[channel]->getFctMappedTo(i)->isActive());
        // Aktualisieren, wenn gerade sichtbar
        if (layer == softkeys[j]->getLayer()) softkeys[j]->setVisible(true);
      }
    }
  }
  if (useExtBtn && useAdvLigths && layer == 0) {
    headLightsSoftkey->setVisible(true);
    headLightsSoftkey->setActivated(loco[channel]->getFct(0)->isActive());
  }

  if (loco[channel]->takenOver) {
    speed->setVisible(true, COLOR_TAKENOVER);
  } else {
    if (loco[channel]->isAccelerating()) speed->setVisible(true, COLOR_ACCELERATING);
    if (loco[channel]->isDecelerating()) speed->setVisible(true, COLOR_DECELERATING);
    if (loco[channel]->isCoasting()) speed->setVisible(true);
  }

}

// ----------------------------------------------------------------------------------------------------
// Lok ist gefahren worden ...

// ... durch Notifikation der Zentrale

void LocoPage::locoInfoChanged(int address, Direction dir, int fst, bool takenOver, int numSpeedSteps, bool f[]) {
  int maxFct = (Pref::get(prefNameFkt31, "off")) ? MaxFct : 28;

  if (!visible) return;

  if (address == addr[channel]->getValue()) {
    loco[channel]->forward = dir == Forward;
    if (driveManually) loco[channel]->targetFst = fst;
    if (takenOver && (loco[channel]->isCoasting())) loco[channel]->takenOver = true;
    if (driveManually) loco[channel]->fst = fst; // hier! Funktioniert nicht für Fremdssteuern?
    if (loco[channel]->takenOver) {
      loco[channel]->targetFst = fst;
      loco[channel]->speed = fst;
      loco[channel]->fst = fst;
    }
    for (int j=0; j<maxFct+1; j++)  {
      loco[channel]->getFct(j)->setActive(f[j]);
    }
    update();
  }

}

// ... durch Loco::drive()

void LocoPage::locoWasDriven(int addr) {
  if (visible) {
    update();
  }
}
