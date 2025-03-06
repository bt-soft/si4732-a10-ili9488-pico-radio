#ifndef __EEPROMMANAGER_H
#define __EEPROMMANAGER_H

// inspiration from https://github.com/bimac/EEPstore/blob/main/src/EEPstore.h

#include "utils.h"
#include <CRC.h>
#include <EEPROM.h>

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 2048 // Alapértelmezett 2K érték, de lehet 512-4096 között módosítani
#endif

/**
 * EEPROM kezelő osztály
 * A konfigurációs adatokat az EEPROM-ba menti és onnan tölti be
 */
template <class T>
class EepromManager {

private:
    const T data;       // Konfigurációs adatok
    const uint16_t crc; // Az adatok CRC ellenőrző összege

public:
    /**
     * Konstruktor
     * @tparam T a konfigurációs adatok típusa
     * @param dataRef a konfigurációs adatok referenciája
     * @note A konstruktorban kiszámoljuk a CRC-t is
     *
     */
    EepromManager(const T &dataRef) : data(dataRef), crc(calcCRC16((uint8_t *)&data, sizeof(T))) {
    }

    /**
     * Ha az adatok érvényesek, azokat beolvassa az EEPROM-ból
     * @tparam T a konfigurációs adatok típusa
     * @param dataRef a konfigurációs adatok referenciája
     * @param address az EEPROM címe
     * @return adatok CRC16 ellenőrző összege
     */
    inline static uint16_t getIfValid(T &dataRef, const uint16_t address = 0) {

        // Lokális példány létrehozása, közben a crc is számítódik
        EepromManager<T> storage(dataRef);

        // Kiolvassuk az EEPROM-ból a lokális példányba, közben crc is számítódik
        EEPROM.get(address, storage);

        // Azonosa a crc?
        bool valid = storage.crc == calcCRC16((uint8_t *)&storage.data, sizeof(T));

        // Ha valid, akkor beállítjuk a dataRef-et
        if (valid) {
            DEBUG("Az EEPROM load ok\n");
            dataRef = storage.data;
        }

        return valid ? storage.crc : 0;
    }

    /**
     * Beolvasás vagy beállítás
     * Ha nem érvényesek, beállítja őket az alapértelmezett értékekre
     * @tparam T a konfigurációs adatok típusa
     * @param dataRef a konfigurációs adatok referenciája
     * @param address az EEPROM címe
     * @return adatok CRC16 ellenőrző összege
     */
    inline static uint16_t load(T &dataRef, const uint16_t address = 0) {

// Az RP2040 platformon az EEPROM.begin() nem szükséges
#ifndef ARDUINO_ARCH_RP2040
        EEPROM.begin(EEPROM_SIZE); // EEPROM inicializálása
#endif

        // Kiolvassuk az adatokat az EEPROM-ból
        uint16_t crc32 = getIfValid(dataRef, address);

        // Ha NEM valid (crc32 == 0), akkor lementjük az EEPROM-ba a dataRef (mint default) értékeit
        if (crc32 == 0) {
            DEBUG("EEPROM data invalid, save defaults!\n");
            save(dataRef, address);
        } else {
            DEBUG("EEPROM load OK\n");
        }

        return crc32;
    }

    /**
     * Az EEPROM-ba való mentés
     * @tparam T a konfigurációs adatok típusa
     * @param address az EEPROM címe
     * @param dataRef a konfigurációs adatok referenciája
     * @return adatok CRC16 ellenőrző összege
     */
    inline static uint16_t save(const T &dataRef, const uint16_t address = 0) {

        // Létrehozunk egy saját példányt, közben a crc is számítódik
        EepromManager<T> storage(dataRef);

        // Lementjük az adatokat + a crc-t az EEPROM-ba
        EEPROM.put(address, storage);

// Az RP2040 platformon az EEPROM.commit() nem szükséges
#ifndef ARDUINO_ARCH_RP2040
        EEPROM.commit(); // ESP platformon szükséges!
#endif
        DEBUG("EEPROM save OK\n");

        return storage.crc;
    }
};

#endif // __EEPROMMANAGER_H
