#include "FmDisplay.h"

// Gombok száma
#define FM_SCREEN_BUTTONS_COUNT 5

/**
 * Konstruktor
 */
FmDisplay::FmDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band, Config &config, uint16_t freqDispX, uint16_t freqDispY)
    : DisplayBase(tft, si4735, band, config), freqDispX(freqDispX), freqDispY(freqDispY) {

    // Dinamikusan létrehozzuk a gombokat
    screenButtons = new TftButton[FM_SCREEN_BUTTONS_COUNT];
    screenButtons[0] = TftButton(&tft, SCREEN_BUTTONS_X(0), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Popup"), ButtonType::PUSHABLE, SCREEN_BUTTON_CALLBACK(FmDisplay, this));
    screenButtons[1] = TftButton(&tft, SCREEN_BUTTONS_X(1), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Multi"), ButtonType::PUSHABLE, SCREEN_BUTTON_CALLBACK(FmDisplay, this));
    screenButtons[2] = TftButton(&tft, SCREEN_BUTTONS_X(2), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Sw-1"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, this));
    screenButtons[3] = TftButton(&tft, SCREEN_BUTTONS_X(3), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Sw-2"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, this));
    screenButtons[4] = TftButton(&tft, SCREEN_BUTTONS_X(4), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Dis"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, this));

    // SMeter példányosítása
    pSMeter = new SMeter(tft, 0, 80);

    // RDS példányosítása
    pRds = new RDS(tft, si4735,
                   80, 62, // Station x,y
                   0, 80,  // Message x,y
                   2, 42,  // Time x,y
                   0, 140  // program type x,y
    );

    pFreqDisplay = new FreqDisplay(tft, band, config, freqDispX, freqDispY);
}

/**
 * Destruktor
 */
FmDisplay::~FmDisplay() {
    // Gombok trölése
    if (screenButtons) {
        delete[] screenButtons;
    }

    // SMeter trölése
    if (pSMeter) {
        delete pSMeter;
    }

    // RDS trölése
    if (pRds) {
        delete pRds;
    }

    // Frekvencia kijelző törlése
    if (pFreqDisplay) {
        delete pFreqDisplay;
    }
}

/**
 * Képernyő kirajzolása
 */
void FmDisplay::drawScreen() {

    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    // RSI skála kirajzoltatása
    pSMeter->drawSmeterScale();

    // Megjelenítjük a képernyő gombokat
    for (uint8_t i = 0; i < FM_SCREEN_BUTTONS_COUNT; ++i) {
        screenButtons[i].draw();
    }

    // Az utolsó gombot letiltjuk
    screenButtons[FM_SCREEN_BUTTONS_COUNT - 1].setState(ButtonState::DISABLED); // A gomb alapértelmezés szerint le van tiltva
}

/**
 * Gombok callback
 * A megnyomott gomb visszaadja a label-jét és az állapotát
 * @param label megnyomott gomb label
 * @param state megnyomott gomb állapota
 */
void FmDisplay::ButtonCallback_t(const char *label, ButtonState_t state) {
    buttonLabel = label;
    buttonState = state;
    DEBUG("ButtonCallback_t -> label: %s, state: %s\n", buttonLabel, TftButton::decodeState(buttonState));
}

/**
 * Dialógus ablak létrehozása
 */
void FmDisplay::createPopupDialog() {
    dialog = PopUpDialog::createDialog(&tft, 300, 150, F("Dialog title"), F("Folytassuk?"), SCREEN_BUTTON_CALLBACK(FmDisplay, this), "Igen", "Lehet megse kellene");
}

/**
 * Multi gombos dialógus ablak létrehozása
 * A gombok törléséről a MultiButtonDialog desktruktora gondoskodik
 * @param buttonLabels gombok labeljei
 * @param buttonsCount gombok száma
 */
void FmDisplay::createMultiButtonDialog(const char *buttonLabels[], int buttonsCount) {
#define MULTI_BUTTON_W 80
#define MULTI_BUTTON_H 30

    TftButton **multiButtons = new TftButton *[buttonsCount];
    for (uint8_t i = 0; i < buttonsCount; i++) {
        multiButtons[i] = new TftButton(&tft, MULTI_BUTTON_W, MULTI_BUTTON_H, buttonLabels[i], ButtonType::PUSHABLE, SCREEN_BUTTON_CALLBACK(FmDisplay, this));
    }
    dialog = MultiButtonDialog::createDialog(&tft, 400, 260, F("Valasszon opciot!"), multiButtons, buttonsCount);
}

/**
 * Gombnyomás esemény kezelése
 */
void FmDisplay::handleScreenButtonPress() {

    if (strcmp("Popup", buttonLabel) == 0) {
        createPopupDialog();

    } else if (strcmp("Multi", buttonLabel) == 0) {

        const __FlashStringHelper *buttonLabels[] = {
            F("OK"), F("Cancel"), F("Retry-1"), F("Retry-2"), F("Retry-3"), F("Retry-4"), F("Retry-5"), F("Retry-6"),
            F("Retry-7"), F("Retry-8"), F("Retry-9"), F("Retry-10"), F("Retry-11"), F("Retry-12"), F("Retry-13"), F("Retry-14"), F("Retry-15")};

        createMultiButtonDialog(reinterpret_cast<const char **>(buttonLabels), ARRAY_ITEM_COUNT(buttonLabels));

    } else if (strcmp("FS List", buttonLabel) == 0) {
        // TouchCalibrate::listAllFilesInDir();

    } else {
        DEBUG("Screen button Label: '%s', állapot változás: %s\n", buttonLabel, TftButton::decodeState(buttonState));
    }
}

/**
 * Rotary encoder esemény kezelése
 * @param encoderState rotary encoder eredmény
 */
void FmDisplay::handleRotaryEncoder(RotaryEncoder::EncoderState encoderState) {

    switch (encoderState.direction) {
    case RotaryEncoder::Direction::UP:
        si4735.frequencyUp();
        break;
    case RotaryEncoder::Direction::DOWN:
        si4735.frequencyDown();
        break;
    }

    band.getBandTable(config.data.bandIdx).currentFreq = si4735.getFrequency();
    pRds->clearRds();
}

/**
 * Képernyő gombnyomás esemény kezelése
 * @touched érintés érzékelve
 * @tx érintés x koordináta
 * @ty érintés y koordináta
 */
void FmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {

    // Ha van dialóg, akkor annak a gombjainak a touch eseményeit hívjuk
    if (dialog) {
        dialog->handleTouch(touched, tx, ty);

    } else {
        // Ha nincs dialóg, de vannak képernyő gombok, akkor azok touch eseményeit hívjuk meg
        for (uint8_t i = 0; i < FM_SCREEN_BUTTONS_COUNT; ++i) {
            screenButtons[i].handleTouch(touched, tx, ty);
        }
    }

    // Nyomtak gombot?
    if (buttonLabel) {
        if (!dialog) {
            handleScreenButtonPress();

        } else {
            // Van dialog és megnyomtak rajta egy gombot
            DEBUG("Dialóg button Label: '%s', állapot változás: %s\n", buttonLabel, TftButton::decodeState(buttonState));

            // 'X'-el zárták be a dialógot?
            if (dialog and strcmp(DIALOG_CLOSE_BUTTON_LABEL, buttonLabel) == 0) {
                delete dialog;
                dialog = nullptr;
                buttonLabel = nullptr;
                drawScreen();
                return;
            }

            // Csak teszt -> töröljük a dialógot
            delete dialog;
            dialog = nullptr;
            drawScreen();
        }
        // Töröljük a gombnyomás eseményét
        buttonLabel = nullptr;
    }
}

/**
 * Mono/Stereo vétel megjelenítése
 */
void FmDisplay::showMonoStereo() {

    static bool prevCurrentPilot = false;
    bool currentPilot = si4735.getCurrentPilot();
    // Ha nem változott, nem frissítünk
    if (currentPilot == prevCurrentPilot) {
        return;
    }
    prevCurrentPilot = currentPilot; // Frissítsük az előző értéket

    // STEREO/MONO háttér
    uint32_t backGroundColor = currentPilot ? TFT_RED : TFT_BLUE;
    tft.fillRect(freqDispX + 191, freqDispY + 60, 38, 12, backGroundColor);

    // Felirat
    tft.setTextColor(TFT_WHITE, backGroundColor);
    tft.setTextSize(1);
    tft.setTextDatum(BC_DATUM);
    tft.setTextPadding(0);
    char buffer[10]; // Useful to handle string
    sprintf(buffer, "%s", si4735.getCurrentPilot() ? "STEREO" : "MONO");
    tft.drawString(buffer, freqDispX + 210, freqDispY + 71);
}

/**
 * Loop esemény kezelése
 * Változó Adatok (mono/sztereo, RDS, rssi) időzített megjelenítése
 */
void FmDisplay::handleLoop() {

    // Náhány adatot csak ritkábban frissítünk
    static long elapsedTimedValues = 0; // Kezdőérték nulla
    if ((millis() - elapsedTimedValues) >= SCREEN_COMPS_REFRESH_TIME_MSEC) {

        // RSSI
        si4735.getCurrentReceivedSignalQuality();
        uint8_t rssi = si4735.getCurrentRSSI();
        uint8_t snr = si4735.getCurrentSNR();
        pSMeter->showRSSI(rssi, snr, band.currentMode == FM);

        // RDS
        pRds->showRDS(snr);

        // Mono/Stereo
        this->showMonoStereo();

        // Frissítjük az időbélyeget
        elapsedTimedValues = millis();
    }

    // A Freqkvenciát azonnal frisítjuk, de csak ha változott
    static float lastFreq = 0;
    float currFreq = band.getBandTable(config.data.bandIdx).currentFreq; // A Rotary változtatásakor már eltettük a Band táblába
    if (lastFreq != currFreq) {
        pFreqDisplay->FreqDraw(currFreq, 0);
        lastFreq = currFreq;
    }
}
