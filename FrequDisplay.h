#ifndef __FREQDISPLAY_H
#define __FREQDISPLAY_H

#include "Band.h"
#include "Config.h"
#include <TFT_eSPI.h>

class FreqDisplay {

private:
    TFT_eSPI &tft;
    TFT_eSprite spr;
    Band &band;
    Config &config;
    uint16_t freqDispX, freqDispY;

    void Segment(String freq, String mask, int d);

public:
    FreqDisplay(TFT_eSPI &tft, Band &band, Config &config, uint16_t freqDispX, uint16_t freqDispY)
        : tft(tft), band(band), config(config), freqDispX(freqDispX), freqDispY(freqDispY), spr(&tft) {
    }

    void FreqDraw(float freq, int d);
};

#endif