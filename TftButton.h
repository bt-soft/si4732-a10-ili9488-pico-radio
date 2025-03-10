#ifndef __TFT_BUTTON_H
#define __TFT_BUTTON_H

#include "utils.h"

#include "EventManager.h"

// Gomb állapotai
typedef enum ButtonState_t {
    OFF,
    ON,
    DISABLED,
    //---- technikai állapotok
    HOLD,   // Nyomva tartják
    PUSHED, // Csak az esemény jelzésére a calbback függvénynek, nincs színhez kötve az állapota
    UNKNOWN // ismeretlen
} ButtonState;

// Gomb állapot String-ek a FLASH memóriából
static const __FlashStringHelper *BUTTON_STATE_NAMES[] = {F("Off"), F("On"), F("Disabled"), F("Hold"), F("Pushed")};

typedef enum ButtonType_t {
    TOGGLE,
    PUSHABLE
} ButtonType;

// Callback típusa egy osztály metódusára (id, felirat, állapot)
typedef std::function<void(const uint8_t id, const char *, ButtonState_t)> ButtonCallback_t;

// Makró egy osztály callback referenciájának átadására
#define SCRN_BTN_CB(ClassName, MethodName, instance) std::bind(&ClassName::MethodName, instance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

class TftButton {

private:
    // Benyomott gomb háttérszín gradienshez, több iterációs lépés -> erősebb hatás
    static constexpr uint8_t DARKEN_COLORS_STEPS = 6;

    TFT_eSPI *pTft;            // Itt pointert használunk a dinamikus tömbök miatt (nem lehet null referenciát használni)
    uint16_t x, y, w, h;       // pozíciója
    uint8_t id;                // A gomb ID-je
    const char *label;         // A gomb felirata
    ButtonState_t state;       // Állapota
    ButtonState_t oldState;    // Előző állapota
    ButtonType_t type;         // Típusa
    ButtonCallback_t callback; // Callback függvénye
    uint16_t colors[3] = {TFT_COLOR(65, 65, 114) /*normal*/, TFT_COLOR(65, 65, 114) /*pushed*/, TFT_COLOR(65, 65, 65) /* diabled */};
    bool buttonPressed; // Flag a gomb nyomva tartásának követésére

    /// @brief Ezt a gombot nyomták meg?
    /// @param tx touch x
    /// @param ty touch y
    /// @return true -> ezt a gombot nyomták meg
    bool contains(uint16_t tx, uint16_t ty) {
        return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
    }

    /// @brief Lenyomták a gombot
    void pressed() {
        buttonPressed = true;
        oldState = state;
        state = HOLD;
        draw();
    }

    /// @brief Felengedték a gombot
    void released() {
        buttonPressed = false;
        if (type == TOGGLE) {
            state = (oldState == OFF) ? ON : OFF;
        } else {
            state = OFF;
        }
        oldState = state;

        draw();

        // Meghívjuk a callback függvényt, ha van
        if (callback) {
            callback(id, label, type == PUSHABLE ? PUSHED : state);
            EventManager::SensorEventData sensorData = {22.3, 55.4};
            eventManager.publish(EventManager::SENSOR_UPDATE, &sensorData);
        }
    }

    /// @brief Benyomott gomb háttérszín gradiens
    /// @param color
    /// @param amount
    /// @return
    uint16_t darkenColor(uint16_t color, uint8_t amount) const {
        uint8_t r = (color & 0xF800) >> 11;
        uint8_t g = (color & 0x07E0) >> 5;
        uint8_t b = (color & 0x001F);

        // A max() hívásnál implicit konverzió történik uint8_t → int, ami elkerülhető így
        r = (r > (amount >> 3)) ? r - (amount >> 3) : 0;
        g = (g > (amount >> 2)) ? g - (amount >> 2) : 0;
        b = (b > (amount >> 3)) ? b - (amount >> 3) : 0;

        return (r << 11) | (g << 5) | b;
    }

public:
    /**
     * Default konstruktor
     * (pl.: a dinamikus tömb deklarációhoz)
     */
    TftButton() {}

    /**
     * Konstruktor
     */
    TftButton(uint8_t id, TFT_eSPI &tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *label, ButtonType_t type, ButtonCallback_t callback = nullptr, ButtonState_t state = OFF)
        : id(id), pTft(&tft), x(x), y(y), w(w), h(h), label(label), type(type), callback(callback), buttonPressed(false), state(state), oldState(state) {}

    /**
     * Konstruktor X/Y pozíció nélkül
     * Automatikus elrendezéshez csak a szélesség és a magasság van megadva
     */
    TftButton(uint8_t id, TFT_eSPI &tft, uint16_t w, uint16_t h, const char *label, ButtonType_t type, ButtonCallback_t callback = NULL, ButtonState_t state = OFF)
        : id(id), pTft(&tft), x(0), y(0), w(w), h(h), label(label), type(type), callback(callback), buttonPressed(false), state(state), oldState(state) {}

