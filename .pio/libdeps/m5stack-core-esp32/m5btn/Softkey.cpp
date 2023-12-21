#include "Softkey.h"

// ----------------------------------------------------------------------------------------------------
//

Softkey::Softkey(TFT_eSPI* tft, int style, String caption, M5Btn::ButtonType hwButton, int layer, int fgColor, int bgColor, int scrBgColor) : 
    tft{tft}, style{style}, caption{caption}, hwButton{hwButton}, layer{layer}, fgColor{fgColor}, bgColor{bgColor}, scrBgColor{scrBgColor} {

      switch(hwButton) {
        case M5Btn::A:
          ymid = yBtn1;
          xmid = xA;
          break;
        case M5Btn::B:
          ymid = yBtn1;
          xmid = xB;
          break;
        case M5Btn::C:
          ymid = yBtn1;
          xmid = xC;
          break;
        case M5Btn::AA:
          ymid = yBtn2;
          xmid = xA;
          break;
        case M5Btn::BB:
          ymid = yBtn2;
          xmid = xB;
          break;
        case M5Btn::CC:
          ymid = yBtn2;
          xmid = xC;
          break;
        default: ; // um Warnungen zu vermeiden
      }
}

// ----------------------------------------------------------------------------------------------------
//

#define TRI_WIDTH (BTN_HEIGHT-2)
#define TRI_HEIGHT (BTN_HEIGHT-2)

Softkey* Softkey::setVisible(bool visible) {

  if (visible) {

    // Pfeil nach oben
    if (caption == CAPTION_UP) {
      tft->fillTriangle(xmid-TRI_WIDTH/2, ymid+TRI_HEIGHT/2, xmid+TRI_WIDTH/2, ymid+TRI_HEIGHT/2, xmid, ymid-TRI_HEIGHT/2, bgColor);

    // Pfeil nach unten
    } else if (caption == CAPTION_DOWN) {
      tft->fillTriangle(xmid-TRI_WIDTH/2, ymid-TRI_HEIGHT/2, xmid+TRI_WIDTH/2, ymid-TRI_HEIGHT/2, xmid, ymid+TRI_HEIGHT/2, bgColor);

    // normaler Textbutton
    } else {
      tft->fillRoundRect(xmid - BTN_WIDTH/2, ymid-BTN_HEIGHT/2, BTN_WIDTH, BTN_HEIGHT, 4, bgColor);
      int oldTextDatum = tft->getTextDatum();
      tft->setTextColor(fgColor, bgColor);
      tft->setTextDatum(CC_DATUM);
      tft->drawString(caption, xmid, ymid, captionTextSize);
      tft->setTextDatum(oldTextDatum);
      if (activated) tft->drawRoundRect(xmid - BTN_WIDTH/2, ymid-BTN_HEIGHT/2, BTN_WIDTH, BTN_HEIGHT, 4, fgColor);
    }

  // not visible
  } else {
    tft->fillRect(xmid - BTN_WIDTH/2, ymid-BTN_HEIGHT/2, BTN_WIDTH, BTN_HEIGHT, screenBgColor);
  }
  return this;

}


