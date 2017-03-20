#include <reon_lexical_analyzer.h>
#include <fstream>
#include <functional>
#include <iostream>
using std::cout;

int main() {
  std::ifstream s("in");
  ReonLexer rl;
  std::function<Symbol(std::istream &)> fp;
  fp = rl;
  Token t(Token::EOI());
  LexicalAnalyzer la{s, rl};
  while ((t = la.get_token()) != Token::EOI()) {
    cout << t.name() << "." << t.attribute() << "\n";
  }
  return 0;
}