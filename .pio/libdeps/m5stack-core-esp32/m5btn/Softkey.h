#ifndef SOFTKEY_H
#define SOFTKEY_H

#include <M5Stack.h>
#include <M5Btn.h>

#define captionTextSize 2

class Softkey {

  // Sonderbedeutung: in nächste (up) oder vorhergehende (down) Ebene
  #define CAPTION_UP "up"
  #define CAPTION_DOWN "down"

  // x-Achsen der drei Hardwarebuttons A, B, C
  #define xA TFT_W/4-12
  #define xB TFT_W/2
  #define xC TFT_W/4*3+12

  // Höhe jeder Buttonbeschriftungen
  #define BTN_HEIGHT 20
  #define BTN_WIDTH TFT_W/4

  // Textgröße Buttonbeschriftungen
  #define captionTextSize 2

  // Abstand der Buttonbeschriftungen untereinander horizontal (x) und vertikal (y)
  #define BTN_DIST_Y 2

  // y-Line der ersten (yBtn1, Kurzclick) und zweiten (yBtn2, Langclick) Buttonbar
  #define yBtn1 TFT_H-1-BTN_DIST_Y-BTN_HEIGHT/2
  #define yBtn2 yBtn1-BTN_HEIGHT-BTN_DIST_Y
  
  public:
    
    Softkey(TFT_eSPI* tft, int style, String caption, M5Btn::ButtonType hwButton, int layer, int fgColor, int bgColor, int scrBgColor);

    Softkey* setVisible(bool visible);
    Softkey* setActivated(bool activated) { this->activated = activated; return this; }
    bool isActivated() { return activated; }

    String getCaption() { return caption; }
    Softkey* setCaption(String caption) { this->caption = caption; return this; }
    M5Btn::ButtonType getButton() { return hwButton; }
    int getLayer() { return layer; }

  private:
    TFT_eSPI* tft;
    int style=0;
    String caption;
    M5Btn::ButtonType hwButton;
    int layer;
    int xmid, ymid;
    int fgColor, bgColor, scrBgColor;
    bool activated = false;

};

#endif