#include <Arduino.h>

//------------------- Pins
#include "pinout.h"

//------------------ TFT
#include <TFT_eSPI.h> // TFT_eSPI könyvtár
TFT_eSPI tft;         // TFT objektum
// TFT_eSprite spr = TFT_eSprite(&tft); // Sprite objektum

#include "MultiButtonDialog.h"
#include "PopUpDialog.h"

PopupBase *dialog = nullptr;       /// @brief dialógus pointer
const char *buttonLabel = nullptr; /// @brief Megnyomott gomb label
ButtonState_t buttonState;         /// @brief Megnyomott gomb állapota

//------------------- Rotary Encoder
// Pico Hardware timer a Rotary encoder olvasására
#include <Ticker.h>
Ticker rotaryTicker;

#include "RotaryEncoder.h"
RotaryEncoder rotaryEncoder = RotaryEncoder(PIN_ENCODER_CLK, PIN_ENCODER_DT, PIN_ENCODER_SW);

//------------------- beeper
#include "Beeper.h"
Beeper beeper = Beeper(PIN_BEEPER);

//------------------- si4735
#include <SI4735.h>
SI4735 si4735;

//------------------- EEPROM Config
#define EEPROM_SAVE_CHECK_INTERVAL_SECONDS 60 * 5 // 5 perc
Ticker eepromSaveChecker;
// Core lock, az EEPROM írásánálhasználjuk
auto_init_mutex(saveEepromMutex);

#include "Config.h"
Config config;

//------------------- Band
#include "Band.h"
Band band(si4735, config);

/**
 * Gombok callback
 * A megnyomott gomb visszaadja a label-jét és az állapotát
 * @param label megnyomott gomb label
 * @param state megnyomott gomb állapota
 */
void buttonCallback(const char *label, ButtonState_t state) {
    buttonLabel = label;
    buttonState = state;
}

