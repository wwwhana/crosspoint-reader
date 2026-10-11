add_executable(HangulComposerTest
  HangulComposerTest.cpp
  ${REPO_ROOT}/lib/Utf8/HangulComposer.cpp
)

target_include_directories(HangulComposerTest PRIVATE
  ${REPO_ROOT}/lib/Utf8
)

target_link_libraries(HangulComposerTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(HangulComposerTest)
