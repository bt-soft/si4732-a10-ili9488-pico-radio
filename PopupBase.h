#ifndef __POPUPBASE_H
#define __POPUPBASE_H

#include <TFT_eSPI.h>
#include <stdlib.h>

#define DLG_BTN_GAP 10                                // A gombok közötti térköz pixelekben
#define DLG_BTN_H 30                                  // Gomb(ok) magassága a dialógusban
#define DIALOG_DEFAULT_BUTTON_TEXT_PADDING_X (2 * 15) // 15-15px X padding a gombok szövegépen

/**
 * @class PopupBase
 * @brief Alaposztály felugró dialógusok létrehozásához TFT kijelzőn.
 *
 * Ez az osztály biztosítja az alapvető funkcionalitást felugró dialógusok megjelenítéséhez
 * és kezeléséhez TFT kijelzőn a TFT_eSPI könyvtár használatával. Kezeli a dialógus megjelenítését,
 * beleértve a címet és a bezárás gombot, valamint az érintési eseményeket a dialógus bezárásához.
 *
 * @note Ez az osztály származtatásra szolgál specifikus típusú dialógusok létrehozásához.
 */
class PopupBase {

private:
    static constexpr uint8_t DLG_CLOSE_BTN_SIZE = 20; // "X" gomb mérete
    static constexpr uint8_t DLG_HEADER_H = 30;       // Fejléc magassága

    const __FlashStringHelper *title;   // Flash memóriában tárolt title szöveg
    const __FlashStringHelper *message; // Flash memóriában tárolt dialóg szöveg
    uint16_t y;                         // A leszármazottak nem láthatják az y pozíciót, csak a contentY alapján pozíciónálhatnak
    uint16_t messageY;
    uint16_t closeButtonX, closeButtonY; // X gomb pozíciója

protected:
    TFT_eSPI *pTft;
    uint16_t x, w, h;  // a dialógus koordinátái az y érték nélkül
    uint16_t contentY; // Ezt az y értéket láthatják a leszármazottak
    bool visible;

    /**
     * @brief PopupBase konstruktora.
     *
     * Inicializál egy új PopupBase osztály példányt a megadott paraméterekkel.
     *
     * @param tft A TFT_eSPI példányra mutató pointer.
     * @param w A dialógus szélessége.
     * @param h A dialógus magassága.
     * @param title A dialógus címe (opcionális).
     */
    PopupBase(TFT_eSPI *tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message = nullptr)
        : pTft(tft), w(w), h(h), title(title), message(message), visible(false) {

        // Dialóg bal felső sarkának kiszámítása a képernyő középre igzaításához
        x = (tft->width() - w) / 2;
        y = (tft->height() - h) / 2;

        messageY = y + (title ? DLG_HEADER_H + 15 : 5);      // Az üzenet a fejléc utánkezdődjön, ha van fejléc
        contentY = messageY + (message != nullptr ? 15 : 0); // A belső tér az üzenet után kezdődjön, ha van üzenet
    }

public:
    /**
     * @brief PopupBase destruktora.
     */
    virtual ~PopupBase() {
    }

    /**
     * @brief Megjeleníti a dialógust.
     *
     * Ez a metódus megjeleníti a dialógust a TFT kijelzőn, beleértve a címet és a bezárás gombot.
     * Emellett elmenti a dialógus által takart háttérterületet.
     */
    virtual void drawDialog() {

        // Kirajzoljuk a dialógot
        pTft->fillRect(x, y, w, h, TFT_DARKGREY); // háttér

        // Title kiírása
        if (title != nullptr) {
            // Fejléc háttér kitöltése
            pTft->fillRect(x, y, w, DLG_HEADER_H, TFT_NAVY);

            // Title kiírása
            pTft->setTextColor(TFT_WHITE);
            pTft->setTextDatum(TL_DATUM);                                                     // Bal felső sarokhoz igazítva
            pTft->drawString(title, x + 10, y + 5 + (DLG_HEADER_H - pTft->fontHeight()) / 2); // Bal oldali margó 10px

            // Fejléc vonala
            pTft->drawFastHLine(x, y + DLG_HEADER_H, w, TFT_WHITE);
        }

        // Dialógus kerete
        pTft->drawRect(x, y, w, h, TFT_WHITE); // keret

        // A header "X" gomb kirajzolása
        closeButtonX = x + w - DLG_CLOSE_BTN_SIZE - 5; // Az "X" gomb pozíciója a title jobb oldalán, kis margóval a jobb szélre
        closeButtonY = y + 5;                          // Fejléc tetejéhez igazítva
        pTft->setTextColor(TFT_WHITE);
        pTft->setTextDatum(MC_DATUM); // Középre igazítva az "X"-et
        pTft->drawString(F(DIALOG_CLOSE_BUTTON_LABEL), closeButtonX + DLG_CLOSE_BTN_SIZE / 2, closeButtonY + DLG_CLOSE_BTN_SIZE / 2);

        // Üzenet kirajzolása, ha van üzenet
        if (message != nullptr) {
            pTft->setTextColor(TFT_WHITE);
            pTft->setTextDatum(MC_DATUM);
            pTft->drawString(message, x + w / 2, messageY);
        }
    }

    /// @brief A párbeszédablak gombjainak érintési eseményeinek kezelése, a leszármazott implemnetálja
    /// @param touched Jelzi, hogy történt-e érintési esemény.
    /// @param tx Az érintési esemény x-koordinátája.
    /// @param ty Az érintési esemény y-koordinátája.
    virtual void handleTouch(bool touched, uint16_t tx, uint16_t ty) {}

protected:
    /**
     * @brief Ez a metódus ellenőrzi, hogy a bezárás gombot megérintették-e?
     *
     * @param touched Jelzi, hogy történt-e érintési esemény.
     * @param tx Az érintési esemény x-koordinátája.
     * @param ty Az érintési esemény y-koordinátája.
     * @returns true, ha megnyomták az X gombot a fejlécben
     */
    bool checkCloseButtonTouch(bool touched, uint16_t tx, uint16_t ty) {

        if (touched) {
            if (tx >= closeButtonX && tx <= closeButtonX + DLG_CLOSE_BTN_SIZE &&
                ty >= closeButtonY && ty <= closeButtonY + DLG_CLOSE_BTN_SIZE) {
                return true;
            }
        }

        return false;
    }

public:
    // Gomb konstansok
    static constexpr uint8_t DIALOG_UNDEFINED_BUTTON_ID = 0; // 'üres'/'nincs' megnyomott button ID
    static constexpr uint8_t DIALOG_OK_BUTTON_ID = 1;        // OK gomb ID-je
    static constexpr uint8_t DIALOG_CANCEL_BUTTON_ID = 2;    // Cancel gomb ID-je
    static constexpr uint8_t DLG_MULTI_BTN_ID_START = 10;    // A multi buttonok kezdő ID-je

    //'X' gomb konstansok
    static constexpr const char *DIALOG_CLOSE_BUTTON_LABEL = "X"; // Jobb felső sarok bezáró gomb felirata
    static constexpr uint8_t DIALOG_CLOSE_BUTTON_ID = 254;        // Jobb felső sarok bezáró gomb ID-je
};

#endif
