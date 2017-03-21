#include <reon_lexical_analyzer.h>
#include <reon_translation_grammar.h>
#include <fstream>
#include <functional>
#include <iostream>
using std::cout;

int main() {
  std::ifstream in("in");
  std::ofstream out("out");
  ReonLexer reonLexer;
  LexicalAnalyzer la{reonLexer};
  OutputGenerator og;

  LLTranslationControl tc{};

  Translation t{la, tc, reonGrammar, og};
  t.run(in, out);

  return 0;
}