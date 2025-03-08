#ifndef __BAND_H
#define __BAND_H

#include "Config.h"
#include <SI4735.h>

// Band index
#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

// Modulation types
#define FM 0
#define LSB 1
#define USB 2
#define AM 3
#define CW 4

// Band data
typedef struct {
    const char *bandName; // Bandname
    uint8_t bandType;     // Band type (FM, MW or SW)
    uint16_t prefmod;     // Pref. modulation
    uint16_t minimumFreq; // Minimum frequency of the band
    uint16_t maximumFreq; // maximum frequency of the band
    uint16_t currentFreq; // Default frequency or current frequency
    uint8_t currentStep;  // Default step (increment and decrement)
    int lastBFO;          // Last BFO per band
    int lastmanuBFO;      // Last Manual BFO per band using X-Tal
} BandTable_t;

/**
 * Band class
 */
class Band {
private:
    // Si4735 referencia
    SI4735 &si4735;
    Config &config;

    // SSB betöltve?
    bool ssbLoaded = false;

    void checkAGC();
    void setBandWidth();
    void loadSSB();
    void useBand();

public:
    uint8_t currentMode; // aktuális mód/modulációs típus (FM, AM, LSB, USB, CW)

    Band(SI4735 &si4735, Config &config) : si4735(si4735), config(config) {}
    virtual ~Band() = default;

    void BandInit();
    void BandSet();

    /**
     * A Band egy rekordjának elkérése az index alapján
     */
    BandTable_t &getBandByIdx(uint8_t bandIdx) { return bandTable[bandIdx]; }
};

#endif