#define SCREEN_BUTTONS_X_START 5 // Gombok kezdő X koordinátája
#define SCREEN_BUTTONS_Y 100
#define SCREEN_BUTTON_HEIGHT 30
#define SCREEN_BUTTON_WIDTH 70
#define SCREEN_BUTTONS_GAP 10 // Define the gap between buttons
#define BUTTON_X(n) (SCREEN_BUTTONS_X_START + (SCREEN_BUTTON_WIDTH + SCREEN_BUTTONS_GAP) * n)
TftButton screenButtons[] = {
    TftButton(&tft, BUTTON_X(0), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Popup"), ButtonType::PUSHABLE, buttonCallback),
    TftButton(&tft, BUTTON_X(1), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Multi"), ButtonType::PUSHABLE, buttonCallback),
    TftButton(&tft, BUTTON_X(2), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Sw-1"), ButtonType::TOGGLE, buttonCallback),
    TftButton(&tft, BUTTON_X(3), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Sw-2"), ButtonType::TOGGLE, buttonCallback, ButtonState::ON),
    TftButton(&tft, BUTTON_X(4), SCREEN_BUTTONS_Y, SCREEN_BUTTON_WIDTH, SCREEN_BUTTON_HEIGHT, F("Dis"), ButtonType::TOGGLE, buttonCallback) //
};

//---------------------------

/**
 * Dialógus ablak létrehozása
 */
void createPopupDialog() {
    dialog = PopUpDialog::createDialog(&tft, 300, 150, F("Dialog title"), F("Folytassuk?"), buttonCallback, "Igen", "Lehet megse kellene");
}

/**
 * Multi gombos dialógus ablak létrehozása
 * @param buttonLabels gombok labeljei
 * @param buttonsCount gombok száma
 */
void createMultiButtonDialog(const char *buttonLabels[], int buttonsCount) {
#define MULTI_BUTTON_W 80
#define MULTI_BUTTON_H 30

    TftButton **multiButtons = new TftButton *[buttonsCount];
    for (uint8_t i = 0; i < buttonsCount; i++) {
        multiButtons[i] = new TftButton(&tft, MULTI_BUTTON_W, MULTI_BUTTON_H, buttonLabels[i], ButtonType::PUSHABLE, buttonCallback);
    }
    dialog = MultiButtonDialog::createDialog(&tft, 400, 260, F("Valasszon opciot!"), multiButtons, buttonsCount);
}

/**
 * Gombnyomás esemény kezelése
 */
void handleScreenButtonPress() {

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
 * Képernyő kirajzolása
 */
void drawScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);
    // Megjelenítjük a képernyő gombokat
    for (TftButton &screenButton : screenButtons) {
        screenButton.draw();
    }

    // Az 5.-et letiltjuk
    // screenButtons[5].setState(ButtonState::DISABLED); // A gomb alapértelmezés szerint le van tiltva
}

/**
 * Arduino setup
 */
void setup() {
#ifdef __DEBUG
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    // TFT LED kimenet
    pinMode(PIN_DISPLAY_LED, OUTPUT);
    digitalWrite(PIN_DISPLAY_LED, 0);

    // Rotary Encoder felhúzása
    rotaryEncoder.setDoubleClickEnabled(true);
    rotaryEncoder.setAccelerationEnabled(false);

    // Pico Ticker beállítása a Rotary Encoder olvasására
    rotaryTicker.attach_ms(5, []() {
        rotaryEncoder.service();
    });
    // Pico Ticker beállítása az EEPROM adatok mentésének ellenőrzésére
    eepromSaveChecker.attach(EEPROM_SAVE_CHECK_INTERVAL_SECONDS, []() {
        // Lokkolunk, hogy ne tudjuk piszkálni a konfigot a mentés közben
        CoreMutex mtx(&saveEepromMutex);
        config.checkSave();
    });

    // TFT inicializálása
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Várakozás a soros port megnyitására
    // debugWaitForSerial(&tft, &beeper);

    // Ha a bekapcsolás alatt nyomva tartjuk a rotary gombját, akkor töröljük a konfigot
    if (digitalRead(PIN_ENCODER_SW) == LOW) {
        beeper.tick();
        delay(1500);
        if (digitalRead(PIN_ENCODER_SW) == LOW) { // Ha még mindig nyomják
            config.loadDefaults();
            beeper.tick();
            DEBUG("Default settings resored!\n");
        }
    } else {
        // konfig betöltése
        config.load();
    }

    // Kell kalibrálni a TFT Touch-t?
    if (isZeroArray(config.data.tftCalibrateData)) {
        beeper.error();
        tftTouchCalibrate(&tft, config.data.tftCalibrateData);
    }
    // Beállítjuk a touch scren-t
    tft.setTouch(config.data.tftCalibrateData);

    // si473x
    Wire.setSDA(PIN_SI4735_I2C_SDA); // I2C for SI4735 SDA
    Wire.setSCL(PIN_SI4735_I2C_SCL); // I2C for SI4735 SCL
    Wire.begin();

    // Si4735 inicializálása
    int16_t si4735Addr = si4735.getDeviceI2CAddress(PIN_SI4735_RESET);
    if (si4735Addr == 0) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        const char *txt = "Si4735 not detected";
        tft.print(txt);
        DEBUG(txt);
        beeper.error();
        while (true) // nem megyünk tovább
            ;
    }

    // Megtaláltuk az SI4735-öt
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print(F("Si473X addr:  "));
    tft.println(si4735Addr, HEX);
    delay(1500);

    si4735.setDeviceI2CAddress(si4735Addr == 0x11 ? 0 : 1); // Sets the I2C Bus Address, FIXME: ez minek kell, ha egyszer már elárulta a címét?
    si4735.setAudioMuteMcuPin(PIN_AUDIO_MUTE);              // Audio Mute pin

    band.BandInit();
    band.BandSet();
    si4735.setVolume(config.data.currentVOL);

    // Képernyő kirajzolása az aktuálismódban
    drawScreen();
}

/**
 * Arduino loop
 */
void loop() {

    // Lokkolunk, hogy ne tudjuk menteni az EEPROM-ot a változtatások közben
    CoreMutex mtx(&saveEepromMutex);

    // Rotary Encoder olvasása
    RotaryEncoder::RotaryEncoderResult rotaryEncoderResult = rotaryEncoder.readRotaryEncoder();
    if (rotaryEncoderResult.direction != RotaryEncoder::Direction::NONE) {
        switch (rotaryEncoderResult.direction) {
        case RotaryEncoder::Direction::UP:
            DEBUG("Rotary Encoder UP\n");
            si4735.frequencyUp();
            break;
        case RotaryEncoder::Direction::DOWN:
            DEBUG("Rotary Encoder DOWN\n");
            si4735.frequencyDown();
            break;
        }

        band.getBandTable(config.data.bandIdx).currentFreq = si4735.getFrequency();
    }

    try {
        // Touch esemény lekérdezése
        uint16_t tx, ty;
        bool touched = tft.getTouch(&tx, &ty, 40); // A treshold értékét megnöveljük a default 20msec-ről 40-re

        // Ha van dialóg, akkor annak a gombjainak a touch eseményeit hívjuk
        if (dialog) {
            dialog->handleTouch(touched, tx, ty);
        } else {
            // Ha nincs dialóg, de vannak képernyő gombok, akkor azok touch eseményeit hívjuk meg
            for (TftButton &screenButton : screenButtons) {
                screenButton.handleTouch(touched, tx, ty);
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
    } catch (...) {
        DEBUG("Hiba történt a touch esemény feldolgozásakor!\n");
        buttonLabel = nullptr;
    }
}
