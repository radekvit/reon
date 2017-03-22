#include <reon_lexical_analyzer.h>
#include <reon_translation_grammar.h>
#include <fstream>
#include <functional>
#include <iostream>
using std::cout;

int main() {
  std::ifstream in("in");
  std::ofstream out("out");

  Translation t{ReonLexer{}, "ll", reonGrammar,
                OutputGenerator::default_output};
  t.run(in, out);

  return 0;
}