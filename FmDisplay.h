#ifndef __FMDISPLAY_H
#define __FMDISPLAY_H

#include "DisplayBase.h"

class FmDisplay : public DisplayBase {

private:
    void buttonCallback(const char *label, ButtonState_t state);
    void createPopupDialog();
    void createMultiButtonDialog(const char *buttonLabels[], int buttonsCount);
    void handleScreenButtonPress();
    TftButton *screenButtons; // Dinamikusan létrehozott gombok tömbje

protected:
    /**
     * Rotary encoder esemény kezelése
     * @param rotaryEncoderResult rotary encoder eredmény
     */
    void handleRotaryEncoder(RotaryEncoder::RotaryEncoderResult rotaryEncoderResult) override;

    /**
     * Képernyő gombnyomás esemény kezelése
     * @touched érintés érzékelve
     * @tx érintés x koordináta
     * @ty érintés y koordináta
     */
    void handleTouch(bool touched, uint16_t tx, uint16_t ty) override;

    /**
     * Loop esemény kezelése
     */
    void handleLoop() override;

public:
    FmDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band, Config &config);
    virtual ~FmDisplay();
    void drawScreen() override;
};

#endif