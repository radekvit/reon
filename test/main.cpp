#include <reon_lexical_analyzer.h>
#include <reon_output_generator.h>
#include <reon_translation_grammar.h>
#include "test_case.h"

#include <iostream>

using reon_test::TestCase;


int main() {
  Translation reon2Py{ReonLexer{}, "ll", reonGrammar, ReonOutput{}};
  
  vector<TestCase> tests{
      {"1: ipv4", reon2Py, "in/test01_in", "in/test01_expected"},
      {"2: all features", reon2Py, "in/test02_in", "in/test02_expected"},
  };

  for (auto &test : tests) {
    if (!test.run()) {
      std::cerr << "Test " << test.name() << " failed.\n";
      std::cerr << "Expected:\n" << test.expected();
      std::cerr << "\n---\nOutput:\n" << test.result() << "\n---\n";
      return 1;
    }
  }
  return 0;
}