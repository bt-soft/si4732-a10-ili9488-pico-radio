#include "utils.h"

/**
 * Várakozás a soros port megnyitására
 * @param pTft a TFT kijelző példánya
 * @param beeper a Beeper példánya
 */
void debugWaitForSerial(TFT_eSPI *pTft, Beeper *beeper) {
#ifdef __DEBUG
    beeper->error();
    pTft->setTextColor(TFT_WHITE);
    pTft->drawString("Nyisd meg a soros portot!", 0, 0);
    while (!Serial) {
    }
    pTft->fillScreen(TFT_BLACK);
    beeper->tick();
#endif
}

/**
 * TFT érintőképernyő kalibráció
 * @param pTft TFT kijelző példánya
 * @param calData kalibrációs adatok
 */
void tftTouchCalibrate(TFT_eSPI *pTft, uint16_t (&calData)[5]) {

    const __FlashStringHelper *txt = F("Erintsd meg a sarkokat a jelzett helyeken!\n");
    pTft->fillScreen(TFT_BLACK);
    pTft->setCursor((pTft->width() - pTft->textWidth(txt)) / 2, pTft->height() / 2 - 40);
    pTft->setTextFont(2);
    pTft->setTextSize(1);
    pTft->setTextColor(TFT_WHITE, TFT_BLACK);
    pTft->println(txt);

    // TFT_eSPI kalibráció indítása
    pTft->calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    txt = F("Kalibracio befejezodott!");
    pTft->fillScreen(TFT_BLACK);
    pTft->setCursor((pTft->width() - pTft->textWidth(txt)) / 2, pTft->height() / 2);
    pTft->setTextColor(TFT_GREEN, TFT_BLACK);
    pTft->setTextSize(1);
    pTft->println(txt);

    DEBUG("// Használd ezt a kalibrációs kódot a setup()-ban:\n");
    DEBUG("  uint16_t calData[5] = { ");
    for (uint8_t i = 0; i < 5; i++) {
        DEBUG("%d", calData[i]);
        if (i < 4) {
            DEBUG(", ");
        }
    }
    DEBUG(" };\n");
    DEBUG("  pTft->setTouch(calData);\n");
}
