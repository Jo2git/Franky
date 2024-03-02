English readers: Franky is a WiFi throttle based on M5Stack Faces platform. It controls X-Bus compatible digital command-control stations (Z21, DR5000, XP Multi)

# Franky

Siehe https://sites.google.com/view/frankydcc/startseite/franky-2

wesentliche Änderungen zum Original:

- nur noch fahren möglich
- Batterieanzeige im Statusbalken
- Gleispannung EIN/AUS mit Drehknopf
   - Doppeldruck kurz-kurz => Not-Halt
   - Doppeldruck kurz-lang => Gleisspannung AUS
   - Einfachdruck kurz => Gleisspannung EIN / Not-Halt auflösen
- Fahrstufenschalter beim manuell Fahren (ähnlich Rocrailfahrpult) durch Drehknopf Langdruck
- Geschwindigkeitsanzeige für Loks, deren vmax in der aufgebohrten locoData.csv eingetragen ist (nur sinnvoll bei linearer Fahrstufenkennlinie)
- WLAN-Konfigurationsseite auch für alternatives WLAN

09.02.2024
- TFT abdunkeln nach 5min ohne Bedienung
- Kurzschlußanzeige im Statusbalken (Magenta)
- Externe Taster anschließbar
   - PIN19
       - kurz => Not-Halt, bzw. Gleisspannung EIN / Not-Halt auflösen
       - lang => Gleisspannung AUS
	- PIN5, 2, 17
       - Funktionstasten in locoData.csv einstellbar
	- PIN16
       - kurz => Lokkanal wechseln nach rechts
       - lang => Lokkanal wechseln nach links
	- PIN26
       - kurz => Funktionstasten blättern vor
       - lang => Funktionstasten blättern zurück

02.03.2024
- zum alternativen WLAN gibt es jetzt auch eine alternative Z21-Adresse