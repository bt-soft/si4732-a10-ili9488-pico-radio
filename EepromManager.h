#ifndef __EEPROMMANAGER_H
#define __EEPROMMANAGER_H

// https://github.com/bimac/EEPstore/blob/main/src/EEPstore.h

#include "utils.h"
#include <CRC.h>
#include <EEPROM.h>

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 2048 // Alapértelmezett érték, de lehet 512-4096 között módosítani
#endif

/**
 * EEPROM kezelő osztály
 * A konfigurációs adatokat az EEPROM-ba menti és onnan tölti be
 * A konfigurációs adatokat egy struktúrában tárolja
 * A struktúra mérete nem lehet nagyobb, mint az EEPROM mérete
 * A struktúra elemeinek típusa csak az EEPROM-ban tárolható típusok lehetnek
 * - uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float, double
 * - struktúra, amely csak az EEPROM-ban tárolható típusokat tartalmaz
 * - tömbök, amelyek csak az EEPROM-ban tárolható típusokat tartalmaznak
 * - A struktúra elemeinek neve nem lehet ugyanaz, mint a struktúra elemeinek neve
 */
template <class T>
class EepromManager {

private:
    const T data;       // Konfigurációs adatok
    const uint16_t crc; // CRC érték

public:
    /**
     * Konstruktor
     * @tparam T a konfigurációs adatok típusa
     * @param dataRef a konfigurációs adatok referenciája
     * @note A konstruktorban kiszámolja a CRC-t
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
            DEBUG("Az EEPROM-ból betöltött adatok érvényesek");
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

        EEPROM.begin(EEPROM_SIZE); // EEPROM inicializálása

        // Kiolvassuk az adatokat az EEPROM-ból
        uint16_t crc32 = getIfValid(dataRef, address);

        // Ha NEM valid (crc32 == 0), akkor lementjük az EEPROM-ba a dataRef (mint default) értékeit
        if (crc32 == 0) {
            DEBUG("EEPROM-ból betöltött adatok érvénytelenek, lementjük az alapértelmezett értékeket");
            save(dataRef, address);
        } else {
            DEBUG("EEPROM betöltés OK");
        }

        return crc32;
    }

    /**
     * Az EEPROM-ba való mentés
     * @tparam T a konfigurációs adatok típusa
     * @param address az EEPROM címe
     * @param dataRef a konfigurációs adatok referenciája
     * @note A konfigurációs adatokat csak akkor menti, ha változott
     *
     */
    inline static void save(const T &dataRef, const uint16_t address = 0) {

        // Létrehozunk egy saját példányt, közben a crc is számítódik
        EepromManager<T> storage(dataRef);

        // Lementjük az adatokat + a crc-t az EEPROM-ba
        EEPROM.put(address, storage);
        EEPROM.commit(); // ESP platformon szükséges!

        DEBUG("EEPROM mentés OK");
    }
};

#endif // __EEPROMMANAGER_H
