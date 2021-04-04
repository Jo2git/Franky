#include "Page.h"
#include "LocoPage.h"
#include "ProgPage.h"
#include "M5Btn.h"
#include "functions.h"
#include <M5Stack.h>
#include <configuration.h>
#include <Z21.h>

TFT_eSPI* Page::tft;

int Page::fgColor = TFT_WHITE;
int Page::bgColor = TFT_BLACK;

int Page::row;
int Page::col;

Page* Page::navigationGrid[PAGE_ROWS][PAGE_COLS];
bool Page::blocked = false;

int Page::oldFocusIndex, Page::newFocusIndex;


// ----------------------------------------------------------------------------------------------------
//

void Page::begin(TFT_eSPI* tft) {

  M5Btn::begin(tft);
  Page::tft = tft;

  ProgPage* pp = new ProgPage(NAV_ALL);
  LocoPage* lp = new LocoPage(NAV_ALL);

  // Initialisierung oben hat nicht funktioniert, Konstruktoren wurden nicht gerufen
  Page* tmp[PAGE_ROWS][PAGE_COLS] = {
      {0, lp, pp}
    };

    memcpy(navigationGrid, tmp, sizeof(tmp));

    // initiale Seite
    row = 0; col = 1; 
}

// ----------------------------------------------------------------------------------------------------
//

void Page::loop() {
}

// ----------------------------------------------------------------------------------------------------
//

#define NAV_IND_LEN  10 // Länge des Indikatorbalkens
#define NAV_IND_WIDTH 4 // Dicke 

void Page::setVisible(bool visible, bool clearScreen) {

  this->visible = visible;
  M5Btn::clearFunctions(); M5Btn::setFunction(M5Btn::B, FN_FOCUS);
  if (clearScreen) tft->fillScreen(bgColor);
  for (int i=0; i<this->numWidgets; i++) this->widgets[i]->setVisible(visible);

}

// ----------------------------------------------------------------------------------------------------
//

void Page::navigationHint() {

  if (blocked) return;

  //----------------- Diese Stati nicht mehr im Record, sondern in Notifikation, oder???? Ansonsten fehlen sie dort

  int color = colorAllConnected;
  if (WiFi.status() != WL_CONNECTED) color = colorWiFiDisconnected;
  else if (millis() - Z21::lastReceived > Z21_HEARTBEAT) color = colorDigiStationDisconnected;
  else if (!Z21::isProgStateOff()) color = colorProgMode;
  else if (Z21::isTrackPowerOff()) color = colorTrackPowerOff;

  // Status: Balken oben
  tft->fillRect(0, 0, TFT_W, NAV_IND_WIDTH, color);

  // Am jeweiligen Rand Navigationshinweis (dass es dort eine Nachbarseite gibt)

  // oben
  if (row > 0) {
    if (navigationGrid[row-1][col] != 0) 
      if ((currentPage()->navigable & NAV_UP))
        tft->fillRect(tft->width()/2-NAV_IND_LEN/2, 0, NAV_IND_LEN, NAV_IND_WIDTH, fgColor);
  }
  // unten
  if (row < PAGE_ROWS - 1) {
    if (navigationGrid[row+1][col] != 0) 
      if ((currentPage()->navigable & NAV_DOWN))
        tft->fillRect(tft->width()/2-NAV_IND_LEN/2, tft->height()-NAV_IND_WIDTH+1, NAV_IND_LEN, NAV_IND_WIDTH, fgColor);
  }
  // links
  if (col > 0) {
    if (navigationGrid[row][col-1] != 0) 
      if ((currentPage()->navigable & NAV_LEFT))
        tft->fillRect(0, tft->height()/2-NAV_IND_LEN/2, NAV_IND_WIDTH, NAV_IND_LEN, fgColor);
  } 
   // rechts
  if (col < PAGE_COLS - 1) {
    if (navigationGrid[row][col+1] != 0)
      if ((currentPage()->navigable & NAV_RIGHT))
        tft->fillRect(tft->width()-NAV_IND_WIDTH+1, tft->height()/2-NAV_IND_LEN/2, NAV_IND_WIDTH, NAV_IND_LEN, fgColor);
  } 

}

Page* Page::currentPage() {
  return navigationGrid[row][col];
}

// ----------------------------------------------------------------------------------------------------
//

void Page::handlePageSwitchAndFocus(M5Btn::ButtonType button) {

  if (blocked) return;

  int oldCol = col, oldRow = row;

  // Umschalten auf Nachbarseite, sofern diese existiert (zweites if) und
  // wenn dorthin navigieren erlaubt (letztes if)

  if (button == M5Btn::AB) { // nach oben
      if (row > 0) if (navigationGrid[row - 1][col] != 0) {
        if ((currentPage()->navigable & NAV_UP) > 0) row--;
      }
  } else if (button == M5Btn::BC) { // nach unten
      if (row < PAGE_ROWS - 1) if (navigationGrid[row + 1][col] != 0) {
        if ((currentPage()->navigable & NAV_DOWN) > 0) row++;
      }
  } else if (button == M5Btn::CA) { // nach links
    Serial.println("nach links");

      if (col > 0) if (navigationGrid[row][col - 1] != 0) {
        if ((currentPage()->navigable & NAV_LEFT) > 0) col--;
      }
  } else if (button == M5Btn::AC) { // nach rechts
  Serial.println("nach rechts");
      if (col < PAGE_COLS - 1) if (navigationGrid[row][col + 1] != 0) {
        if ((currentPage()->navigable & NAV_RIGHT) > 0) col++;
      }
  }

  // Falls es neue Seite gibt, alte verbergen und neue anzeigen
  if (oldCol != col || oldRow != row) {
    navigationGrid[oldRow][oldCol] -> setVisible(false, false);
    navigationGrid[row][col] -> setVisible(true, true);
  }
  
  // Fokuswechsel
  if (button == M5Btn::B) {
    
    oldFocusIndex = -1;

    for (int i=0; i<currentPage()->numWidgets; i++) 
      if (currentPage()->widgets[i]->hasFocus()) { oldFocusIndex = i; break; }

    for (int i=0; i<currentPage()->numWidgets; i++) {
      newFocusIndex = (oldFocusIndex + 1 + i) % currentPage()->numWidgets;
      if (currentPage()->widgets[newFocusIndex]->canHaveFocus()) break;
    }

    // neu zeichnen, dabei wandert Fokusdarstellung auf nächstes Widget
    if (oldFocusIndex != -1) { currentPage()->widgets[oldFocusIndex]->setFocus(false); currentPage()->widgets[oldFocusIndex]->setVisible(true); }
    currentPage()->widgets[newFocusIndex]->setFocus(true);  currentPage()->widgets[newFocusIndex]->setVisible(true);

    if (oldFocusIndex != newFocusIndex) currentPage()->focusChanged();
 
  } 
}

// ----------------------------------------------------------------------------------------------------
//

Widget* Page::focussedWidget() {
  for (int i=0; i<numWidgets; i++) {
    if (widgets[i]->hasFocus()) return widgets[i];
  }
}

void buttonPressed(M5Btn::ButtonType btn) {
    Page::handlePageSwitchAndFocus(btn);
    Page::currentPage()->buttonPressed(btn);
}

// ----------------------------------------------------------------------------------------------------
//

void Page::trackPowerStateChanged(bool trackPowerOff) {
  navigationHint();
}

void Page::progModeStateChanged(bool progModeOff) {
  navigationHint();
};

