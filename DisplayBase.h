#ifndef __DISPLAYBASE_H
#define __DISPLAYBASE_H

#include "Band.h"
#include "RotaryEncoder.h"
#include <Arduino.h>
#include <TFT_eSPI.h> // TFT_eSPI könyvtár

#include "MultiButtonDialog.h"
#include "PopUpDialog.h"

// Képernyő gomb méret és pozíció definíciók
#define SCREEN_BTNS_X_START 5    // Gombok kezdő X koordinátája
#define SCREEN_BTNS_Y_START 250  // Gombok kezdő Y koordinátája
#define SCRN_BTN_H 30            // Gombok magassága
#define SCRN_BTN_W 70            // Gombok szélessége
#define SCREEN_BTNS_GAP 10       // Gombok közötti gap
#define SCREEN_BUTTONS_PER_ROW 6 // Egy sorban hány gomb van
#define SCREEN_BTN_ROW_SPACING 5 // Gombok sorai közötti távolság

#define SCREEN_BTNS_X(n) (SCREEN_BTNS_X_START + (SCRN_BTN_W + SCREEN_BTNS_GAP) * n)

#define SCREEN_COMPS_REFRESH_TIME_MSEC 500 // Változó adatok frissítési ciklusideje

class DisplayBase {

protected:
    TFT_eSPI &tft;
    SI4735 &si4735;
    Band &band;
    Config &config;

    PopupBase *dialog; // Dialógus pointer

    // Lenyomott gomb info
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

    /**
     * A megadott feliratú gombot nyomták meg?
     */
    bool isButton(const char *title) {
        return lastButton.valid and strcmp(title, lastButton.label) == 0;
    }

public:
    /**
     *
     */
    DisplayBase(TFT_eSPI &tft, SI4735 &si4735, Band &band, Config &config)
        : tft(tft), si4735(si4735), band(band), config(config), screenWidth(tft.width()), screenHeight(tft.height()), dialog(nullptr) {
        clearLastButton();
    }

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
    void handleLoop(RotaryEncoder::EncoderState encoderState) {

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

protected:
    // A Screen gombok automatikus elhelyezéséhez használjuk
    uint16_t screenWidth;
    uint16_t screenHeight;

    /**
     * Screen gombok automatikus X koordinátájának kiszámítása
     * Ha nem férnek el egy sorban a gombok, akkor nyit egy új sort
     */
    uint16_t getAutoX(uint8_t index) {
        uint8_t buttonsPerRow = screenWidth / (SCRN_BTN_W + SCREEN_BTNS_GAP);
        return SCREEN_BTNS_X_START + ((SCRN_BTN_W + SCREEN_BTNS_GAP) * (index % buttonsPerRow));
    }

    /**
     * Screen gombok automatikus Y koordinátájának kiszámítása
     * A gombok több sorban is elhelyezkedhetnek, az alsó sor a képernyő aljához igazodik
     */
    uint16_t getAutoY(uint8_t index, uint8_t screenButtonsCount) {
        uint8_t row = index / SCREEN_BUTTONS_PER_ROW; // Hányadik sorban van a gomb

        // Teljes gombterület kiszámítása
        uint8_t totalRows = (screenButtonsCount + SCREEN_BUTTONS_PER_ROW - 1) / SCREEN_BUTTONS_PER_ROW;
        uint16_t totalHeight = totalRows * SCRN_BTN_H + (totalRows - 1) * SCREEN_BTN_ROW_SPACING;

        // Első sor pozíciója, hogy az utolsó sor alja a kijelző aljára essen
        uint16_t firstRowY = screenHeight - totalHeight;

        // Az adott sor Y koordinátája
        return firstRowY + row * (SCRN_BTN_H + SCREEN_BTN_ROW_SPACING);
    }
};

#endif // __DISPLAYBASE_H