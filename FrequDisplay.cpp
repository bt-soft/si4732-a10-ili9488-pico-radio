#include "FrequDisplay.h"
#include "DSEG7_Classic_Mini_Regular_34.h"
#include "RuntimeVars.h"

#define TFT_COLOR_INACTIVE_SEGMENT TFT_COLOR(50, 50, 50) // Nem aktív szegmens színe

#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_INDICATOR_FREQ TFT_GOLD
/**
 *
 */
void FreqDisplay::Segment(String freq, String mask, int d) {

    if (!config.data.digitLigth) {
        mask = "";
    }

    if (SEEK) {
        spr.createSprite(194, 38);
        d = 46;
    } else {

        if (bfoOn) {
            spr.createSprite(110, 38);
        } else {
            spr.createSprite(240, 38);
        }
    }
    spr.fillScreen(COLOR_BACKGROUND);
    spr.setTextSize(1);
    spr.setTextPadding(0);
    spr.setFreeFont(&DSEG7_Classic_Mini_Regular_34);
    spr.setTextDatum(BR_DATUM);
    int x = 222;
    if (bfoOn) {
        x = 110;
        spr.setTextColor(TFT_BROWN);
        spr.drawString(mask, x, 38);
        spr.setTextColor(TFT_ORANGE);
        spr.drawString(freq, x, 38);

    } else {

        if ((band.currentMode == AM || band.currentMode == FM)) {
            x = 190;
        }

        if (SEEK) {
            x = 144;
        }

        if (bfoOn) {
            spr.setTextColor(TFT_BROWN);
        } else {
            spr.setTextColor(TFT_COLOR_INACTIVE_SEGMENT); // Nem aktív szegmens színe
        }

        spr.drawString(mask, x, 38);

        if (bfoOn) {
            spr.setTextColor(TFT_ORANGE);
        } else {
            spr.setTextColor(COLOR_INDICATOR_FREQ);
        }

        spr.drawString(freq, x, 38);
    }

    spr.pushSprite(freqDispX + d, freqDispY + 20);
    spr.setFreeFont();
    spr.deleteSprite();
}

/**
 *
 */
void FreqDisplay::FreqDraw(float freq, int d) {

    String unitStr = "MHz";
    float displayFreq = 0;

    // ELőző érték törlése
    tft.fillRect(freqDispX + 46 + d, freqDispY + 20, 194, 40, TFT_BLACK);

    if (band.currentMode == FM) {
        displayFreq = freq / 100;
        Segment(String(displayFreq, 2), "188.88", d - 10);

    } else {
        if (band.getBandTable(config.data.bandIdx).bandType == MW_BAND_TYPE or band.getBandTable(config.data.bandIdx).bandType == LW_BAND_TYPE) {
            displayFreq = freq;
            Segment(String(displayFreq, 0), "1888", d);
            unitStr = "kHz";

        } else { // SW
            displayFreq = freq / 1000;
            Segment(String(displayFreq, 3), "88.888", d);
        }
    }

    // Mértékegység kirajzolása
    tft.setTextDatum(BC_DATUM);
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(unitStr, freqDispX + 215 + d, freqDispY + 60);
}
