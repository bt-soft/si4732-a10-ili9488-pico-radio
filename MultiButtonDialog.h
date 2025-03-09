#ifndef __MULTIBUTTONDIALOG_H
#define __MULTIBUTTONDIALOG_H

#include "PopupBase.h"
#include "TftButton.h"

#define MULTI_BTN_W 80 // Multibutton dialog gombjainak szélessége
#define MULTI_BTN_H 30 // Multibutton dialog gombjainak magassága

/**
 * @class MultiButtonDialog
 * @brief Több gombos párbeszédpanel TFT képernyőn.
 *
 * Ez az osztály egy olyan párbeszédpanelt képvisel, amely üzenetet és több gombot jelenít meg egy TFT képernyőn.
 * A PopupBase osztályból származik.
 */
class MultiButtonDialog : public PopupBase {

private:
    /**
     * @brief Kiszámítja a gombok elrendezését.
     *
     * @param maxRowWidth A rendelkezésre álló szélesség
     * @param buttonsPerRow Hány gomb fér el egy sorban
     * @param rowCount Hány sorra van szükség
     */
    void calculateButtonLayout(uint16_t maxRowWidth, uint8_t &buttonsPerRow, uint8_t &rowCount) {
        buttonsPerRow = 0;
        uint16_t totalWidth = 0;

        // Próbáljuk feltölteni egy sort, amíg elférnek a gombok
        for (uint8_t i = 0; i < buttonCount; i++) {
            uint16_t nextWidth = totalWidth + buttons[i]->getWidth() + (buttonsPerRow > 0 ? DLG_BTN_GAP : 0);

            if (nextWidth > maxRowWidth) {
                break; // Ha már nem fér el, kilépünk
            }

            totalWidth = nextWidth;
            buttonsPerRow++;
        }

        rowCount = (buttonCount + buttonsPerRow - 1) / buttonsPerRow; // Felkerekítés
    }

    /**
     * @brief Gombok elhelyezése a dialóguson belül.
     */
    void positionButtons(uint8_t buttonsPerRow, uint8_t rowCount) {
        uint16_t buttonHeight = DLG_BTN_H;
        uint16_t totalHeight = rowCount * buttonHeight + (rowCount - 1) * DLG_BTN_GAP;

        uint16_t startY = contentY;
        uint8_t row = 0, col = 0;
        uint16_t startX = 0;

        for (uint8_t i = 0; i < buttonCount; i++) {
            if (col == 0) {
                uint16_t rowWidth = 0;
                uint8_t itemsInRow = min(buttonsPerRow, buttonCount - i);
                for (uint8_t j = 0; j < itemsInRow; j++) {
                    rowWidth += buttons[i + j]->getWidth();
                }
                rowWidth += (itemsInRow - 1) * DLG_BTN_GAP;
                startX = x + (w - rowWidth) / 2;
            }

            buttons[i]->setPosition(startX, startY);
            startX += buttons[i]->getWidth() + DLG_BTN_GAP;
            col++;

            if (col >= buttonsPerRow) {
                col = 0;
                row++;
                startY += buttonHeight + DLG_BTN_GAP;
            }
        }
    }

protected:
    TftButton **buttons;             // A megjelenítendő gombok mutatóinak tömbje.
    uint8_t buttonCount;             // A párbeszédpanelen lévő gombok száma.
    ButtonCallback_t buttonCallback; // Gombok callback-ja

    /**
     *
     */
    void buildButtonArray(const char *buttonLabels[], uint8_t buttonCount, ButtonCallback_t btnCb) {

        if (!buttonLabels || buttonCount == 0) {
            return;
        }

        this->buttonCount = buttonCount;
        this->buttonCallback = btnCb;
        buttons = new TftButton *[buttonCount];

        // Kezdő multiButton ID érték
        uint8_t id = PopupBase::DLG_MULTI_BTN_ID_START;

        // Button array feltöltése a gombokkal
        for (uint8_t i = 0; i < buttonCount; i++) {
            buttons[i] = new TftButton(id++, pTft, MULTI_BTN_W, MULTI_BTN_H, buttonLabels[i], ButtonType::PUSHABLE, buttonCallback);
        }
    }

