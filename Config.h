#ifndef __CONFIG_H
#define __CONFIG_H

#include "StoreBase.h"
#include "pinout.h"

// --------------------------------
// Konfig struktúra típusdefiníció
struct Config_t {
    //-- Band
    uint8_t bandIdx;
    uint16_t currentFreq;
    uint8_t currentStep;

    // BandWidht
    uint8_t bwIdxAM;
    uint8_t bwIdxFM;
    uint8_t bwIdxMW;
    uint8_t bwIdxSSB;

    // Step
    uint8_t ssIdxMW;
    uint8_t ssIdxAM;
    uint8_t ssIdxFM;

    // BFO
    int currentBFO;
    uint8_t currentBFOStep;
    int currentBFOmanu;

    // Squelch
    uint8_t currentSquelch;
    bool squelchUsesRSSI; // A squlech RSSI alapú legyen?

    // Hangerő
    uint8_t currentVOL;

    // AGC
    uint8_t AGCgain;

    //--- TFT
    uint16_t tftCalibrateData[5]; // TFT touch kalibrációs adatok
    bool digitLigth;              // Inaktív szegmens látszódjon?
};

// Alapértelmezett konfigurációs adatok (readonly, const)
extern const Config_t DEFAULT_CONFIG;

/**
 * Konfigurációs adatok kezelése
 */
class Config : public StoreBase<Config_t> {

public:
    // A 'config' változó, alapértelmezett értékeket veszi fel a konstruktorban
    // Szándékosan public, nem kell a sok getter egy embedded rendszerben
    Config_t data;

protected:
    /**
     * Referencia az adattagra, csak az ős használja
     */
    Config_t &r() override {
        return data;
    };

public:
    /**
     * Konstruktor
     * @param pData Pointer a konfigurációs adatokhoz
     */
    Config() : StoreBase<Config_t>(), data(DEFAULT_CONFIG) {}

    /**
     * Alapértelmezett adatok betöltése
     */
    void loadDefaults() override {
        memcpy(&data, &DEFAULT_CONFIG, sizeof(Config_t));
    }
};

#endif // __CONFIG_H