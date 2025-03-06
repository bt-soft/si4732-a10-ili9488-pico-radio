#include "ConfigStore.h"

/**
 * Alapértelmezett readonly konfigurációs adatok
 */
const Config_t DEFAULT_CONFIG = {
    .tftCalibrateData = {0, 0, 0, 0, 0}, // TFT touch kalibrációs adatok
    //.tftCalibrateData = {208, 3706, 219, 3609, 7},
    .someValue = 42,
    .anotherValue = 3.14,
    .name = "Default" //
};

// A 'config' változó, amely az alapértelmezett értékeket veszi fel, kívülről nem módosítható
Config_t config = DEFAULT_CONFIG;

// A pConfig pointert a config-ra állítjuk, ezen keresztül módosítható a config
Config_t *pConfig = &config;