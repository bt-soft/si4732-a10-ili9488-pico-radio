#ifndef __TEXTFIELD_H
#define __TEXTFIELD_H

#include <TFT_eSPI.h>

class InputTextField {
private:
    TFT_eSPI &tft;
    uint16_t x, y, w, h;
    String text;

public:
    InputTextField(TFT_eSPI &tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
        : tft(tft), x(x), y(y), w(w), h(h) {
        text = "";
    }

    void append(char c) {
        text += c;
        draw();
    }

    void backspace() {
        if (text.length() > 0) {
            text.remove(text.length() - 1);
            draw();
        }
    }

    void clear() {
        text = "";
        draw();
    }

    String getText() const {
        return text;
    }

    void draw() {
        tft.fillRect(x, y, w, h, TFT_DARKGREY);
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(text, x, y);
    }
};
#endif