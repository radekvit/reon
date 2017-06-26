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
/**
Runtime error return value.
 */
const int RUNTIME_ERROR = 1;
/**
Invalid argument was passed to the application or some function return value.
 */
const int INVALID_ARGUMENT = 2;
/**
An unspecified error within ctf return value.
*/
const int TRANSLATION_ENGINE_ERROR = 3;
/**
Lexical analysis error return value.
*/
const int LEXICAL_ERROR = 5;
/**
A syntax error return value.
*/
const int SYNTAX_ERROR = 6;
/**
A semantic error return value.
 */
const int SEMANTIC_ERROR = 7;
/**
Unknown error return value.
 */
const int UNKNOWN_EXCEPTION = 666;

namespace globals {
string varname = "re";
}  // namespace globals

void print_help();

void translation(std::istream &input, std::ostream &output) {
  // reon translation unit, LL table driven translation
  Translation t{std::make_unique<ReonLexer>(), "ll", reonGrammar, std::make_unique<ReonOutput>()};
  t.run(input, output);
}

void run_with_arguments(int argc, char **argv);

// main
int main(int argc, char **argv) {
  try {
    run_with_arguments(argc, argv);
  } catch (TranslationError &le) {
    cerr << "\nTranslation error:\n" << le.what();
    return SYNTAX_ERROR;
  } catch (SemanticError &se) {
    cerr << "\nSemantic Error: " << se.what() << "\n";
    return SEMANTIC_ERROR;
  } catch (TranslationException &te) {
    cerr << "\nTranslation Error: " << te.what() << "\n";
    return TRANSLATION_ENGINE_ERROR;
  } catch (std::invalid_argument &ia) {
    cerr << "\nInvalid argument: " << ia.what() << "\n";
    return INVALID_ARGUMENT;
  } catch (std::exception &e) {
    cerr << "\nRuntime Error: " << e.what() << "\n";
    return RUNTIME_ERROR;
  } catch (...) {
    cerr << "\nAn unknown exception was thrown.\n";
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
  bool varDefined = false;
  for (int i = 1; i < argc; i++) {
    std::string arg{argv[i]};
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
        throw std::invalid_argument("Could not open file " +
                                    std::string{argv[i]} + " for input.");
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
        throw std::invalid_argument("Could not open file " +
                                    std::string{argv[i]} + " for output.");
      }
      output = &fileOut;
    } else if (arg == "-v") {
      if (varDefined) {
        throw std::invalid_argument("Multiple variable name definitions.");
      }
      varDefined = true;
      if (++i == argc) {
        throw std::invalid_argument("No variable name given after -v.");
      }
      globals::varname = string(argv[i]);
      if (globals::varname.size() == 0) {
        throw std::invalid_argument(
            "Variable name must be at least 1 character long.");
      }
      for (char c : globals::varname) {
        if (!std::isalpha(c)) {
          throw std::invalid_argument(
              "Variable name must contain only alphabetical characters.");
        }
      }
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
  cout << "usage: ./reon [-i input] [-o output] [-v variable]\n";
  cout << "\n";
  cout << "-i input: Sets input to the input file. Default input is stdin.\n";
  cout << "-o output: Sets output to the output file. Default output is "
          "stdout.\n";
  cout << "-v variable: Sets the variable name set in the input. Default "
          "variable name is \"re\".\n";
}