#ifndef __MULTIBUTTONDIALOG_H
#define __MULTIBUTTONDIALOG_H

#include "PopupBase.h"
#include "TftButton.h"

/**
 * @class MultiButtonDialog
 * @brief Több gombos párbeszédpanel TFT képernyőn.
 *
 * Ez az osztály egy olyan párbeszédpanelt képvisel, amely üzenetet és több gombot jelenít meg egy TFT képernyőn.
 * A PopupBase osztályból származik.
 */
class MultiButtonDialog : public PopupBase {

private:
    TftButton **buttons; ///< A megjelenítendő gombok mutatóinak tömbje.
    uint8_t buttonCount; ///< A párbeszédpanelen lévő gombok száma.

    /**
     * @brief A párbeszédpanel megrajzolása a TFT képernyőn.
     *
     * Ez a metódus beállítja a szöveg színét, szöveg helyzetét, és megrajzolja az üzenetet és a gombokat a képernyőn.
     */
    virtual void drawDialog() override {

        // Kirajzoljuk a dialógot
        PopupBase::drawDialog();

        // Gombok kirajzolása
        for (uint8_t i = 0; i < buttonCount; i++) {
            buttons[i]->draw();
        }
    }

protected:
    /**
     * @brief MultiButtonDialog objektum létrehozása.
     *
     * @param pTft Pointer a TFT_eSPI objektumra.
     * @param w A párbeszédpanel szélessége.
     * @param h A párbeszédpanel magassága.
     * @param title A dialógus címe (opcionális).
     * @param message Az üzenet, amely megjelenik a párbeszédpanelen.
     * @param buttons A gombok mutatóinak tömbje.
     * @param buttonCount A gombok száma.
     */
    MultiButtonDialog(TFT_eSPI *pTft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, TftButton **buttons, uint8_t buttonCount)
        : PopupBase(pTft, w, h, title, message), buttons(buttons), buttonCount(buttonCount) {

        uint16_t maxRowWidth = w - 20; // Max szélesség, kis margóval
        uint16_t buttonHeight = DIALOG_DEFAULT_BUTTON_HEIGHT;
        uint8_t buttonsPerRow = 1;
        uint16_t totalWidth = 0;

        // Kiszámoljuk, hogy hány gomb fér el egy sorban
        for (uint8_t i = 0; i < buttonCount; i++) {
            totalWidth += buttons[i]->getWidth() + DIALOG_DEFAULT_BUTTONS_GAP;
            if (totalWidth - DIALOG_DEFAULT_BUTTONS_GAP > maxRowWidth) {
                break; // Ha túlcsordul, az előző szám volt a max
            }
            buttonsPerRow++;
        }
        buttonsPerRow--; // Az utolsó túlcsordulás miatt csökkentjük

        // Kiszámoljuk a sorok számát
        uint8_t rowCount = (buttonCount + buttonsPerRow - 1) / buttonsPerRow; // Felkerekítés
        uint16_t totalHeight = rowCount * buttonHeight + (rowCount - 1) * DIALOG_DEFAULT_BUTTONS_GAP;

        // Button kezdő y pozíció
        uint16_t startY = contentY;

        // Gombok pozicionálása több sorban
        uint8_t row = 0, col = 0;
        uint16_t startX = 0;
        for (uint8_t i = 0; i < buttonCount; i++) {
            // Sor elején újraszámoljuk a kezdő X-et (középre igazítás)
            if (col == 0) {
                uint16_t rowWidth = 0;
                uint8_t itemsInRow = min(buttonsPerRow, buttonCount - i);
                for (uint8_t j = 0; j < itemsInRow; j++) {
                    rowWidth += buttons[i + j]->getWidth();
                }
                rowWidth += (itemsInRow - 1) * DIALOG_DEFAULT_BUTTONS_GAP;
                startX = x + (w - rowWidth) / 2;
            }

            // Gomb elhelyezése
            buttons[i]->setPosition(startX, startY);
            startX += buttons[i]->getWidth() + DIALOG_DEFAULT_BUTTONS_GAP;
            col++;

            // Ha betelt egy sor, új sorba lépünk
            if (col >= buttonsPerRow) {
                col = 0;
                row++;
                startY += buttonHeight + DIALOG_DEFAULT_BUTTONS_GAP;
            }
        }

        // Megjelenítjük a dialógust
        drawDialog();
    }

public:
    /// @brief Dialóg destruktor
    ~MultiButtonDialog() {
        for (uint8_t i = 0; i < buttonCount; i++) {
            delete buttons[i];
        }
    }

    /// @brief Dialóg gombok touch eseményeinek kezelése
    /// @param touched Jelzi, hogy történt-e érintési esemény.
    /// @param tx Az érintési esemény x-koordinátája.
    /// @param ty Az érintési esemény y-koordinátája.
    void handleTouch(bool touched, uint16_t tx, uint16_t ty) override {

        // Először meghívjuk a PopupBase érintéskezelőjét az 'X' detektálásához
        if (PopupBase::checkCloseButtonTouch(touched, tx, ty)) {

            // Megszerezzük valamelyik gombtól a callback függvényt, és meghívjuk az "X" gombfelirattal
            ButtonCallback callback = nullptr;

            // Végigmegyünk az összes gombon és megkeressük az első nem NULL callback függvényt
            for (uint8_t i = 0; i < buttonCount; i++) {
                callback = buttons[i]->getCallback();
                if (callback) {
                    // Megszereztük a callback függvényt, jól meghívjuk az "X" gombfelirattal
                    callback(DIALOG_CLOSE_BUTTON_LABEL, ButtonState::PUSHED);
                    return;
                }
            }
        }

        // Végigmegyünk az összes gombon és meghívjuk a touch kezeléseiket
        for (uint8_t i = 0; i < buttonCount; i++) {
            buttons[i]->handleTouch(touched, tx, ty);
        }
    }

    /**
     * @brief MultiButtonDialog objektum létrehozása.
     *
     * @param dialogPointer Az új dialóg pointere
     * @param pTft Pointer a TFT_eSPI objektumra.
     * @param w A párbeszédpanel szélessége.
     * @param h A párbeszédpanel magassága.
     * @param title A dialógus címe (opcionális).
     * @param message Az üzenet, amely megjelenik a párbeszédpanelen.
     * @param buttons A gombok mutatóinak tömbje.
     * @param buttonCount A gombok száma.
     */
    static MultiButtonDialog *createDialog(TFT_eSPI *pTft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, TftButton **buttons, uint8_t buttonCount) {
        return new MultiButtonDialog(pTft, w, h, title, message, buttons, buttonCount);
    }

    /**
     * @brief Üzenet nélküli MultiButtonDialog objektum létrehozása.
     *
     * @param dialogPointer Az új dialóg pointere
     * @param pTft Pointer a TFT_eSPI objektumra.
     * @param w A párbeszédpanel szélessége.
     * @param h A párbeszédpanel magassága.
     * @param title A dialógus címe (opcionális).
     * @param buttons A gombok mutatóinak tömbje.
     * @param buttonCount A gombok száma.
     */
    static MultiButtonDialog *createDialog(TFT_eSPI *pTft, uint16_t w, uint16_t h, const __FlashStringHelper *title, TftButton **buttons, uint8_t buttonCount) {
        return new MultiButtonDialog(pTft, w, h, title, nullptr, buttons, buttonCount);
    }
};

#endif
