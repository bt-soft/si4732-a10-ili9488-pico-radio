// https://github.com/bimac/EEPstore/blob/main/src/EEPstore.h
// https://github.com/RobTillaart/CRC/blob/master/examples/CRC16_test/CRC16_test.ino

#ifndef __CONFIGMANAGER_H
#define __CONFIGMANAGER_H

#include <CRC.h>
#include <CRC16.h>
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
class ConfigManager {

private:
    const T data;       // Konfigurációs adatok
    const uint16_t crc; // CRC érték

    /**
     * CRC kiszámítása
     * @return CRC érték
     */
    uint16_t calcCRC() {

        CRC16 crc;

        const uint8_t *bytes = (uint8_t *)&data;
        for (size_t i = 0; i < sizeof(T); i++) {
            crc.add(bytes[i]);
        }

        return crc.calc();
    }

public:
    /**
     * Konstruktor
     * @tparam T a konfigurációs adatok típusa
     * @param dataRef a konfigurációs adatok referenciája
     * @note A konstruktorban kiszámolja a CRC-t
     *
     */
    ConfigManager(const T &dataRef) : data(dataRef), crc(calcCRC()) {
    }

    /**
     * Ha az adatok érvényesek, azokat beolvassa az EEPROM-ból
     * @tparam T a konfigurációs adatok típusa
     * @param dataRef a konfigurációs adatok referenciája
     * @param address az EEPROM címe
     * @return true, ha az adatok érvényesek
     * @note Ha az adatok érvényesek, akkor azokat beolvassa az EEPROM-ból
     *
     */
    static inline bool getIfValid(T &dataRef, const uint16_t address = 0) {

        // Kiolvassuk az adatokat az EEPROM-ból
        ConfigManager<T> storage(dataRef);
        EEPROM.get(address, storage);

        // Valid?
        bool valid = storage.crc == storage.calcCRC();
        // Ha valid, akkor beállítjuk a dataRef-et
        if (valid) {
            dataRef = storage.data;
        }

        return valid;
    }

    /**
     *  Beolvasás vagy beállítás
     * Ha nem érvényesek, beállítja őket az alapértelmezett értékekre
     * @tparam T a konfigurációs adatok típusa
     * @param dataRef a konfigurációs adatok referenciája
     * @param address az EEPROM címe
     * @return true, ha az adatok érvényesek
     * @note Ha az adatok nem érvényesek, akkor az alapértelmezett értékeket állítja be
     */
    static inline bool getOrSet(T &dataRef, const uint16_t address = 0) {

        // Kiolvassuk az adatokat az EEPROM-ból
        bool valid = getIfValid(dataRef, address);

        // Ha NEM valid, akkor lementjük az EEPROM-ba a dataRef (mint default) értékeit
        if (!valid) {
            set(dataRef, address);
        }

        return valid;
    }

    /**
     * Az EEPROM-ba való mentés
     * @tparam T a konfigurációs adatok típusa
     * @param address az EEPROM címe
     * @param dataRef a konfigurációs adatok referenciája
     * @note A konfigurációs adatokat csak akkor menti, ha változott
     *
     */
    static inline void set(const T &dataRef, const uint16_t address = 0) {
        ConfigManager<T> storage(dataRef);
        EEPROM.put(address, storage);
    }

    // /**
    //  * Konfigurációs adatok frissítése
    //  * Ha az adat változott, akkor menti az EEPROM-ba
    //  * @param address az EEPROM címe
    //  * @param newConfig az új konfigurációs adatok
    //  * @tparam T a konfigurációs adatok típusa
    //  * @note A konfigurációs adatokat csak akkor menti, ha változott
    //  * @note A konfigurációs adatokat csak akkor menti, ha az utolsó mentés óta eltelt 3 perc
    //  * @note A konfigurációs adatokat csak akkor menti, ha az utolsó mentés óta változott
    //  *
    //  */
    // template <typename T>
    // static void updateConfig(int address, const T &newConfig) {
    //    static unsigned long lastSaveTime;
    //    static bool hasChanges;
    //     static T cachedConfig;
    //     if (memcmp(&cachedConfig, &newConfig, sizeof(T)) != 0) {
    //         cachedConfig = newConfig;
    //         hasChanges = true;
    //     }
    //     // Ha az adat változott, és eltel 3 perc az utolsó mentés óta, akkor mentjük
    //     if (hasChanges && (millis() - lastSaveTime > 180000)) { // 3 perc eltelt
    //         saveConfig(address, cachedConfig);
    //         lastSaveTime = millis();
    //         hasChanges = false;
    //     }
    // }
};

// --------------------------------
struct MyConfig_t {
    int someValue = 42;
    float anotherValue = 3.14;
    char name[10] = "Default";
} MyConfig;

extern MyConfig_t myConfig;

#endif // __EEPROMMANAGER_H