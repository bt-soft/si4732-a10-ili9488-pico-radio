#ifndef __STOREBASE_H
#define __STOREBASE_H
#include "EepromManager.h"
#include <Arduino.h>
#include <Ticker.h>
#include <list>

/**
 * Generikus wrapper ősosztály a mentés és betöltés + CRC számítás funkciókhoz
 */
template <typename T>
class StoreBase {

private:
    // A tárolt adatok CRC32 ellenőrző összege
    uint16_t lastCRC = 0;

public:
    /**
     * Pointer az adattagra
     */
    virtual T &p() = 0;

    /**
     * Tárolt adatok mentése
     */
    virtual void save() {
        EepromManager<T>::save(p());
    }

    /**
     * Tárolt adatok betöltése
     */
    virtual void load() {
        lastCRC = EepromManager<T>::load(p()); // dereferálás
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

        DEBUG("checkSave start\n");

        uint16_t crc = calcCRC16((uint8_t *)&p(), sizeof(T));
        if (lastCRC != crc) {
            DEBUG("CRC diff, need to save\n");

            digitalWrite(LED_BUILTIN, HIGH);

            crc = EepromManager<T>::save(p());
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
