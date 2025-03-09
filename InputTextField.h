#ifndef __TEXTFIELD_H
#define __TEXTFIELD_H

#include <TFT_eSPI.h>

class InputTextField {
private:
    TFT_eSPI *pTft;
    uint16_t x, y, w, h;
    String text;

public:
    InputTextField(TFT_eSPI *pTft, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
        : pTft(pTft), x(x), y(y), w(w), h(h) {
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
        pTft->fillRect(x, y, w, h, TFT_DARKGREY);
        pTft->setTextColor(TFT_WHITE);
        pTft->setTextDatum(TL_DATUM);
        pTft->drawString(text, x, y);
    }
};
#endif