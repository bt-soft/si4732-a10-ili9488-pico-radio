#ifndef __STOREBASE_H
#define __STOREBASE_H
#include "EepromManager.h"
#include <Arduino.h>
#include <Ticker.h>

/**
 * Generikus ősosztály a mentés és betöltés funkciókhoz
 */
template <typename T>
class StoreBase {

private:
    // Ticker crcChecker;
    uint16_t lastCRC = 0;
    T *pData = nullptr;

public:
    /**
     * Konstruktor
     * @param pData Pointer a konfigurációs adatokhoz
     */
    StoreBase(T *pData) : pData(pData) {
    }

    /**
     * Konfigurációs adatok mentése
     */
    virtual void save() {
        EepromManager<T>::save(*pData);
    }

    /**
     * Konfigurációs adatok betöltése
     */
    virtual void load() {
        lastCRC = EepromManager<T>::load(*pData);
    }

    /**
     * Alapértelmezett adatok betöltése
     */
    virtual void loadDefaults() = 0;
};

#endif //__STOREBASE_H
