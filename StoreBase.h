#ifndef __STOREBASE_H
#define __STOREBASE_H
#include "EepromManager.h"
#include <Arduino.h>
#include <Ticker.h>
#include <list>

#define EEPROM_SAVE_CHECK_INTERVAL_SECONDS 60 * 5 // 5 perc

/**
 * Generikus ősosztály a mentés és betöltés funkciókhoz
 */
template <typename T>
class StoreBase {

private:
    static Ticker crcChecker;
    static std::list<StoreBase<T> *> instances; // 🔥 Az összes példány listája

    uint16_t lastCRC = 0;
    T *pData = nullptr;

    /**
     * CRC ellenőrzés és mentés indítása ha szükséges
     */
    static void checkSave() {
        DEBUG("crcChecker check start\n");

        for (StoreBase<T> *instance : instances) { // 🔥 Végigmegyünk az összes példányon
            uint16_t crc = calcCRC16((uint8_t *)instance->pData, sizeof(T));
            if (instance->lastCRC != crc) {
                DEBUG("EEPROM save start\n");
                instance->lastCRC = crc;

                digitalWrite(LED_PIN, HIGH);
                EepromManager<T>::save(*instance->pData); // 🔥 Javított rész (dereferálás)
                digitalWrite(LED_PIN, LOW);

                DEBUG("EEPROM save end\n");
            } else {
                DEBUG("CRC OK\n");
            }
        }

        DEBUG("crcChecker check end\n");
    }

public:
    /**
     * Konstruktor
     * @param pData Pointer a konfigurációs adatokhoz
     */
    StoreBase(T *pData) : pData(pData) {

        instances.push_back(this); // 🔥 Az aktuális példányt hozzáadjuk a listához

        if (!crcChecker.active()) {
            crcChecker.attach(EEPROM_SAVE_CHECK_INTERVAL_SECONDS, []() { checkSave(); });
        }
    }

    /**
     * Destruktor – törli a példányt a listából
     */
    virtual ~StoreBase() {
        instances.remove(this);
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

// 🔥 Statikus tag inicializálása 🔥
template <typename T>
Ticker StoreBase<T>::crcChecker;

template <typename T>
std::list<StoreBase<T> *> StoreBase<T>::instances;

#endif //__STOREBASE_H
