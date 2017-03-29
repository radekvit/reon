#include <reon_lexical_analyzer.h>
#include <reon_output_generator.h>
#include <reon_translation_grammar.h>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>

// using declarations
using std::cin;
using std::cout;
using std::cerr;

// error codes
const int RUNTIME_ERROR = 1;
const int INVALID_ARGUMENT = 2;
const int TRANSLATION_ENGINE_ERROR = 3;
const int LEXICAL_ERROR = 5;
const int SYNTAX_ERROR = 6;
const int SEMANTIC_ERROR = 7;
const int UNKNOWN_EXCEPTION = 666;

void print_help();

void translation(std::istream &input, std::ostream &output) {
  // reon translation unit, LL table driven translation
  Translation t{ReonLexer{}, "ll", reonGrammar, ReonOutput{}};
  t.run(input, output);
}

void run_with_arguments(int argc, char **argv);

// main
int main(int argc, char **argv) {
  try {
    run_with_arguments(argc, argv);
  } catch (LexicalError &le) {
    cerr << "Lexical Error: " << le.what() << "\n";
    return LEXICAL_ERROR;
  } catch (SyntaxError &se) {
    cerr << "Syntax Error: " << se.what() << "\n";
    return SYNTAX_ERROR;
  } catch (SemanticError &se) {
    cerr << "Semantic Error: " << se.what() << "\n";
    return SEMANTIC_ERROR;
  } catch (TranslationException &te) {
    cerr << "Translation Error: " << te.what() << "\n";
    return TRANSLATION_ENGINE_ERROR;
  } catch (std::invalid_argument &ia) {
    cerr << "Invalid argument: " << ia.what() << "\n";
    return INVALID_ARGUMENT;
  } catch (std::exception &e) {
    cerr << "Runtime Error: " << e.what() << "\n";
    return RUNTIME_ERROR;
  } catch (...) {
    cerr << "An unknown exception was thrown.\n";
    return UNKNOWN_EXCEPTION;
  }
  return 0;
}

void run_with_arguments(int argc, char **argv) {
  std::ifstream fileIn;
  std::ofstream fileOut;

  std::istream *input = &cin;
  std::ostream *output = &cout;

  bool inputDefined = false;
  bool outputDefined = false;
  for (int i = 1; i < argc; i++) {
    string arg{argv[i]};
    if (arg == "-i") {
      if (inputDefined) {
        throw std::invalid_argument("Multiple input definitions.");
      }
      inputDefined = true;
      if (++i == argc) {
        throw std::invalid_argument("No input file given after -i.");
      }
      fileIn.open(argv[i]);
      if (fileIn.fail()) {
        throw std::invalid_argument("Could not open file " + string{argv[i]} +
                                    " for input.");
      }
      input = &fileIn;
    } else if (arg == "-o") {
      if (outputDefined) {
        throw std::invalid_argument("Multiple output definitions.");
      }
      outputDefined = true;
      if (++i == argc) {
        throw std::invalid_argument("No input file given after -i.");
      }
      fileOut.open(argv[i]);
      if (fileOut.fail()) {
        throw std::invalid_argument("Could not open file " + string{argv[i]} +
                                    " for output.");
      }
      output = &fileOut;
    } else if (arg == "-h" || arg == "--help") {
      print_help();
      return;
    } else {
      throw std::invalid_argument("Unknown argument " + arg +
                                  ". Run with -h for help.");
    }
  }

  translation(*input, *output);
}

void print_help() {
  cout << "reon - translates reon to Python 3 RE.\n\n";
  cout << "usage: ./reon [-i input] [-o output]\n";
  cout << "\n";
  cout << "-i input: Sets input to the input file. Default input is stdin.\n";
  cout << "-o output: Sets output to the output file. Default output is "
          "stdout.\n";
}