#include "utils.h"

/**
 * Biztonságos string másolás
 * @param dest cél string
 * @param src forrás string
 */
void safeStrCpy(char *dest, char *src) {
    do {
        strncpy(dest, src, sizeof(dest) - 1);
        dest[sizeof(dest) - 1] = '\0';
    } while (0);
}

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
