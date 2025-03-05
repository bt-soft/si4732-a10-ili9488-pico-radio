#include "ConfigStore.h"

/**
 * Alapértelmezett konfigurációs adatok
 */
Config_t DEFAULT_CONFIG = {
    .tftCalibrateData = {0, 0, 0, 0, 0}, // TFT touch kalibrációs adatok
    //.tftCalibrateData = {208, 3706, 219, 3609, 7},
    .someValue = 42,
    .anotherValue = 3.14,
    .name = "Default" //
};
Config_t config = DEFAULT_CONFIG; // Az alapértelmezett konfiguráció alapján inicializálva
Config_t *pConfig = &config;      // Pointer a struktúrára
