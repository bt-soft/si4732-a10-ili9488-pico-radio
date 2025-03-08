#ifndef __DISPLAYBASE_H
#define __DISPLAYBASE_H

#include "Band.h"
#include "RotaryEncoder.h"
#include <Arduino.h>
#include <TFT_eSPI.h> // TFT_eSPI könyvtár

#include "MultiButtonDialog.h"
#include "PopUpDialog.h"

#define SCREEN_BUTTONS_X_START 5 // Gombok kezdő X koordinátája
#define SCREEN_BUTTONS_Y 280
#define SCREEN_BUTTON_HEIGHT 30
#define SCREEN_BUTTON_WIDTH 70
#define SCREEN_BUTTONS_GAP 10 // Define the gap between buttons
#define SCREEN_BUTTONS_X(n) (SCREEN_BUTTONS_X_START + (SCREEN_BUTTON_WIDTH + SCREEN_BUTTONS_GAP) * n)
#define SCREEN_COMPS_REFRESH_TIME_MSEC 500 // Változó adatok frissítési ciklusideje

class DisplayBase {

protected:
    TFT_eSPI &tft;
    SI4735 &si4735;
    Band &band;
    Config &config;

    PopupBase *dialog = nullptr; /// @brief dialógus pointer

    struct ButtonInfo_t {
        bool valid;
        uint8_t id;
        const char *label;
        ButtonState_t state;
    };
    ButtonInfo_t lastButton; // Az utolsó megnyomott gomb adatai

    /**
     * Lenyomott gomb info törlése
     */
    void clearLastButton() {
        lastButton = {
            false,
            PopupBase::DIALOG_UNDEFINED_BUTTON_ID,
            nullptr,
            ButtonState_t::UNKNOWN};
    }

    /**
     * Rotary encoder esemény kezelése
     * @param encoderState rotary encoder eredmény
     */
    virtual void handleRotaryEncoder(RotaryEncoder::EncoderState encoderState) = 0;

    /**
     * Képernyő gombnyomás esemény kezelése
     * @touched érintés érzékelve
     * @tx érintés x koordináta
     * @ty érintés y koordináta
     */
    virtual void handleTouch(bool touched, uint16_t tx, uint16_t ty) = 0;

    /**
     * Loop esemény kezelése
     */
    virtual void handleLoop() = 0;

public:
    /**
     *
     */
    DisplayBase(TFT_eSPI &tft, SI4735 &si4735, Band &band, Config &config) : tft(tft), si4735(si4735), band(band), config(config) {}

    /**
     *
     */
    virtual ~DisplayBase() {
        if (dialog != nullptr) {
            delete dialog;
        }
    }

    /**
     * Képernyő kirajzolása
     */
    virtual void drawScreen() = 0;

    /**
     * Loop esemény kezelése
     * @param encoderState rotary encoder eredmény
     */
    void handeLoop(RotaryEncoder::EncoderState encoderState) {

        // Rotary Encoder olvasása
        if (encoderState.direction != RotaryEncoder::Direction::NONE) {
            try {
                handleRotaryEncoder(encoderState);
            } catch (const std::exception &e) {
                DEBUG("Hiba a handleRotaryEncoder() függvényben: %s\n", e.what());
            }
        }

        uint16_t tx, ty;
        bool touched = tft.getTouch(&tx, &ty, 40); // A treshold értékét megnöveljük a default 20msec-ről 40-re
        try {
            handleTouch(touched, tx, ty);
        } catch (const std::exception &e) {
            DEBUG("Hiba a handleTouch() függvényben: %s\n", e.what());
        }

        try {
            handleLoop();
        } catch (const std::exception &e) {
            DEBUG("Hiba a handleLoop() függvényben: %s\n", e.what());
        }
    }
};

#endif // __DISPLAYBASE_H