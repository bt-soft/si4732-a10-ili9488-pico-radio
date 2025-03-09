#include <Arduino.h>

#include "utils.h"

//------------------- Pins
#include "pinout.h"

//------------------ TFT
#include <TFT_eSPI.h> // TFT_eSPI könyvtár
TFT_eSPI tft;         // TFT objektum
#include "ESP_free_fonts.h"

#include "FmDisplay.h"
DisplayBase *pDisplay;

//------------------- Rotary Encoder
// Pico Hardware timer a Rotary encoder olvasására
#include <Ticker.h>
Ticker rotaryTicker;

#include "RotaryEncoder.h"
#define ROTARY_ENCODER_TICKER_INTERVAL_MSEC 1 // 1msec
RotaryEncoder rotaryEncoder = RotaryEncoder(PIN_ENCODER_CLK, PIN_ENCODER_DT, PIN_ENCODER_SW);

//------------------- beeper
#include "Beeper.h"

//------------------- si4735
#include <SI4735.h>
SI4735 si4735;

//------------------- EEPROM Config
#define EEPROM_SAVE_CHECK_TICKER_INTERVAL_SECONDS 60 * 5 // 5 perc
Ticker eepromSaveChecker;
auto_init_mutex(saveEepromMutex); // Core lock, az EEPROM írásánálhasználjuk

#include "Config.h"
Config config;

//------------------- Band
#include "Band.h"
Band band(si4735, config);

//------------------- Runtime variables
#include "RuntimeVars.h"

//------------------- Memória információk megjelenítése
#include "PicoMemoryInfo.h"
#ifdef __DEBUG
#define MEMORY_INFO_TICKER_INTERVAL_SECONDS 60 * 1 // 1 perc
Ticker memoryInfoTicker;
#endif
//--------------------------------------------------------------------------------------------------------------------------------------

/**
 * Arduino setup
 */
void setup() {
#ifdef __DEBUG
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    // Beeper
    pinMode(PIN_BEEPER, OUTPUT);
    digitalWrite(PIN_BEEPER, LOW);

    // TFT LED kimenet
    pinMode(PIN_DISPLAY_LED, OUTPUT);
    digitalWrite(PIN_DISPLAY_LED, 0);

    // Rotary Encoder beállítása
    rotaryEncoder.setDoubleClickEnabled(true);
    rotaryEncoder.setAccelerationEnabled(true);

    // Pico Ticker beállítása a Rotary Encoder olvasására
    rotaryTicker.attach_ms(ROTARY_ENCODER_TICKER_INTERVAL_MSEC, []() {
        rotaryEncoder.service();
    });
    // Pico Ticker beállítása az EEPROM adatok mentésének ellenőrzésére
    eepromSaveChecker.attach(EEPROM_SAVE_CHECK_TICKER_INTERVAL_SECONDS, []() {
        // Lokkolunk, hogy ne tudjuk piszkálni a konfigot a mentés közben
        CoreMutex mtx(&saveEepromMutex);
        config.checkSave();
    });

    // TFT inicializálása
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setFreeFont(FF18);

    // Várakozás a soros port megnyitására
    debugWaitForSerial(&tft);

    // Ha a bekapcsolás alatt nyomva tartjuk a rotary gombját, akkor töröljük a konfigot
    if (digitalRead(PIN_ENCODER_SW) == LOW) {
        Beeper::tick();
        delay(1500);
        if (digitalRead(PIN_ENCODER_SW) == LOW) { // Ha még mindig nyomják
            config.loadDefaults();
            Beeper::tick();
            DEBUG("Default settings resored!\n");
        }
    } else {
        // konfig betöltése
        config.load();
    }

    // Kell kalibrálni a TFT Touch-t?
    if (isZeroArray(config.data.tftCalibrateData)) {
        Beeper::error();
        tftTouchCalibrate(&tft, config.data.tftCalibrateData);
    }
    // Beállítjuk a touch scren-t
    tft.setTouch(config.data.tftCalibrateData);

    // si473x (Nem a default I2C lábakon [4,5] van!!!)
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
        Beeper::error();
        while (true) // nem megyünk tovább
            ;
    }
    si4735.setDeviceI2CAddress(si4735Addr == 0x11 ? 0 : 1); // Sets the I2C Bus Address

    // Megtaláltuk az SI4735-öt, kiírjuk az I2C címét a képernyőre
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print(F("Si473X addr:  0x"));
    tft.println(si4735Addr, HEX);
    delay(1500);

    // Band init
    band.BandInit();
    band.BandSet();

    // Si4735 init
    si4735.setVolume(config.data.currentVOL);  // Hangerő
    si4735.setAudioMuteMcuPin(PIN_AUDIO_MUTE); // Audio Mute pin

    // Képernyő példányosítása az aktuális mód alapján
    if (band.currentMode == FM) {
        pDisplay = new FmDisplay(tft, si4735, band, config, 0, 0);
    } else {
        // pDisplay = new AmDisplay(tft, si4735, band);
    }

    // Képernyő kirajzolása az aktuálismódban
    pDisplay->drawScreen();

#ifdef __DEBUG
    // Memória információk megjelenítése a Serial-on DEBUG módban
    debugMemoryInfo();
    memoryInfoTicker.attach(MEMORY_INFO_TICKER_INTERVAL_SECONDS, []() {
        debugMemoryInfo();
    });
#endif
}

/**
 * Arduino loop
 */
void loop() {

    // Lokkolunk, hogy ne tudjuk menteni az EEPROM-ot a változtatások közben
    CoreMutex mtx(&saveEepromMutex);

    // ======================= Manage Squelch =========================
    // squelchIndicator(pCfg->vars.currentSquelch);
    if (!muteStat) {
        si4735.getCurrentReceivedSignalQuality();
        uint8_t rssi = si4735.getCurrentRSSI();
        uint8_t snr = si4735.getCurrentSNR();

        uint8_t signalQuality = config.data.squelchUsesRSSI ? rssi : snr;
        if (signalQuality >= config.data.currentSquelch) {
            if (SCANpause == true) {
                si4735.setAudioMute(AUDIO_MUTE_OFF);
                squelchDecay = millis();
            }
        } else {
            if (millis() > (squelchDecay + SQUELCH_DECAY_TIME)) {
                si4735.setAudioMute(AUDIO_MUTE_ON);
            }
        }
    }
    // ================================================================

    // Rotary Encoder olvasása
    RotaryEncoder::EncoderState encoderState = rotaryEncoder.read();
    if (encoderState.buttonState == RotaryEncoder::ButtonState::Held) {
        // TODO: Kikapcsolás figyelését még implementálni
        DEBUG("Ki kellene kapcsolni...\n");
        delay(1000);
        return;
    }

    // Aktuális Display loopja
    pDisplay->handleLoop(encoderState);
}
