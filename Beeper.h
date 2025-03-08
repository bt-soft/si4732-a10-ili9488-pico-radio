#ifndef __BEEPER_H__
#define __BEEPER_H__

#include "pinout.h"
#include <Arduino.h>

/**
 * Hangjelzés osztály
 */
class Beeper {

public:
    /**
     *  Pitty hangjelzés
     */
    static void tick() {
        tone(PIN_BEEPER, 800);
        delay(10);
        noTone(PIN_BEEPER);
    }

    /**
     * Hiba jelzés
     */
    static void error() {
        tone(PIN_BEEPER, 500);
        delay(100);
        tone(PIN_BEEPER, 500);
        delay(100);
        tone(PIN_BEEPER, 500);
        delay(100);
        noTone(PIN_BEEPER);
    }
};
#endif