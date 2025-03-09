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
struct MemoryMonitor {
    uint32_t measurements[MEASUREMENTS_COUNT]; // A memóriahasználatot tároló tömb
    uint8_t index;                             // Az aktuális mérési index

    MemoryMonitor() : index(0) {
        // Kezdeti értékek beállítása 0-ra
        for (uint8_t i = 0; i < MEASUREMENTS_COUNT; i++) {
            measurements[i] = 0;
        }
    }

    void addMeasurement(uint32_t usedHeap) {
        // Ha már van legalább egy mérés, az elsőt eldobjuk, majd hozzáadjuk az új mérést
        if (index > 0) {
            measurements[index] = usedHeap;
        }
        // Ha az index elérte a maximális értéket, az első elem is eldobásra kerül
        index = (index + 1) % MEASUREMENTS_COUNT; // Körkörös indexelés
    }

    // Kiszámítja az átlagos memóriahasználatot
    float getAverageUsedHeap() {
        uint32_t total = 0;
        uint8_t count = 0;

        // Az első mérést kihagyjuk
        for (uint8_t i = 1; i < MEASUREMENTS_COUNT; i++) {
            total += measurements[i];
            count++;
        }

        return count > 0 ? total / (float)count : 0;
    }

    // Kiszámítja a legnagyobb növekedést (lehet memóriaszivárgás indikátor)
    float getMaxIncrease() {
        float maxIncrease = 0.0;
        for (uint8_t i = 1; i < MEASUREMENTS_COUNT; i++) {
            float increase = measurements[i] - measurements[i - 1];
            if (increase > maxIncrease) {
                maxIncrease = increase;
            }
        }
        return maxIncrease;
    }
};

// Globális memóriafigyelő objektum, csak DEBUG módban
MemoryMonitor memoryMonitor;
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
    memoryMonitor.addMeasurement(status.usedHeap);
#endif

    return status;
}

/**
 * Debug módban az adatok kiírása
 */
#ifdef __DEBUG
void debugMemoryInfo() {
    MemoryStatus_t status = getMemoryStatus(); // Adatok lekérése

    DEBUG("===== Memória állapot =====\n");

    // Program memória (flash)
    DEBUG("Flash\n");
    DEBUG("Total: %d B (%.2f kB)\n", FULL_FLASH_SIZE, FULL_FLASH_SIZE / 1024.0);
    DEBUG("Used: %d B (%.2f kB) - %.2f%%\n", status.programSize, status.programSize / 1024.0, status.programPercent);
    DEBUG("Free: %d B (%.2f kB) - %.2f%%\n", status.freeFlash, status.freeFlash / 1024.0, status.freeFlashPercent);

    DEBUG("-- \n");

    // Heap memória (RAM)
    DEBUG("Heap\n");
    DEBUG("Total: %d B (%.2f kB)\n", status.heapSize, status.heapSize / 1024.0);
    DEBUG("Used: %d B (%.2f kB) - %.2f%%\n", status.usedHeap, status.usedHeap / 1024.0, status.usedHeapPercent);
    DEBUG("Free: %d B (%.2f kB) - %.2f%%\n", status.freeHeap, status.freeHeap / 1024.0, status.freeHeapPercent);

    // Memória szivárgás ellenőrzése
    float averageUsedHeap = memoryMonitor.getAverageUsedHeap();
    float maxIncrease = memoryMonitor.getMaxIncrease();

    // Használt mérések számának lekérése
    uint8_t usedMeasurements = 0;
    for (uint8_t i = 0; i < MEASUREMENTS_COUNT; i++) {
        if (memoryMonitor.measurements[i] > 0) {
            usedMeasurements++;
        }
    }

    DEBUG("-- \n");
    DEBUG("Átlagos memóriahasználat: %.2f kB\n", averageUsedHeap / 1024.0);
    DEBUG("Legnagyobb növekedés az utolsó 20 mérés alapján: %.2f kB\n", maxIncrease / 1024.0);
    DEBUG("Legnagyobb növekedés az utolsó %d mérés (a max %d-ból) alapján: %.2f kB\n", usedMeasurements, MEASUREMENTS_COUNT, maxIncrease / 1024.0);

    if (maxIncrease > 1024) { // Ha a növekedés nagyobb mint 1 kB, akkor figyelmeztetünk
        DEBUG("Figyelem! Lehetséges memóriaszivárgás!\n");
    }

    DEBUG("===========================\n");
}
#endif

#endif //__PICOMEMORYINFO_H
