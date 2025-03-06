#ifndef __CONFIG_H
#define __CONFIG_H

#include "StoreBase.h"

// --------------------------------
// Konfig struktúra típusdefiníció
struct Config_t {
    uint16_t tftCalibrateData[5]; // TFT touch kalibrációs adatok
    int someValue;
    float anotherValue;
    char name[10];
};

// Alapértelmezett konfigurációs adatok (readonly, const)
extern const Config_t DEFAULT_CONFIG;

/**
 * Konfigurációs adatok kezelése
 */
class Config : public StoreBase<Config_t> {

private:
    // A 'config' változó, amely az alapértelmezett értékeket veszi fel, kívülről nem módosítható
    Config_t config;

public:
    /**
     * Konstruktor
     * @param pData Pointer a konfigurációs adatokhoz
     */
    Config() : StoreBase<Config_t>(), config(DEFAULT_CONFIG) {}

    /**
     * Pointer az adattagra
     */
    Config_t *p() override {
        return &config;
    };

    /**
     * Alapértelmezett adatok betöltése
     */
    void loadDefaults() override {
        memcpy(&config, &DEFAULT_CONFIG, sizeof(Config_t));
        DEBUG("Default config loaded\n");
    }
};

#endif // __CONFIG_H