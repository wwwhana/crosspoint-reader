#pragma once

#include <FreeInkUI.h>

namespace keyboard_layouts {

// Korean two-set (두벌식, KS X 5002) letter layer over the SDK's keyboard
// structs, with the digit row the other letter layers carry. The SDK has no
// Hangul table, so this one lives in the app. Keys emit compatibility jamo
// (U+3131..U+3163); HangulComposer turns them into syllables.
// Shift reaches the doubled consonants and ㅒ/ㅖ; long-press does the same.
const freeink::ui::KeyboardLayout& hangulKeyboardLayout(bool shifted);

// Every jamo label and long-press hint on both layers, as one UTF-8 string.
// The built-in UI fonts have no Hangul, so labels draw through the SD-font
// fallback; prewarming this once per repaint replaces a per-label SD rebuild.
inline constexpr const char* HANGUL_KEYBOARD_GLYPHS =
    "ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎㅏㅐㅑㅒㅓㅔㅕㅖㅗㅛㅜㅠㅡㅣ";

}  // namespace keyboard_layouts
