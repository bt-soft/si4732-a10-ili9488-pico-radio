#ifndef __TFT_BUTTON_H
#define __TFT_BUTTON_H

#include "ESP_free_fonts.h"
#include "utils.h"

// Gomb állapotai
typedef enum ButtonState_t {
    OFF,
    ON,
    DISABLED,
    //---- technikai állapotok
    HOLD,  // Nyomva tartják
    PUSHED // Csak az esemény jelzésére a calbback függvénynek, nincs színhez kötve az állapota
} ButtonState;

typedef enum ButtonType_t {
    TOGGLE,
    PUSHABLE
} ButtonType;

// Callback típusa
typedef void (*ButtonCallback)(const char *, ButtonState_t);

class TftButton {

private:
    // Benyomott gomb háttérszín gradienshez, több iterációs lépés -> erősebb hatás
    static constexpr uint8_t DARKEN_COLORS_STEPS = 6;

    TFT_eSPI *pTft;
    uint16_t x, y, w, h;
    const char *label;
    ButtonState state;
    ButtonState oldState;
    ButtonType type;
    ButtonCallback callback;
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
        if (callback) {
            callback(label, type == PUSHABLE ? PUSHED : state);
        }
    }

    /// @brief Benyomott gomb háttérszín gradiens
    /// @param color
    /// @param amount
    /// @return
    uint16_t darkenColor(uint16_t color, uint8_t amount) {
        uint8_t r = (color & 0xF800) >> 11;
        uint8_t g = (color & 0x07E0) >> 5;
        uint8_t b = (color & 0x001F);

        r = max(0, r - (amount >> 3)); // 5 bites piros csökkentés
        g = max(0, g - (amount >> 2)); // 6 bites zöld csökkentés
        b = max(0, b - (amount >> 3)); // 5 bites kék csökkentés

        return (r << 11) | (g << 5) | b;
    }

public:
    /// @brief button konstruktor
    /// @param pTft TFT példány
    /// @param x  x pozíció
    /// @param y y pozíció
    /// @param w szélesség
    /// @param h magasság
    /// @param label felirat
    /// @param type típus (push, toggle)
    /// @param callback callback
    /// @param state aktuális állapot
    TftButton(TFT_eSPI *pTft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *label, ButtonType type, ButtonCallback callback = nullptr, ButtonState state = OFF)
        : pTft(pTft), x(x), y(y), w(w), h(h), label(label), type(type), callback(callback), buttonPressed(false), state(state), oldState(state) {}

    TftButton(TFT_eSPI *pTft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const __FlashStringHelper *label, ButtonType type, ButtonCallback callback = nullptr, ButtonState state = OFF)
        : pTft(pTft), x(x), y(y), w(w), h(h), label(reinterpret_cast<const char *>(label)), type(type), callback(callback), buttonPressed(false), state(state), oldState(state) {}

    /// @brief Konstruktor csak a szélesség és a magasság megadásával.
    ///         A pozíciót kiszámítjuk máshol és beállítjuk a setPosition(uint16_t x, uint16_t y)-al
    /// @param pTft TFT példány
    /// @param w szélesség
    /// @param h magasság
    /// @param label felirat
    /// @param type típus (push, toggle)
    /// @param callback callback
    /// @param state aktuális állapot
    TftButton(TFT_eSPI *pTft, uint16_t w, uint16_t h, const __FlashStringHelper *label, ButtonType type, ButtonCallback callback = NULL, ButtonState state = OFF)
        : TftButton(pTft, w, h, reinterpret_cast<const char *>(label), type, callback, state) {}

    TftButton(TFT_eSPI *pTft, uint16_t w, uint16_t h, const char *label, ButtonType type, ButtonCallback callback = NULL, ButtonState state = OFF)
        : pTft(pTft), x(0), y(0), w(w), h(h), label(label), type(type), callback(callback), buttonPressed(false), state(state), oldState(state) {}

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
    /// @param state új állapot
    void setState(ButtonState_t state) {
        this->state = state;
        draw();
    }

    /// @brief Button állapotának lekérése
    /// @return állapot
    ButtonState_t getState() {
        return state;
    }

    /// @brief Callback függvény elkérése
    /// A PopupBase 'X' gomb kezeléséhez kell
    /// @return
    ButtonCallback getCallback() {
        return callback;
    }

    /**
     * Button állapot -> String konverzió
     */
    static const __FlashStringHelper *decodeState(ButtonState_t _state) {
        switch (_state) {
        case OFF:
            return F("Off");
        case ON:
            return F("On");
        case HOLD:
            return F("Hold");
        case DISABLED:
            return F("Disabled");
        //---- technikai állapot
        case PUSHED:
            return F("Pushed");
        default:
            return F("Unknown!!");
        }
    }
};

#endif