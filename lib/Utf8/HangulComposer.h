#pragma once

#include <cstddef>
#include <cstdint>

// Composes Hangul syllables from two-set (두벌식) keyboard input, one
// compatibility jamo (U+3131..U+3163) at a time, following the standard
// automaton: initial + medial [+ final], compound vowels (ㅗ+ㅏ=ㅘ), compound
// finals (ㄱ+ㅅ=ㄳ), and final-consonant carry-over when a vowel follows
// (값+ㅣ -> 갑시). Backspace removes one jamo from the composing syllable.
//
// The composer tracks only the syllable under construction; the caller owns
// the text buffer and applies each Edit to the end of it: erase `eraseBytes`
// bytes before the cursor, then insert `insert`. Fixed-size state, no heap.
class HangulComposer {
 public:
  struct Edit {
    uint8_t eraseBytes = 0;  // bytes of the previous composing text to remove
    char insert[7] = {};     // up to two UTF-8 Hangul code points, NUL-terminated
  };

  // True for the jamo keys a two-set layout produces (U+3131..U+3163).
  static bool isJamo(uint32_t cp);

  // Feed one jamo. Returns false (and leaves state untouched) for any other
  // code point: the caller commits and inserts it as plain text.
  bool feed(uint32_t cp, Edit& out);

  // Remove the last jamo of the composing syllable. Returns false when nothing
  // is composing, so the caller deletes a whole code point instead.
  bool backspace(Edit& out);

  // Forget the composing syllable (cursor moved, layout switched, submitted).
  void reset() { lead_ = vowel_ = tail_ = -1; }
  bool composing() const { return lead_ >= 0 || vowel_ >= 0; }

  // Byte length of the composing text currently in the buffer; 0 when idle.
  // Lets the caller check the buffer still ends with what the composer wrote.
  size_t composingBytes() const;
  // Code point of the composing text (jamo or syllable); 0 when idle.
  uint32_t composingCodepoint() const;
  // The composing text as NUL-terminated UTF-8; empty when idle.
  void composingText(char (&out)[4]) const;

 private:
  int8_t lead_ = -1;   // initial consonant index 0..18
  int8_t vowel_ = -1;  // medial vowel index 0..20
  int8_t tail_ = -1;   // final consonant index 1..27 (-1 = none)

  uint32_t currentCodepoint() const;
  void writeEdit(Edit& out, uint8_t eraseBytes, uint32_t committed, uint32_t current) const;
};
