#ifndef __POPUPDIALOG_H
#define __POPUPDIALOG_H

#include "PopupBase.h"
#include "TftButton.h"

/**
 * @class PopUpDialog
 */
class PopUpDialog : public PopupBase {
private:
    TftButton *okButton;
    TftButton *cancelButton;
    ButtonCallback_t callback;

protected:
    /// @brief Párbeszédablak konstruktor
    /// @param pTft A TFT_eSPI példányra mutató referencia.
    /// @param w A párbeszédablak szélessége.
    /// @param h A párbeszédablak magassága.
    /// @param title A párbeszédablak címe (opcionális).
    /// @param message A párbeszédablak üzenete.
    /// @param callback Az OK gomb visszahívási függvénye.
    /// @param okText Az OK gomb felirata.
    /// @param cancelText A Cancel gomb felirata (opcionális).
    PopUpDialog(TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, ButtonCallback_t callback, const char *okText = "OK", const char *cancelText = nullptr)
        : PopupBase(tft, w, h, title, message), callback(callback), cancelButton(nullptr) {

        // Kiszedjük a legnagyobb gomb felirat szélességét (10-10 pixel a szélén)
        uint8_t okButtonWidth = tft.textWidth(okText) + DIALOG_DEFAULT_BUTTON_TEXT_PADDING_X;                          // OK gomb szöveg szélessége + padding a gomb széleihez
        uint8_t cancelButtonWidth = cancelText ? tft.textWidth(cancelText) + DIALOG_DEFAULT_BUTTON_TEXT_PADDING_X : 0; // Cancel gomb szöveg szélessége, ha van

        // Ha van Cancel gomb, akkor a két gomb közötti gap-et is figyelembe vesszük
        uint16_t totalButtonWidth = cancelButtonWidth > 0 ? okButtonWidth + cancelButtonWidth + DLG_BTN_GAP : okButtonWidth;
        uint16_t okX = x + (w - totalButtonWidth) / 2; // Az OK gomb X pozíciója -> a gombok kezdő X pozíciója

        // Gombok Y pozíció
        uint16_t buttonY = contentY + DLG_BTN_H;

        // OK gomb
        okButton = new TftButton(PopupBase::DIALOG_OK_BUTTON_ID, tft, okX, buttonY, okButtonWidth, DLG_BTN_H, okText, ButtonType::PUSHABLE, callback);

        // Cancel gomb, ha van
        if (cancelText) {
            uint16_t cancelX = okX + okButtonWidth + DLG_BTN_GAP; // A Cancel gomb X pozíciója
            cancelButton = new TftButton(PopupBase::DIALOG_CANCEL_BUTTON_ID, tft, cancelX, buttonY, cancelButtonWidth, DLG_BTN_H, cancelText, ButtonType::PUSHABLE);
        }
    }

public:
    /**
     * Dialóg destruktor
     */
    ~PopUpDialog() {
        delete okButton;
        if (cancelButton) {
            delete cancelButton;
        }
    }

    /// @brief A párbeszédablak komponenseinek megjelenítése
    virtual void drawDialog() override {

        // Ha már látszik, nem rajzoljuk ki újra
        if (visible) {
            return;
        }

        // Kirajzoljuk a dialógot
        PopupBase::drawDialog();

        // Kirajzoljuk az OK gombot
        okButton->draw();

        // Ha van Cancel gomb, akkor kirajzoljuk azt is
        if (cancelButton) {
            cancelButton->draw();
        }
    }

    /// @brief A párbeszédablak gombjainak érintési eseményeinek kezelése
    /// @param touched Jelzi, hogy történt-e érintési esemény.
    /// @param tx Az érintési esemény x-koordinátája.
    /// @param ty Az érintési esemény y-koordinátája.
    void handleTouch(bool touched, uint16_t tx, uint16_t ty) override {

        // Először meghívjuk a PopupBase érintéskezelőjét az 'X' detektálásához
        if (PopupBase::checkCloseButtonTouch(touched, tx, ty)) {

            // Megszereztük a callback függvényt, jól meghívjuk az "X" id-jével és a feliratával
            ButtonCallback_t callback = okButton->getCallback();
            if (callback) {
                callback(PopupBase::DIALOG_CLOSE_BUTTON_ID, PopupBase::DIALOG_CLOSE_BUTTON_LABEL, ButtonState::PUSHED);
            }
            return;
        }

        // OK gomb touch vizsgálat
        okButton->handleTouch(touched, tx, ty);

        // Cancel gomb touch vizsgálat, ha van Cancel gomb
        if (cancelButton != nullptr) {
            cancelButton->handleTouch(touched, tx, ty);
        }
    }

    /**
     * @brief Létrehoz egy új PopUpDialog példányt.
     *
     * @param pTft A TFT_eSPI objektumra mutató referencia
     * @param w A párbeszédablak szélessége.
     * @param h A párbeszédablak magassága.
     * @param title A cím szövegére mutató pointer, amely a flash memóriában van tárolva.
     * @param message Az üzenet szövegére mutató pointer, amely a flash memóriában van tárolva.
     * @param callback A függvény, amelyet a gomb megnyomásakor hívnak meg.
     * @param okText Az OK gomb szövege (alapértelmezett érték: "OK").
     * @param cancelText A Cancel gomb szövege (alapértelmezett érték: nullptr).
     * @return PopUpDialog* Pointer az újonnan létrehozott PopUpDialog példányra.
     */
    static PopUpDialog *createDialog(TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, ButtonCallback_t callback, const char *okText = "OK", const char *cancelText = nullptr) {
        return new PopUpDialog(tft, w, h, title, message, callback, okText, cancelText);
    }
};

#endif
