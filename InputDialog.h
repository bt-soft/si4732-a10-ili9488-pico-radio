#ifndef __INPUTDIALOG_H
#define __INPUTDIALOG_H

#include "InputTextField.h"
#include "MultiButtonDialog.h"

class InputDialog : public MultiButtonDialog {
private:
    InputTextField *inputField;

private:
    void buttonCallback(const uint8_t id, const char *label, ButtonState_t state) {
        DEBUG("InputDialog::buttonCallback() - Id: %d, Label: '%s', állapot változás: %s\n", id, label, TftButton::decodeState(state));
    }

public:
    InputDialog(TFT_eSPI *pTft, uint16_t w, uint16_t h, const __FlashStringHelper *title)
        : MultiButtonDialog(pTft, w, h, title) {

        inputField = new InputTextField(pTft, x - 5, contentY + 5, w - 5, 20);

        // // Beállítjuk az ősnek a gombok számát
        buttonCount = 2;

        // // Dinamikusan foglalunk helyet a gomboknak az ősben
        // buttons = new TftButton *[buttonCount];
        // uint8_t id = PopupBase::DLG_MULTI_BTN_ID_START;
        // uint8_t arrIdx = 0;

        // // Létrehozzuk a gombokat
        // const char *buttonLabels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", ".", "<<"};
        //
        // for (uint8_t i = 0; i < buttonCount; i++) {
        //     buttons[arrIdx++] = new TftButton(id++, pTft, MULTI_BTN_W, MULTI_BTN_H, buttonLabels[i], ButtonType::PUSHABLE, SCRN_BTN_CB(InputDialog, buttonCallback, this));
        // }

        //*buttons[arrIdx++] = new TftButton(PopupBase::DIALOG_OK_BUTTON_ID, pTft, 60, DLG_BTN_H, "OK", ButtonType::PUSHABLE, SCRN_BTN_CB(InputDialog, buttonCallback, this));
        //*buttons[arrIdx++] = new TftButton(PopupBase::DIALOG_CANCEL_BUTTON_ID, pTft, 60, DLG_BTN_H, "Cancel", ButtonType::PUSHABLE, SCRN_BTN_CB(InputDialog, buttonCallback, this)); // Mégsem gomb
    }
    /**
     *
     */
    ~InputDialog() {
        delete inputField;
    }

    virtual void handleTouch(bool touched, uint16_t tx, uint16_t ty) override {
        // Először meghívhatjuk az ősosztály handleTouch metódusát
        MultiButtonDialog::handleTouch(touched, tx, ty);

        // A szövegbevitel mező kezelése
        if (inputField) {
            // inputField->handleTouch(touched, tx, ty);
        }

        // Esetleg a gombok kezelése is itt történhet
        for (uint8_t i = 0; i < buttonCount; i++) {
            buttons[i]->handleTouch(touched, tx, ty);
        }
    }
};
#endif // __INPUTDIALOG_H