    /**
     * @brief A gombok elhelyezésének fő metódusa.
     */
    virtual void placeButtons() {
        if (!buttons || buttonCount == 0) {
            return;
        }

        uint16_t maxRowWidth = w - 20;
        uint8_t buttonsPerRow, rowCount;
        calculateButtonLayout(maxRowWidth, buttonsPerRow, rowCount);
        positionButtons(buttonsPerRow, rowCount);
    }

    /**
     * @brief Ellenőrzi és kezeli az "X" gomb érintését.
     *
     * @param touched Történt-e érintési esemény
     * @param tx Az érintési esemény X koordinátája
     * @param ty Az érintési esemény Y koordinátája
     * @return true ha az "X" gombot érintették, különben false
     */
    virtual bool handleCloseButtonTouch(bool touched, uint16_t tx, uint16_t ty) {
        if (PopupBase::checkCloseButtonTouch(touched, tx, ty)) {

            // Megszerezzük az első gombtól a callback függvényt, és meghívjuk az "X" gombfelirattal
            ButtonCallback_t callback = (buttonCount > 0) ? buttons[0]->getCallback() : nullptr;
            if (callback) {
                callback(PopupBase::DIALOG_CLOSE_BUTTON_ID, PopupBase::DIALOG_CLOSE_BUTTON_LABEL, ButtonState::PUSHED);
                return true;
            }
        }
        return false;
    }

public:
    /**
     * @brief MultiButtonDialog létrehozása gombokkal, üzenet nélkül
     *
     * @param pTft Pointer a TFT_eSPI objektumra.
     * @param w A párbeszédpanel szélessége.
     * @param h A párbeszédpanel magassága.
     * @param title A dialógus címe (opcionális).
     * @param buttons A gombok mutatóinak tömbje.
     */
    MultiButtonDialog(TFT_eSPI *pTft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const char *buttonLabels[] = nullptr, uint8_t buttonCount = 0, ButtonCallback_t buttonCallback = nullptr)
        : PopupBase(pTft, w, h, title) {

        // Legyártjuk a gombok tömbjét
        buildButtonArray(buttonLabels, buttonCount, buttonCallback);

        // Elrendezzük a gombokat, ha vannak
        placeButtons();
    }

    /**
     * Dialóg destruktor
     * Töröljük a gombokat
     */
    ~MultiButtonDialog() {
        for (uint8_t i = 0; i < buttonCount; i++) {
            delete buttons[i];
        }
        delete[] buttons;
    }

    /**
     * @brief A dialóg kirajzolása a TFT képernyőn.
     *
     * Ez a metódus beállítja a szöveg színét, szöveg helyzetét, és megrajzolja az üzenetet és a gombokat a képernyőn.
     */
    virtual void drawDialog() override {

        // Ha már látszik, nem rajzoljuk ki újra
        if (visible) {
            return;
        }

        // Kirajzoljuk a dialógot
        PopupBase::drawDialog();

        // Gombok kirajzolása, ha vannak
        if (buttons) {
            for (uint8_t i = 0; i < buttonCount; i++) {
                buttons[i]->draw();
            }
        }
    }

    /// @brief Dialóg gombok touch eseményeinek kezelése
    /// @param touched Jelzi, hogy történt-e érintési esemény.
    /// @param tx Az érintési esemény x-koordinátája.
    /// @param ty Az érintési esemény y-koordinátája.
    virtual void handleTouch(bool touched, uint16_t tx, uint16_t ty) override {

        // Először meghívjuk a PopupBase érintéskezelőjét az 'X' detektálásához
        if (handleCloseButtonTouch(touched, tx, ty)) {
            return;
        }

        // Végigmegyünk az összes gombon és meghívjuk a touch kezeléseiket
        for (uint8_t i = 0; i < buttonCount; i++) {
            buttons[i]->handleTouch(touched, tx, ty);
        }
    }
};

#endif
