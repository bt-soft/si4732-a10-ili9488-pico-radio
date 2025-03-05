#ifndef __BEEPER_H__
#define __BEEPER_H__

#include <Arduino.h>

/**
 * Hangjelzés osztály
 */
class Beeper {

private:
    uint8_t beeperPin;

public:
    /**
     * Konstruktor
     * @param beeperPin hangszóró lába
     */
    Beeper(uint8_t beeperPin) {
        this->beeperPin = beeperPin;
        pinMode(beeperPin, OUTPUT);
        digitalWrite(beeperPin, LOW);
    }

    /**
     *  Hangjelzés
     */
    void tick() {
        tone(beeperPin, 800);
        delay(10);
        noTone(beeperPin);
    }

    /**
     * Hiba jelzés
     */
    void error() {
        tone(beeperPin, 500);
        delay(100);
        tone(beeperPin, 500);
        delay(100);
        tone(beeperPin, 500);
        delay(100);
        noTone(beeperPin);
    }
};
#endif