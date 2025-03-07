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
        DEBUG("Default config loaded\n");
    }
};

#endif // __CONFIG_H