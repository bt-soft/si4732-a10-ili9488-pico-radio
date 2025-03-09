#ifndef __PICOMEMORYINFO_H
#define __PICOMEMORYINFO_H

#include "utils.h"
#include <RP2040Support.h>

// A fordító  output-ból:
// Sketch uses 195272 bytes (9%) of program storage space. Maximum is 2093056 bytes.
// Global variables use 16748 bytes (6%) of dynamic memory, leaving 245396 bytes for local variables. Maximum is 262144 bytes.

// Fordító output alapján
#define FULL_FLASH_SIZE 2093056     // Teljes flash memória méret (2MB)
#define FULL_MEMORY_SIZE 262144     // Teljes RAM méret (heap) byte-ban
extern "C" char __flash_binary_end; // A program végét jelző cím

/**
 * Struktúra a memóriaállapot tárolására
 */
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
    DEBUG("Program mérete: %d B (%.2f kB) - %.2f%%\n", status.programSize, status.programSize / 1024.0, status.programPercent);
    DEBUG("Szabad flash memória: %d B (%.2f kB) - %.2f%%\n", status.freeFlash, status.freeFlash / 1024.0, status.freeFlashPercent);
    DEBUG("-- \n");
    // Heap memória (RAM)
    DEBUG("Teljes heap méret: %d B (%.2f kB)\n", status.heapSize, status.heapSize / 1024.0);
    DEBUG("Felhasznált heap: %d B (%.2f kB) - %.2f%%\n", status.usedHeap, status.usedHeap / 1024.0, status.usedHeapPercent);
    DEBUG("Szabad heap: %d B (%.2f kB) - %.2f%%\n", status.freeHeap, status.freeHeap / 1024.0, status.freeHeapPercent);
    DEBUG("===========================\n");
}
#endif

#endif //__MEMORYINFO_H