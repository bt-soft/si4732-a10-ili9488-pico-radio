#ifndef __PICOMEMORYINFO_H
#define __PICOMEMORYINFO_H

#include "utils.h"
#include <RP2040Support.h>

#define FULL_FLASH_SIZE 2093056     // Teljes flash memória méret (2MB)
#define FULL_MEMORY_SIZE 262144     // Teljes RAM méret (heap) byte-ban
extern "C" char __flash_binary_end; // A program végét jelző cím

// Struktúra a memóriaállapot tárolására
struct MemoryStatus_t {
    // Flash memória (programtár)
    uint32_t programSize;
    float programPercent;
    uint32_t freeFlash;
    float freeFlashPercent;

    // Heap memória (RAM)
    uint32_t heapSize;
    uint32_t usedHeap;
    float usedHeapPercent;
    uint32_t freeHeap;
    float freeHeapPercent;
};

#ifdef __DEBUG
#define MEASUREMENTS_COUNT 20 // A memóriahasználat mérésének száma

/**
 * A memóriahasználat mérésének tárolása
 */
struct HeapMemoryMonitor {
    uint32_t measurements[MEASUREMENTS_COUNT]; // A memóriahasználatot tároló tömb
    uint8_t index;                             // Az aktuális mérési index

    // A mérések száma (csak érvényes méréseket számol)
    uint8_t validMeasurements;

    HeapMemoryMonitor() : index(0), validMeasurements(0) {
        // Kezdeti értékek beállítása 0-ra
        for (uint8_t i = 0; i < MEASUREMENTS_COUNT; i++) {
            measurements[i] = 0;
        }
    }

    // Adott heap használat mérésének hozzáadása
    void addMeasurement(uint32_t usedHeap) {
        if (usedHeap > 0) {
            measurements[index] = usedHeap;
            validMeasurements = (validMeasurements < MEASUREMENTS_COUNT) ? validMeasurements + 1 : MEASUREMENTS_COUNT; // Valid mérések száma
        } else {
            measurements[index] = 0; // Ha a mérés nulla, akkor ne számoljuk
        }

        // Ha az index elérte a maximális értéket, az első elem is eldobásra kerül
        index = (index + 1) % MEASUREMENTS_COUNT; // Körkörös indexelés
    }

    // Kiszámítja az átlagos memóriahasználatot
    float getAverageUsedHeap() {
        uint32_t total = 0;
        uint8_t validCount = 0; // Az érvényes mérések száma
        for (uint8_t i = 0; i < MEASUREMENTS_COUNT; i++) {
            if (measurements[i] > 0) { // Csak a nem nulla mérések
                total += measurements[i];
                validCount++;
            }
        }
        return validCount > 0 ? total / (float)validCount : 0;
    }

    // Kiszámítja a legnagyobb növekedést
    float getMaxIncrease() {
        float maxIncrease = 0.0;

        // Legalább 2 valid mérés szükséges
        if (validMeasurements > 1) {
            for (uint8_t i = 1; i < MEASUREMENTS_COUNT; i++) {
                // Csak akkor számoljuk, ha az előző mérés valid
                if (measurements[i - 1] > 0 && measurements[i] > 0) {
                    float increase = measurements[i] - measurements[i - 1];
                    if (increase > maxIncrease) {
                        maxIncrease = increase;
                    }
                }
            }
        }

        return maxIncrease;
    }
};

// Globális memóriafigyelő objektum, csak DEBUG módban
HeapMemoryMonitor heapMemoryMonitor;
#endif

/**
 * Memóriaállapot lekérdezése
 */
MemoryStatus_t getMemoryStatus() {
    MemoryStatus_t status;

    // Flash memória méretének meghatározása
    status.programSize = (uint32_t)&__flash_binary_end - 0x10000000;
    status.programPercent = (status.programSize * 100.0) / FULL_FLASH_SIZE;
    status.freeFlash = FULL_FLASH_SIZE - status.programSize;
    status.freeFlashPercent = 100.0 - status.programPercent;

    // Heap memória (RAM)
    RP2040 rp2040;
    status.heapSize = rp2040.getTotalHeap();
    status.usedHeap = rp2040.getUsedHeap();
    status.freeHeap = rp2040.getFreeHeap();

    // Százalékszámítás a heap teljes méretére vonatkozóan
    status.usedHeapPercent = (status.usedHeap * 100.0) / status.heapSize;
    status.freeHeapPercent = (status.freeHeap * 100.0) / status.heapSize;

#ifdef __DEBUG
    // Mérési adat hozzáadása
    heapMemoryMonitor.addMeasurement(status.usedHeap);
#endif

    return status;
}

/**
 * Debug módban az adatok kiírása
 */
#ifdef __DEBUG
void debugMemoryInfo() {
    MemoryStatus_t status = getMemoryStatus(); // Adatok lekérése

    DEBUG("===== Memory info =====\n");

    // Program memória (flash)
    DEBUG("Flash\t\t\t\t\t\tHeap\n");
    DEBUG("Total: %d B (%.2f kB)\t\t\t%d B (%.2f kB)\n",
          FULL_FLASH_SIZE, FULL_FLASH_SIZE / 1024.0, // Flash
          status.heapSize, status.heapSize / 1024.0  // Heap
    );
    DEBUG("Used: %d B (%.2f kB) - %.2f%%\t\t%d B (%.2f kB) - %.2f%%\n",
          status.programSize, status.programSize / 1024.0, status.programPercent, // Flash
          status.usedHeap, status.usedHeap / 1024.0, status.usedHeapPercent       // Heap
    );
    DEBUG("Free: %d B (%.2f kB) - %.2f%%\t\t%d B (%.2f kB) - %.2f%%\n",
          status.freeFlash, status.freeFlash / 1024.0, status.freeFlashPercent, // Flash
          status.freeHeap, status.freeHeap / 1024.0, status.freeHeapPercent     // Heap
    );

    // Memória szivárgás ellenőrzése
    float averageUsedHeap = heapMemoryMonitor.getAverageUsedHeap();
    float maxIncrease = heapMemoryMonitor.getMaxIncrease();

    // Használt mérések számának lekérése
    uint8_t usedMeasurements = 0;
    for (uint8_t i = 0; i < MEASUREMENTS_COUNT; i++) {
        if (heapMemoryMonitor.measurements[i] > 0) {
            usedMeasurements++;
        }
    }

    DEBUG("Heap usage:\n  Ave: %.2f kB, max grow: %.2f kB (%d/%d)\n",
          averageUsedHeap / 1024.0,                                  // average
          maxIncrease / 1024.0, usedMeasurements, MEASUREMENTS_COUNT // max grow
    );

    if (maxIncrease > 1024) { // Ha a növekedés nagyobb mint 1 kB, akkor figyelmeztetünk
        DEBUG("Warning! Possible memory leak!\n");
    }

    DEBUG("===========================\n");
}
#endif

#endif //__PICOMEMORYINFO_H
