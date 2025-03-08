#ifndef __FMDISPLAY_H
#define __FMDISPLAY_H

#include "DisplayBase.h"
#include "FrequDisplay.h"
#include "Rds.h"
#include "SMeter.h"

class FmDisplay : public DisplayBase {

private:
    void buttonCallback(const uint8_t id, const char *label, ButtonState_t state); // Gombok callback
    void createPopupDialog();
    void createMultiButtonDialog(const char *buttonLabels[], int buttonsCount);
    void handleScreenButtonPress();

    void showMonoStereo(bool stereo);
    void displayValues();

    uint16_t freqDispX, freqDispY;
    TftButton *screenButtons; // Dinamikusan létrehozott gombok tömbje
    SMeter *pSMeter;
    RDS *pRds;
    FreqDisplay *pFreqDisplay;

protected:
    /**
     * Rotary encoder esemény kezelése
     * @param encoderState rotary encoder eredmény
     */
    void handleRotaryEncoder(RotaryEncoder::EncoderState encoderState) override;

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
    FmDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band, Config &config, uint16_t freqDispX, uint16_t freqDispY);
    virtual ~FmDisplay();
    void drawScreen() override;
};

#endif