#include "ConfigStore.h"

/**
 * Konfigurációs adatok
 */
Config_t config{
    .tftCalibrateData = {0, 0, 0, 0, 0}, // TFT touch kalibrációs adatok
    .someValue = 42,
    .anotherValue = 3.14,
    .name = "Default" //
};
Config_t *pConfig = &config; // Pointer a struktúrára
