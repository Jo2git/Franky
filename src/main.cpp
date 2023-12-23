/*
  FRANKY - Drahtloser Handregler für X-Bus-basierte Zentralen (Z21, DR5000, XP Multi)
*/

#include <M5Stack.h>
#include <M5Btn.h>
#include <Pref.h>
#include "Loco.h"
#include "LocoPage.h"
#include <configuration.h>

// ----------------------------------------------------------------------------------------------------
// Webserver und Netzwerk

#include <WiFi.h>
#include "Webserver.h"
WiFiUDP Udp;

// ----------------------------------------------------------------------------------------------------
// GUI

#include "Page.h"

// ----------------------------------------------------------------------------------------------------
// Z21

#include <Z21.h>

// ----------------------------------------------------------------------------------------------------
//

void setup() {

  // Präferenzen
  Pref::begin();

  // M5Stack
  M5.begin(true, false, true, false);
  M5.Speaker.begin(); // ohne: Pfeifton
  Serial.begin(115200);
  Wire.begin(); // für Drehregler

  M5.Power.begin();
  if (!M5.Power.canControl()) {
    Serial.println("M5.Power.canControl() == false");
  }
  M5.Power.setPowerBoostSet(true); // Ein-/Aus nur mit einfachem kurzen Druck
  // M5.Power.setPowerBoostOnOff(bool); // zweimal drücken oder lang drücken, macht nur Sinn nach setPoweBoostSet(false)
  M5.Power.setPowerVin(false); // Abziehen von USB schaltet aus (true: power up again)
  //M5.Power.setAutoBootOnLoad(false); // Nicht beim Einstecken USB hochstarten, dann aber jedes zweite Einschalten erfolglos !?!?

  // Filesystem
  SPIFFS.begin();

  // Lokdaten lesen - vorher muss SPIFFS.begin() ausgeführt worden sein
  Loco::begin();

  // Webserver, und wenn nötig, Access Point für erstmalige Credentials-Eingabe

  // Kommentare entfernen, um für Testzwecke AP-Modus zu erzwingen:
  // Pref::set(prefNameSSID, "");
  // Pref::set(prefNamePasswd, "");
  Webserver::webconfig();
  while (!Webserver::wlConnected){};
  delay(1000);

  // Z21-Adresse bekanntgeben (kann über Webserver geändert werden)
  Z21::setIPAddress(Pref::get(prefNameZ21IPAddr, Z21_DEFAULT_ADDR));

  // Adressoffset, falls Gerds Gleisbox?
  Z21::setAddrOffs(Pref::get(prefNameGerdOffs, "off") == "on" ? 0x2000 : 0);

  // GUI initialisieren
  Page::begin(&M5.lcd);
  if (!Page::isBlocked()) Page::currentPage()->setVisible(true, true);

  Serial.printf("Heapauslastung zu Beginn: %d Byte\n", ESP.getFreeHeap());

}

// ----------------------------------------------------------------------------------------------------
//

bool UDPServerInitialised = false;
bool initState = true;
long lastHeartbeatSent = -Z21_HEARTBEAT + 100 ; // gleich anfangs Heartbeat erzwingen
long lastDriven = 0;
long lastCheck = -BAT_CHECK_CYCLE + 50; // vorbelegen, damit schon beim ersten Aufruf der Batteriestatus gelesen wird

void loop() {

  // Da die WLAN-Verbindung noch nicht unbedingt abgeschlossen ist,
  // kann der UDP-Server auch erst nach dem Verbindungsaufbau gestartet werden
  if (WiFi.status() == WL_CONNECTED) {

    if (!UDPServerInitialised) {
      Udp.begin(Z21_PORT);
      UDPServerInitialised = true;
      Z21::init();
      delay(100);
      Z21::LAN_X_GET_LOCO_INFO(LocoPage::currentLoco()->getAddr()); // beim Einschalten Status der aktiven Lok angefordern, da sie noch nicht abboniert ist!
    }

    if (initState && UDPServerInitialised) Z21::LAN_X_GET_STATUS(); initState = false; // einmalig Status einlesen
  }

  if (millis() - lastHeartbeatSent > Z21_HEARTBEAT) {
    Z21::heartbeat();
    lastHeartbeatSent = millis();
    if (WiFi.status() == WL_CONNECTED)  Z21::LAN_X_GET_LOCO_INFO(LocoPage::currentLoco()->getAddr());
    if (Webserver::wlConnected == true)  Page::drawStatusBar();
  }

  // Fahren
  if (millis() - lastDriven > LOCO_CYCLE) {
    Loco::drive();
    lastDriven = millis();
  }

  // Notifikationen erhalten
  Z21::receive();

  // Buttons und Drehregler einlesen
  M5Btn::loop();

  // Batteriestatus lesen
  if (millis() - lastCheck > BAT_CHECK_CYCLE) {
    lastCheck = millis();
    Page::batterie = M5.Power.getBatteryLevel();
  }
}