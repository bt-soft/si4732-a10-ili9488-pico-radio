#ifndef __UTILS_H
#define __UTILS_H

#include "Beeper.h"
#include <TFT_eSPI.h>

//--- TFT colors ---
#define TFT_COLOR(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
// #define COMPLEMENT_COLOR(color) \
//     (TFT_COLOR((255 - ((color >> 16) & 0xFF)), (255 - ((color >> 8) & 0xFF)), (255 - (color & 0xFF))))
// #define PUSHED_COLOR(color) ((((color & 0xF800) >> 1) & 0xF800) | (((color & 0x07E0) >> 1) & 0x07E0) | (((color & 0x001F) >> 1) & 0x001F))

//--- Array Utils ---
#define ARRAY_ITEM_COUNT(array) (sizeof(array) / sizeof(array[0]))

//--- Debug ---
#define __DEBUG // Debug mód bekapcsolása
#ifdef __DEBUG
#define DEBUG(fmt, ...) Serial.printf_P(PSTR(fmt "\n") __VA_OPT__(, ) __VA_ARGS__)
#else
#define DEBUG(fmt, ...) // Üres makró, ha __DEBUG nincs definiálva
#endif

//--- Utils ---
// /**
//  * Biztonságos string másolás
//  * @param dest cél string
//  * @param src forrás string
//  */
#define SAFE_STRCPY(dest, src)                \
    do {                                      \
        strncpy(dest, src, sizeof(dest) - 1); \
        dest[sizeof(dest) - 1] = '\0';        \
    } while (0)
void safeStrCpy(char *dest, char *src);

/**
 * Várakozás a soros port megnyitására
 * @param pTft a TFT kijelző példánya
 * @param beeper a Beeper példánya
 */
void debugWaitForSerial(TFT_eSPI *pTft, Beeper *beeper);

#endif // __UTILS_H