    // A label F() makróval megadva
    // TftButton(uint8_t id, TFT_eSPI &tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const __FlashStringHelper *label, ButtonType_t type, ButtonCallback_t callback = nullptr, ButtonState_t state = OFF)
    //     : id(id), pTft(&tft), x(x), y(y), w(w), h(h), label(reinterpret_cast<const char *>(label)), type(type), callback(callback), buttonPressed(false), state(state), oldState(state) {}
    // TftButton(uint8_t id, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *label, ButtonType_t type, ButtonCallback_t callback = NULL, ButtonState_t state = OFF)
    //     : TftButton(id, tft, w, h, reinterpret_cast<const char *>(label), type, callback, state) {}

    /// @brief Destruktor
    virtual ~TftButton() {
    }

    /// @brief Button szélességének lekérése
    /// @return
    uint8_t getWidth() {
        return w;
    }

    /// @brief Button x/y pozíciójának beállítása
    /// @param x
    /// @param y
    void setPosition(uint16_t x, uint16_t y) {
        this->x = x;
        this->y = y;
    }

    /// @brief button kirajzolása
    void draw() {

        // A gomb teljes szélességét és magasságát kihasználó sötétedés -> benyomás hatás keltés
        if (buttonPressed) {
            uint8_t stepWidth = w / DARKEN_COLORS_STEPS;
            uint8_t stepHeight = h / DARKEN_COLORS_STEPS;
            for (uint8_t i = 0; i < DARKEN_COLORS_STEPS; i++) {
                uint16_t fadedColor = darkenColor(colors[oldState], i * 30); // Erősebb sötétítés
                pTft->fillRoundRect(x + i * stepWidth / 2, y + i * stepHeight / 2, w - i * stepWidth, h - i * stepHeight, 5, fadedColor);
            }
        } else {
            pTft->fillRoundRect(x, y, w, h, 5, colors[state]);
        }

        // Ha tiltott, akkor sötétszürke a keret, ha aktív, akkor zöld, narancs ha nyomják
        pTft->drawRoundRect(x, y, w, h, 5, state == DISABLED ? TFT_DARKGREY : state == ON ? TFT_GREEN
                                                                          : buttonPressed ? TFT_ORANGE
                                                                                          : TFT_WHITE);
        // zöld a szöveg, ha aktív, narancs ha nyomják
        pTft->setTextColor(state == DISABLED ? TFT_DARKGREY : state == ON ? TFT_GREEN
                                                          : buttonPressed ? TFT_ORANGE
                                                                          : TFT_WHITE);
        // Az (x, y) koordináta a szöveg középpontja
        pTft->setTextDatum(MC_DATUM);

        // Fontváltás a gomb feliratozásához
        pTft->setFreeFont(&FreeSansBold9pt7b);
        pTft->setTextSize(1);
        pTft->setTextPadding(0);
        constexpr uint8_t BUTTON_LABEL_MARGIN_TOP = 3; // A felirat a gomb felső részéhez képest
        pTft->drawString(label, x + w / 2, y - BUTTON_LABEL_MARGIN_TOP + h / 2);

        // LED csík kirajzolása ha a gomb aktív vagy push, és nyomják
        constexpr uint8_t BUTTON_LED_HEIGHT = 5;
        if (state == ON or (type == PUSHABLE and buttonPressed)) {
            pTft->fillRect(x + 10, y + h - BUTTON_LED_HEIGHT - 3, w - 20, BUTTON_LED_HEIGHT, TFT_GREEN);
        } else if (type == TOGGLE) {
            pTft->fillRect(x + 10, y + h - BUTTON_LED_HEIGHT - 3, w - 20, BUTTON_LED_HEIGHT, TFT_DARKGREEN);
        }
    }

    /// @brief A gomb touch eseményeinek kezelése
    /// @param touched Jelzi, hogy történt-e érintési esemény.
    /// @param tx Az érintési esemény x-koordinátája.
    /// @param ty Az érintési esemény y-koordinátája.
    void handleTouch(bool touched, uint16_t tx, uint16_t ty) {

        // Ha tiltott a gomb, akkor nem megyünk tovább
        if (state == DISABLED) {
            return;
        }

        // Ha van touch, de még nincs lenyomva a gomb, és erre a gombra jött a touch
        if (touched and !buttonPressed and contains(tx, ty)) {
            pressed();
        } else if (!touched and buttonPressed) {
            // Ha nincs ugyan touch, de ezt a gombot nyomva tartották eddig
            released();
        }
    }

    /// @brief Button állapotának beállítása
    /// @param newState új állapot
    void setState(ButtonState_t newState) {
        if (state != newState) {
            state = newState;
            draw();
        }
    }

    /// @brief Button állapotának lekérése
    /// @return állapot
    ButtonState_t getState() {
        return state;
    }

    /// @brief Callback függvény elkérése
    /// A PopupBase 'X' gomb kezeléséhez kell
    /// @return
    ButtonCallback_t getCallback() {
        return callback;
    }

    /**
     * Button állapot -> String konverzió
     */
    static const __FlashStringHelper *decodeState(ButtonState_t _state) {
        return (_state <= PUSHED) ? BUTTON_STATE_NAMES[_state] : F("Unknown!!");
    }
};

#endif