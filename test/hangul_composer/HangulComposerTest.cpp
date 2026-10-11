#include <HangulComposer.h>
#include <gtest/gtest.h>

#include <string>

namespace {

// Drives the composer the way KeyboardEntryActivity does: every Edit is
// applied to the end of a text buffer.
struct Field {
  HangulComposer composer;
  std::string text;

  void type(const std::u32string& jamo) {
    for (const char32_t cp : jamo) {
      HangulComposer::Edit edit;
      ASSERT_TRUE(composer.feed(cp, edit)) << "not a jamo: " << static_cast<uint32_t>(cp);
      apply(edit);
    }
  }
  bool backspace() {
    HangulComposer::Edit edit;
    if (!composer.backspace(edit)) return false;
    apply(edit);
    return true;
  }
  void apply(const HangulComposer::Edit& edit) {
    ASSERT_LE(edit.eraseBytes, text.size());
    text.erase(text.size() - edit.eraseBytes);
    text += edit.insert;
  }
};

}  // namespace

TEST(HangulComposer, ComposesInitialMedialFinal) {
  Field f;
  f.type(U"ㅎㅏㄴ");
  EXPECT_EQ(f.text, "한");
  EXPECT_TRUE(f.composer.composing());
}

TEST(HangulComposer, ShowsTheLoneJamoWhileComposing) {
  Field f;
  f.type(U"ㅎ");
  EXPECT_EQ(f.text, "ㅎ");
  f.type(U"ㅏ");
  EXPECT_EQ(f.text, "하");
}

TEST(HangulComposer, FinalConsonantCarriesToTheNextSyllableBeforeAVowel) {
  Field f;
  f.type(U"ㄱㅏㄱㅏ");
  EXPECT_EQ(f.text, "가가");
  Field g;
  g.type(U"ㅎㅏㄴㄱㅡㄹ");
  EXPECT_EQ(g.text, "한글");
}

TEST(HangulComposer, CompoundFinalSplitsBeforeAVowel) {
  Field f;
  f.type(U"ㄱㅏㅂㅅㅣ");  // 값 + ㅣ -> 갑시
  EXPECT_EQ(f.text, "갑시");
  Field g;
  g.type(U"ㄷㅏㄹㄱ");
  EXPECT_EQ(g.text, "닭");
}

TEST(HangulComposer, CompoundVowels) {
  Field f;
  f.type(U"ㄱㅗㅏ");
  EXPECT_EQ(f.text, "과");
  Field g;
  g.type(U"ㅇㅜㅣ");
  EXPECT_EQ(g.text, "위");
  Field h;
  h.type(U"ㅇㅡㅣㅅㅏ");
  EXPECT_EQ(h.text, "의사");
}

TEST(HangulComposer, ConsonantsThatCannotBeFinalsStartANewSyllable) {
  Field f;
  f.type(U"ㄱㅏㅃㅏ");  // ㅃ is never a final
  EXPECT_EQ(f.text, "가빠");
}

TEST(HangulComposer, DoubleConsonantKeys) {
  Field f;
  f.type(U"ㄲㅗㅊ");
  EXPECT_EQ(f.text, "꽃");
  Field g;
  g.type(U"ㅇㅣㅆㄷㅏ");
  EXPECT_EQ(g.text, "있다");
}

TEST(HangulComposer, VowelAfterACompletedSyllableStartsStandalone) {
  Field f;
  f.type(U"ㄱㅏㅗ");
  EXPECT_EQ(f.text, "가ㅗ");
  f.type(U"ㅏ");
  EXPECT_EQ(f.text, "가ㅘ");
}

TEST(HangulComposer, BackspaceRemovesOneJamoAtATime) {
  Field f;
  f.type(U"ㄷㅏㄹㄱ");
  EXPECT_EQ(f.text, "닭");
  EXPECT_TRUE(f.backspace());
  EXPECT_EQ(f.text, "달");
  EXPECT_TRUE(f.backspace());
  EXPECT_EQ(f.text, "다");
  EXPECT_TRUE(f.backspace());
  EXPECT_EQ(f.text, "ㄷ");
  EXPECT_TRUE(f.backspace());
  EXPECT_EQ(f.text, "");
  EXPECT_FALSE(f.composer.composing());
  EXPECT_FALSE(f.backspace());
}

TEST(HangulComposer, BackspaceSplitsCompoundVowel) {
  Field f;
  f.type(U"ㄱㅗㅏ");
  EXPECT_TRUE(f.backspace());
  EXPECT_EQ(f.text, "고");
}

TEST(HangulComposer, NonJamoIsRejectedAndLeavesStateAlone) {
  HangulComposer composer;
  HangulComposer::Edit edit;
  EXPECT_FALSE(composer.feed(U'a', edit));
  EXPECT_FALSE(composer.feed(U' ', edit));
  EXPECT_FALSE(composer.composing());
  ASSERT_TRUE(composer.feed(U'ㅎ', edit));
  EXPECT_FALSE(composer.feed(U'a', edit));
  EXPECT_TRUE(composer.composing());
  composer.reset();
  EXPECT_FALSE(composer.composing());
  EXPECT_EQ(composer.composingBytes(), 0u);
}

TEST(HangulComposer, ReportsTheComposingCodepoint) {
  HangulComposer composer;
  HangulComposer::Edit edit;
  composer.feed(U'ㅎ', edit);
  EXPECT_EQ(composer.composingCodepoint(), 0x314Eu);
  composer.feed(U'ㅏ', edit);
  EXPECT_EQ(composer.composingCodepoint(), 0xD558u);  // 하
  EXPECT_EQ(composer.composingBytes(), 3u);
  char text[4];
  composer.composingText(text);
  EXPECT_STREQ(text, "하");
  composer.reset();
  composer.composingText(text);
  EXPECT_STREQ(text, "");
}

TEST(HangulComposer, EveryTwoSetKeyTypesSomething) {
  // All 33 jamo the layout can emit, each on a fresh composer.
  for (char32_t cp = 0x3131; cp <= 0x3163; ++cp) {
    HangulComposer composer;
    HangulComposer::Edit edit;
    ASSERT_TRUE(composer.feed(cp, edit)) << std::hex << static_cast<uint32_t>(cp);
    EXPECT_EQ(edit.eraseBytes, 0);
    EXPECT_EQ(std::string(edit.insert).size(), 3u) << std::hex << static_cast<uint32_t>(cp);
  }
}
