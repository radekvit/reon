#include <reon_lexical_analyzer.h>
#include <reon_output_generator.h>
#include <reon_translation_grammar.h>
#include <fstream>
#include <functional>
#include <iostream>
using std::cout;

int main() {
  std::ifstream in("in");

  Translation t{ReonLexer{}, "ll", reonGrammar, ReonOutput()};
  t.run(in, std::cout);

  return 0;
}