#include "HangulKeyboardLayout.h"

namespace fui = freeink::ui;

namespace keyboard_layouts {

namespace {

// Same half-key width units as the SDK tables and the URL layers.
constexpr uint8_t KEY_WIDTH = 2;
constexpr uint8_t WIDE_CONTROL_WIDTH = 3;

#define HK(label, value) \
  fui::KeyboardKey { label, label, fui::KeyKind::Normal, fui::StateNormal, value, KEY_WIDTH, true, nullptr }
#define HKA(label, value, alt) \
  fui::KeyboardKey { label, label, fui::KeyKind::Normal, fui::StateNormal, value, KEY_WIDTH, true, alt }
#define HKS(label, kind, value, units) \
  fui::KeyboardKey { label, nullptr, kind, fui::StateNormal, value, units *KEY_WIDTH, true, nullptr }
#define HK15(label, kind, value) \
  fui::KeyboardKey { label, nullptr, kind, fui::StateNormal, value, WIDE_CONTROL_WIDTH, true, nullptr }

// Digits stay primary on both layers (no case); symbols on long-press.
const fui::KeyboardKey NUM_ROW[] = {HKA("1", '1', "!"), HKA("2", '2', "@"), HKA("3", '3', "#"), HKA("4", '4', "$"),
                                    HKA("5", '5', "%"), HKA("6", '6', "^"), HKA("7", '7', "&"), HKA("8", '8', "*"),
                                    HKA("9", '9', "("), HKA("0", '0', ")")};

// Key values are the jamo code points (fit int16_t), unique across layers.
const fui::KeyboardKey ROW1[] = {HKA("ㅂ", 0x3142, "ㅃ"), HKA("ㅈ", 0x3148, "ㅉ"), HKA("ㄷ", 0x3137, "ㄸ"),
                                 HKA("ㄱ", 0x3131, "ㄲ"), HKA("ㅅ", 0x3145, "ㅆ"), HK("ㅛ", 0x315B),
                                 HK("ㅕ", 0x3155),        HK("ㅑ", 0x3151),        HKA("ㅐ", 0x3150, "ㅒ"),
                                 HKA("ㅔ", 0x3154, "ㅖ")};
const fui::KeyboardKey ROW2[] = {HK("ㅁ", 0x3141), HK("ㄴ", 0x3134), HK("ㅇ", 0x3147),
                                 HK("ㄹ", 0x3139), HK("ㅎ", 0x314E), HK("ㅗ", 0x3157),
                                 HK("ㅓ", 0x3153), HK("ㅏ", 0x314F), HK("ㅣ", 0x3163)};
const fui::KeyboardKey ROW3[] = {HK15(nullptr, fui::KeyKind::Shift, fui::QWERTY_KEY_SHIFT),
                                 HK("ㅋ", 0x314B),
                                 HK("ㅌ", 0x314C),
                                 HK("ㅊ", 0x314A),
                                 HK("ㅍ", 0x314D),
                                 HK("ㅠ", 0x3160),
                                 HK("ㅜ", 0x315C),
                                 HK("ㅡ", 0x3161),
                                 HK15("Del", fui::KeyKind::Delete, fui::QWERTY_KEY_BACKSPACE)};
const fui::KeyboardKey SHIFT_ROW1[] = {HKA("ㅃ", 0x3143, "ㅂ"), HKA("ㅉ", 0x3149, "ㅈ"), HKA("ㄸ", 0x3138, "ㄷ"),
                                       HKA("ㄲ", 0x3132, "ㄱ"), HKA("ㅆ", 0x3146, "ㅅ"), HK("ㅛ", 0x315B),
                                       HK("ㅕ", 0x3155),        HK("ㅑ", 0x3151),        HKA("ㅒ", 0x3152, "ㅐ"),
                                       HKA("ㅖ", 0x3156, "ㅔ")};
// Like the SDK's case-less layouts, the language key is always present:
// without Latin letters there must be a way back to a Wi-Fi password.
const fui::KeyboardKey BOTTOM[] = {
    HKS("?123", fui::KeyKind::Mode, fui::QWERTY_KEY_MODE, 2), HKS(nullptr, fui::KeyKind::Lang, fui::QWERTY_KEY_LANG, 1),
    HKS("Space", fui::KeyKind::Space, fui::QWERTY_KEY_SPACE, 5), HKS("OK", fui::KeyKind::Ok, fui::QWERTY_KEY_ENTER, 2)};

#undef HK
#undef HKA
#undef HKS
#undef HK15

const fui::KeyboardRow ROWS[] = {{NUM_ROW, 10, 0}, {ROW1, 10, 0}, {ROW2, 9, 0}, {ROW3, 9, 0}, {BOTTOM, 4, 0}};
const fui::KeyboardRow SHIFT_ROWS[] = {
    {NUM_ROW, 10, 0}, {SHIFT_ROW1, 10, 0}, {ROW2, 9, 0}, {ROW3, 9, 0}, {BOTTOM, 4, 0}};

const fui::KeyboardLayout LAYOUT{ROWS, 5};
const fui::KeyboardLayout SHIFT_LAYOUT{SHIFT_ROWS, 5};

}  // namespace

const fui::KeyboardLayout& hangulKeyboardLayout(const bool shifted) { return shifted ? SHIFT_LAYOUT : LAYOUT; }

}  // namespace keyboard_layouts
