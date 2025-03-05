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
// Konfigurációs változó deklarációja
extern Config_t *pConfig;

/**
 * Konfigurációs adatok kezelése
 */
class ConfigStore : public StoreBase<Config_t> {

public:
    /**
     * Konstruktor
     * @param pData Pointer a konfigurációs adatokhoz
     */
    ConfigStore() : StoreBase<Config_t>(pConfig) {}
};

#endif // __CONFIG_H