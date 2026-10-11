#include "HangulComposer.h"

namespace {

constexpr uint32_t SYLLABLE_BASE = 0xAC00;
constexpr uint32_t COMPAT_JAMO_FIRST = 0x3131;
constexpr uint32_t COMPAT_JAMO_LAST = 0x3163;
constexpr int VOWEL_COUNT = 21;
constexpr int TAIL_COUNT = 28;

// Compatibility jamo U+3131..U+314E in code point order, as initial (L) and
// final (T) indices of the syllable formula. -1: not usable in that position.
constexpr int8_t LEAD_OF_COMPAT[30] = {0,  1, -1, 2, -1, -1, 3,  4,  5,  -1, -1, -1, -1, -1, -1,
                                       -1, 6, 7,  8, -1, 9,  10, 11, 12, 13, 14, 15, 16, 17, 18};
constexpr int8_t TAIL_OF_COMPAT[30] = {1,  2,  3,  4,  5,  6,  7,  -1, 8,  9,  10, 11, 12, 13, 14,
                                       15, 16, 17, -1, 18, 19, 20, 21, 22, -1, 23, 24, 25, 26, 27};
// Initial / final indices back to compatibility jamo offsets from U+3131.
constexpr uint8_t COMPAT_OF_LEAD[19] = {0, 1, 3, 6, 7, 8, 16, 17, 18, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
constexpr uint8_t COMPAT_OF_TAIL[TAIL_COUNT] = {0,  0,  1,  2,  3,  4,  5,  6,  8,  9,  10, 11, 12, 13,
                                                14, 15, 16, 17, 19, 20, 21, 22, 23, 25, 26, 27, 28, 29};

// Compound vowels: {first, second, result} as medial indices.
struct VowelPair {
  int8_t first, second, result;
};
constexpr VowelPair VOWEL_PAIRS[] = {
    {8, 0, 9},     // ㅗ+ㅏ=ㅘ
    {8, 1, 10},    // ㅗ+ㅐ=ㅙ
    {8, 20, 11},   // ㅗ+ㅣ=ㅚ
    {13, 4, 14},   // ㅜ+ㅓ=ㅝ
    {13, 5, 15},   // ㅜ+ㅔ=ㅞ
    {13, 20, 16},  // ㅜ+ㅣ=ㅟ
    {18, 20, 19},  // ㅡ+ㅣ=ㅢ
};
// Compound finals: {first, second, result} as final indices.
struct TailPair {
  int8_t first, second, result;
};
constexpr TailPair TAIL_PAIRS[] = {
    {1, 19, 3},    // ㄱ+ㅅ=ㄳ
    {4, 22, 5},    // ㄴ+ㅈ=ㄵ
    {4, 27, 6},    // ㄴ+ㅎ=ㄶ
    {8, 1, 9},     // ㄹ+ㄱ=ㄺ
    {8, 16, 10},   // ㄹ+ㅁ=ㄻ
    {8, 17, 11},   // ㄹ+ㅂ=ㄼ
    {8, 19, 12},   // ㄹ+ㅅ=ㄽ
    {8, 25, 13},   // ㄹ+ㅌ=ㄾ
    {8, 26, 14},   // ㄹ+ㅍ=ㄿ
    {8, 27, 15},   // ㄹ+ㅎ=ㅀ
    {17, 19, 18},  // ㅂ+ㅅ=ㅄ
};

int8_t compoundVowel(const int8_t first, const int8_t second) {
  for (const auto& p : VOWEL_PAIRS) {
    if (p.first == first && p.second == second) return p.result;
  }
  return -1;
}

// Splits a compound medial into its parts; returns false for a simple vowel.
bool splitVowel(const int8_t vowel, int8_t& first, int8_t& second) {
  for (const auto& p : VOWEL_PAIRS) {
    if (p.result == vowel) {
      first = p.first;
      second = p.second;
      return true;
    }
  }
  return false;
}

int8_t compoundTail(const int8_t first, const int8_t second) {
  for (const auto& p : TAIL_PAIRS) {
    if (p.first == first && p.second == second) return p.result;
  }
  return -1;
}

bool splitTail(const int8_t tail, int8_t& first, int8_t& second) {
  for (const auto& p : TAIL_PAIRS) {
    if (p.result == tail) {
      first = p.first;
      second = p.second;
      return true;
    }
  }
  return false;
}

size_t encodeUtf8(const uint32_t cp, char* out) {
  if (cp == 0) return 0;
  if (cp < 0x80) {
    out[0] = static_cast<char>(cp);
    return 1;
  }
  if (cp < 0x800) {
    out[0] = static_cast<char>(0xC0 | (cp >> 6));
    out[1] = static_cast<char>(0x80 | (cp & 0x3F));
    return 2;
  }
  out[0] = static_cast<char>(0xE0 | (cp >> 12));
  out[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
  out[2] = static_cast<char>(0x80 | (cp & 0x3F));
  return 3;
}

}  // namespace

bool HangulComposer::isJamo(const uint32_t cp) { return cp >= COMPAT_JAMO_FIRST && cp <= COMPAT_JAMO_LAST; }

uint32_t HangulComposer::currentCodepoint() const {
  if (lead_ >= 0 && vowel_ >= 0) {
    const uint32_t tail = tail_ > 0 ? static_cast<uint32_t>(tail_) : 0;
    return SYLLABLE_BASE + (static_cast<uint32_t>(lead_) * VOWEL_COUNT + static_cast<uint32_t>(vowel_)) * TAIL_COUNT +
           tail;
  }
  if (lead_ >= 0) return COMPAT_JAMO_FIRST + COMPAT_OF_LEAD[lead_];
  if (vowel_ >= 0) return COMPAT_JAMO_FIRST + 30 + static_cast<uint32_t>(vowel_);
  return 0;
}

uint32_t HangulComposer::composingCodepoint() const { return currentCodepoint(); }

void HangulComposer::composingText(char (&out)[4]) const { out[encodeUtf8(currentCodepoint(), out)] = '\0'; }

size_t HangulComposer::composingBytes() const { return composing() ? 3 : 0; }

void HangulComposer::writeEdit(Edit& out, const uint8_t eraseBytes, const uint32_t committed,
                               const uint32_t current) const {
  out.eraseBytes = eraseBytes;
  size_t n = encodeUtf8(committed, out.insert);
  n += encodeUtf8(current, out.insert + n);
  out.insert[n] = '\0';
}

bool HangulComposer::feed(const uint32_t cp, Edit& out) {
  if (!isJamo(cp)) return false;
  const uint8_t erase = static_cast<uint8_t>(composingBytes());
  const uint32_t before = currentCodepoint();
  const int compat = static_cast<int>(cp - COMPAT_JAMO_FIRST);

  if (compat >= 30) {
    const int8_t v = static_cast<int8_t>(compat - 30);
    if (lead_ >= 0 && vowel_ < 0) {
      vowel_ = v;  // ㄱ + ㅏ -> 가
    } else if (lead_ >= 0 && tail_ < 0) {
      const int8_t compound = compoundVowel(vowel_, v);
      if (compound >= 0) {
        vowel_ = compound;  // 고 + ㅏ -> 과
      } else {
        lead_ = -1;  // 가 + ㅗ -> 가 committed, ㅗ starts a new syllable
        vowel_ = v;
        tail_ = -1;
        writeEdit(out, erase, before, currentCodepoint());
        return true;
      }
    } else if (lead_ >= 0) {
      // Final consonant moves to the next syllable: 값 + ㅣ -> 갑시, 각 + ㅏ -> 가카.
      int8_t keep, carry;
      if (splitTail(tail_, keep, carry)) {
        tail_ = keep;
      } else {
        carry = tail_;
        tail_ = -1;
      }
      const uint32_t committed = currentCodepoint();
      lead_ = LEAD_OF_COMPAT[COMPAT_OF_TAIL[carry]];
      vowel_ = v;
      tail_ = -1;
      writeEdit(out, erase, committed, currentCodepoint());
      return true;
    } else if (vowel_ >= 0) {
      const int8_t compound = compoundVowel(vowel_, v);
      if (compound >= 0) {
        vowel_ = compound;  // standalone ㅗ + ㅏ -> ㅘ
      } else {
        writeEdit(out, erase, before, cp);
        vowel_ = v;
        return true;
      }
    } else {
      vowel_ = v;  // standalone vowel
    }
    writeEdit(out, erase, 0, currentCodepoint());
    return true;
  }

  const int8_t lead = LEAD_OF_COMPAT[compat];
  const int8_t tail = TAIL_OF_COMPAT[compat];
  if (lead_ >= 0 && vowel_ >= 0 && tail_ < 0 && tail > 0) {
    tail_ = tail;  // 가 + ㄱ -> 각
  } else if (lead_ >= 0 && vowel_ >= 0 && tail_ > 0 && tail > 0 && compoundTail(tail_, tail) > 0) {
    tail_ = compoundTail(tail_, tail);  // 각 + ㅅ -> 갃
  } else if (lead >= 0) {
    // Commit whatever is composing and start a new syllable with this initial.
    writeEdit(out, erase, before, cp);
    lead_ = lead;
    vowel_ = -1;
    tail_ = -1;
    return true;
  } else {
    // A cluster jamo key (ㄳ etc.) that cannot start a syllable: emit as is.
    writeEdit(out, erase, before, cp);
    reset();
    return true;
  }
  writeEdit(out, erase, 0, currentCodepoint());
  return true;
}

bool HangulComposer::backspace(Edit& out) {
  if (!composing()) return false;
  const uint8_t erase = static_cast<uint8_t>(composingBytes());
  int8_t first, second;
  if (tail_ > 0) {
    tail_ = splitTail(tail_, first, second) ? first : -1;
  } else if (vowel_ >= 0 && splitVowel(vowel_, first, second)) {
    vowel_ = first;
  } else if (vowel_ >= 0) {
    vowel_ = -1;
  } else {
    lead_ = -1;
  }
  writeEdit(out, erase, 0, currentCodepoint());
  return true;
}
