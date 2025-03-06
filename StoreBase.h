#ifndef __STOREBASE_H
#define __STOREBASE_H
#include "EepromManager.h"
#include <Arduino.h>
#include <Ticker.h>
#include <list>

/**
 * Generikus ősosztály a mentés és betöltés funkciókhoz
 */
template <typename T>
class StoreBase {

private:
    // Pointer a tárolt adatoknak
    T *pData = nullptr;

    // A tárolt adatok CRC32 ellenőrző összege
    uint16_t lastCRC = 0;

public:
    /**
     * Konstruktor
     * @param pData Pointer a tárolandó adatokhoz
     */
    StoreBase(T *pData) : pData(pData) {
    }

    /**
     * Tárolt adatok mentése
     */
    virtual void save() {
        EepromManager<T>::save(*pData);
    }

    /**
     * Tárolt adatok betöltése
     */
    virtual void load() {
        lastCRC = EepromManager<T>::load(*pData);
    }

    /**
     * Alapértelmezett adatok betöltése
     * Muszáj implementálni a leszármazottban
     */
    virtual void loadDefaults() = 0;

    /**
     * CRC ellenőrzés és mentés indítása ha szükséges
     */
    virtual void checkSave() final {
        if (!pData) {
            DEBUG("pData is nullptr, aborting checkSave\n");
            return;
        }

        DEBUG("checkSave start\n");

        uint16_t crc = calcCRC16((uint8_t *)pData, sizeof(T));
        if (lastCRC != crc) {
            DEBUG("CRC diff, need to save\n");

            digitalWrite(LED_BUILTIN, HIGH);

            crc = EepromManager<T>::save(*pData); // dereferálás
            lastCRC = crc;

            digitalWrite(LED_BUILTIN, LOW);

            DEBUG("EEPROM save end, crc = %d\n", crc);

        } else {
            DEBUG("No need to save EEPROM\n");
        }

        DEBUG("checkSave end\n");
    }
};

#endif //__STOREBASE_H
