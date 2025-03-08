#include "FmDisplay.h"

// Gombok száma
#define FM_SCREEN_BUTTONS_COUNT 12

/**
 * Konstruktor
 */
FmDisplay::FmDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band, Config &config, uint16_t freqDispX, uint16_t freqDispY)
    : DisplayBase(tft, si4735, band, config), freqDispX(freqDispX), freqDispY(freqDispY) {

    // Dinamikusan létrehozzuk a gombokat

    uint8_t id = PopupBase::DIALOG_MULTI_BUTTON_ID_START;   // Kezdő multiButton ID érték
    screenButtons = new TftButton[FM_SCREEN_BUTTONS_COUNT]; // Lefoglaljuk a gombok tömbjét

    screenButtons[0] = TftButton(id++, &tft, getAutoX(0), getAutoY(0), SCREEN_BTN_W, SCREEN_BTN_H, F("Popup"), ButtonType::PUSHABLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[1] = TftButton(id++, &tft, getAutoX(1), getAutoY(1), SCREEN_BTN_W, SCREEN_BTN_H, F("Multi"), ButtonType::PUSHABLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[2] = TftButton(id++, &tft, getAutoX(2), getAutoY(2), SCREEN_BTN_W, SCREEN_BTN_H, F("Sw-1"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[3] = TftButton(id++, &tft, getAutoX(3), getAutoY(3), SCREEN_BTN_W, SCREEN_BTN_H, F("Sw-2"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[4] = TftButton(id++, &tft, getAutoX(4), getAutoY(4), SCREEN_BTN_W, SCREEN_BTN_H, F("Dis"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[5] = TftButton(id++, &tft, getAutoX(5), getAutoY(5), SCREEN_BTN_W, SCREEN_BTN_H, F("Btn-5"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));

    screenButtons[6] = TftButton(id++, &tft, getAutoX(6), getAutoY(6), SCREEN_BTN_W, SCREEN_BTN_H, F("Btn-6"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[7] = TftButton(id++, &tft, getAutoX(7), getAutoY(7), SCREEN_BTN_W, SCREEN_BTN_H, F("Btn-7"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[8] = TftButton(id++, &tft, getAutoX(8), getAutoY(8), SCREEN_BTN_W, SCREEN_BTN_H, F("Btn-8"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[9] = TftButton(id++, &tft, getAutoX(9), getAutoY(9), SCREEN_BTN_W, SCREEN_BTN_H, F("Btn-9"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[10] = TftButton(id++, &tft, getAutoX(10), getAutoY(10), SCREEN_BTN_W, SCREEN_BTN_H, F("Btn-10"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    screenButtons[11] = TftButton(id++, &tft, getAutoX(11), getAutoY(11), SCREEN_BTN_W, SCREEN_BTN_H, F("Btn-11"), ButtonType::TOGGLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));

    // SMeter példányosítása
    pSMeter = new SMeter(tft, 0, 80);

    // RDS példányosítása
    pRds = new RDS(tft, si4735,
                   80, 62, // Station x,y
                   0, 80,  // Message x,y
                   2, 42,  // Time x,y
                   0, 140  // program type x,y
    );

    // Frekvencia kijelzés pédányosítása
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
 * (Az esetleges dialóg eltünése után a teljes képernyőt újra rajzoljuk)
 */
void FmDisplay::drawScreen() {

    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    // RSI skála kirajzoltatása
    pSMeter->drawSmeterScale();

    // RSSI aktuális érték
    si4735.getCurrentReceivedSignalQuality();
    uint8_t rssi = si4735.getCurrentRSSI();
    uint8_t snr = si4735.getCurrentSNR();
    pSMeter->showRSSI(rssi, snr, band.currentMode == FM);

    // RDS (erőből a 'valamilyen' adatok megjelenítése)
    pRds->displayRds(true);

    // Mono/Stereo aktuális érték
    this->showMonoStereo(si4735.getCurrentPilot());

    // Frekvencia
    float currFreq = band.getBandByIdx(config.data.bandIdx).currentFreq; // A Rotary változtatásakor már eltettük a Band táblába
    pFreqDisplay->FreqDraw(currFreq, 0);

    // Megjelenítjük a képernyő gombokat
    for (uint8_t i = 0; i < FM_SCREEN_BUTTONS_COUNT; ++i) {
        screenButtons[i].draw();
    }
    // Az 5. gombot letiltjuk
    screenButtons[4].setState(ButtonState::DISABLED); // A gomb alapértelmezés szerint le van tiltva
}

/**
 * Gombok callback
 * A megnyomott gomb visszaadja a label-jét és az állapotát
 * @param id megnyomott gomb ID-je
 * @param label megnyomott gomb label
 * @param state megnyomott gomb állapota
 */
void FmDisplay::buttonCallback(const uint8_t id, const char *label, ButtonState_t state) {
    lastButton = {true, id, label, state};
    DEBUG("buttonCallback -> id: %d, label: %s, state: %s\n", lastButton.id, lastButton.label, TftButton::decodeState(lastButton.state));
}

/**
 * Dialógus ablak létrehozása
 */
void FmDisplay::createPopupDialog() {
    dialog = PopUpDialog::createDialog(&tft, 300, 150, F("Dialog title"), F("Folytassuk?"), SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this), "Igen", "Lehet megse kellene");
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

    // Kezdő multiButton ID érték
    uint8_t id = PopupBase::DIALOG_MULTI_BUTTON_ID_START;

    TftButton **multiButtons = new TftButton *[buttonsCount];
    for (uint8_t i = 0; i < buttonsCount; i++) {
        multiButtons[i] = new TftButton(id++, &tft, MULTI_BUTTON_W, MULTI_BUTTON_H, buttonLabels[i], ButtonType::PUSHABLE, SCREEN_BUTTON_CALLBACK(FmDisplay, buttonCallback, this));
    }
    dialog = MultiButtonDialog::createDialog(&tft, 400, 260, F("Valasszon opciot!"), multiButtons, buttonsCount);
}

/**
 * Gombnyomás esemény kezelése
 */
void FmDisplay::handleScreenButtonPress() {

    if (strcmp("Popup", lastButton.label) == 0) {
        createPopupDialog();

    } else if (strcmp("Multi", lastButton.label) == 0) {

        const __FlashStringHelper *buttonLabels[] = {
            F("OK"), F("Cancel"), F("Retry-1"), F("Retry-2"), F("Retry-3"), F("Retry-4"), F("Retry-5"), F("Retry-6"),
            F("Retry-7"), F("Retry-8"), F("Retry-9"), F("Retry-10"), F("Retry-11"), F("Retry-12"), F("Retry-13"), F("Retry-14"), F("Retry-15")};

        createMultiButtonDialog(reinterpret_cast<const char **>(buttonLabels), ARRAY_ITEM_COUNT(buttonLabels));

    } else if (strcmp("FS List", lastButton.label) == 0) {
        // TouchCalibrate::listAllFilesInDir();

    } else {
        DEBUG("Screen button Id: %d, Label: '%s', állapot változás: %s\n", lastButton.id, lastButton.label, TftButton::decodeState(lastButton.state));
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

    band.getBandByIdx(config.data.bandIdx).currentFreq = si4735.getFrequency();
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
    if (lastButton.valid) {
        if (!dialog) {
            handleScreenButtonPress();

        } else {
            // Van dialog és megnyomtak rajta egy gombot
            DEBUG("Dialóg button Id: %d, Label: '%s', állapot változás: %s\n", lastButton.id, lastButton.label, TftButton::decodeState(lastButton.state));

            // 'X'-el zárták be a dialógot?
            if (lastButton.id == PopupBase::DIALOG_CLOSE_BUTTON_ID) {
                delete dialog;
                dialog = nullptr;
                clearLastButton();
                drawScreen();
                return;
            }

            // Csak teszt -> töröljük a dialógot
            delete dialog;
            dialog = nullptr;
            drawScreen();
        }
        // Töröljük a gombnyomás eseményét
        clearLastButton();
    }
}

/**
 * Mono/Stereo vétel megjelenítése
 */
void FmDisplay::showMonoStereo(bool stereo) {

    // STEREO/MONO háttér
    uint32_t backGroundColor = stereo ? TFT_RED : TFT_BLUE;
    tft.fillRect(freqDispX + 191, freqDispY + 60, 38, 12, backGroundColor);

    // Felirat
    tft.setFreeFont();
    tft.setTextColor(TFT_WHITE, backGroundColor);
    tft.setTextSize(1);
    tft.setTextDatum(BC_DATUM);
    tft.setTextPadding(0);
    char buffer[10]; // Useful to handle string
    sprintf(buffer, "%s", stereo ? "STEREO" : "MONO");
    tft.drawString(buffer, freqDispX + 210, freqDispY + 71);
}

/**
 * Adatok periódikus megjelenítése
 */
void FmDisplay::displayValues() {

    // Néhány adatot csak ritkábban frissítünk
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
        static bool prevStereo = false;
        bool stereo = si4735.getCurrentPilot();
        // Ha változott, akkor frissítünk
        if (stereo != prevStereo) {
            this->showMonoStereo(stereo);
            prevStereo = stereo; // Frissítsük az előző értéket
        }

        // Frissítjük az időbélyeget
        elapsedTimedValues = millis();
    }

    // A Freqkvenciát azonnal frisítjuk, de csak ha változott
    static float lastFreq = 0;
    float currFreq = band.getBandByIdx(config.data.bandIdx).currentFreq; // A Rotary változtatásakor már eltettük a Band táblába
    if (lastFreq != currFreq) {
        pFreqDisplay->FreqDraw(currFreq, 0);
        lastFreq = currFreq;
    }
}

/**
 * Loop esemény kezelése
 * Változó Adatok (mono/sztereo, RDS, rssi) időzített megjelenítése
 */
void FmDisplay::handleLoop() {

    // Ha nincs dialóg, akkor megjeleníthetjük az értékeket
    if (!dialog) {
        displayValues();
    }
}
