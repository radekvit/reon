/**
\file reon_lexical_analyzer.h
\brief Implements a lexical analyzer for reon.
\author Radek Vít
*/
#ifndef REON_LEXICAL_ANALYZER
#define REON_LEXICAL_ANALYZER

#include <ctf.h>
#include <cctype>
#include <iterator>
#include <sstream>
#include <string>

/**
\brief Recursive descent lexical analyzer for reon. Callable class.

Buffers all of the stream input. Resets on input stream change and on returning
Symbol::EOI.
*/
class ReonLexer {
 public:
  using uint_type = size_t;

  /**
  \brief Stream that is read from.
  */
  std::istream *assignedStream_ = nullptr;
  /**
  \brief Buffer containing all characters from the stream.
  */
  std::string buffer_;
  /**
  \brief Attribute string.
  */
  std::string read_;
  /**
  \brief Currently read character.
  */
  char c;

  /**
  \brief Current read position.
  */
  uint_type position_ = 0;
  /**
  \brief Number of characters in buffer.
  */
  uint_type size_ = 0;

  /**
  \brief Column of current read position.
  */
  uint_type col_ = 1;
  /**
  \brief Row of current read position.
  */
  uint_type row_ = 1;

  /**
  \brief Assigns a stream and fills the buffer from it.
  \param[in] is Input stream for assignment.

  Sets new buffer size, resets read position, col and row positions.
  */
  void fill_buffer(std::istream &is) {
    buffer_ = std::string(std::istreambuf_iterator<char>(is), {});
    size_ = buffer_.size();
    position_ = 0;
    assignedStream_ = &is;
    col_ = 1;
    row_ = 1;
  }

  /**
  \brief Reads the next character into c. Adjusts positions.
  \returns False if EOF is encountered, true otherwise.
  */
  bool read() {
    if (position_ == size_)
      return false;

    c = buffer_[position_++];
    col_++;
    if (c == '\n') {
      col_ = 1;
      row_++;
    }
    return true;
  }

  /**
  \brief Appends char to attribute string.
  \param[in] a Character to append.
  */
  void append(const char &a) { read_ += a; }

  /**
  \brief Appends c to attribute string.
  */
  void append() { read_ += c; }

  /**
  \brief Reads and appends character to attribute string.
  \returns False if EOF is encountered. True otherwise.
  */
  bool read_append() {
    if (read()) {
      append();
      return true;
    }
    return false;
  }

  /**
  \brief Moves reading position back.
  \param[in] i How many characters to roll back.
  */
  void roll_back(uint_type i) { position_ -= i; }

  /**
  \brief Clears attribute string and c before reading new token.
  */
  void clear() {
    read_ = "";
    c = '\0';
  }

  /**
  \brief Returns current attribute string.
  \returns Current attribute string.
  */
  string &atr() { return read_; }

  /**
  \brief Resets analyzer. After assigning a stream, it acts as if it was just
  constructed.
  */
  void reset() { assignedStream_ = nullptr; }

  /**
  \brief Returns Symbol::EOI(), resets lexical analyzer.
  \returns Symbol::EOI().
  */
  Symbol EOI() {
    reset();
    return Symbol::EOI();
  }

  /**
  \brief Throws exception with given message and adds row and col information.
  */
  void throw_exception(const string &msg) {
    throw LexicalError("Lexical error on row " + std::to_string(row_) +
                       ", col " + std::to_string(col_) + ": " + msg);
  }

  /**
  \brief Converts a char to a string.
  */
  string s(char cs) { return string{cs}; }

  /**
  \brief Initial state. Clears attribute string and acts according to first read
  character.
  \returns Appropriate Token.
  */
  Token state_init() {
    clear();
    do {
      if (!read())
        return EOI();
    } while (std::isspace(c));
    /* : is hadled after string because of special identifiers */
    switch (c) {
      case '[':
      case ']':
      case '{':
      case '}':
      case ',':
      case ':':
        return Terminal(s(c));
      case '"':
        return state_string();
      case '-':
        append();
        return state_number_minus();
      case '0':
        append();
        return state_number_zero();
      case 't':
        return state_true();
      case 'f':
        return state_false();
      case 'n':
        return state_null();
      default:
        if (std::isdigit(c)) {
          append();
          return state_number();
        }
    }  // switch
    throw_exception("No token beginning with " + s(c) + ".");
    // to shut compiler up
    return Symbol::EOI();
  }

  /**
  \brief Reading string contents. Resolves escapes.
  \returns Token string or one of the keyword tokens.
  */
  Token state_string() {
    while (1) {
      if (!read())
        throw_exception("Unexpected EOF when reading string.");
      switch (c) {
        case '"':
          // may return keywords if there is a : after
          return state_string_end();
        case '\\': {
          if (!read())
            throw_exception("Unexpected EOF when reading string.");
          switch (c) {
            /* Allow all characters to be escaped
            case '"':
            case '\\':
            case '/':
            case 'b':
            case 'n':
            case 'r':
            case 't':
              append('\\');
              append();
              break;
            */
            case 'u': {  // converts to literal character
              char temp[5] = {
                  0,
              };
              for (auto i = 0; i < 4; ++i) {
                if (!read())
                  throw_exception("Unexpected EOF when reading \\u literal.");
                if (!isxdigit(c))
                  throw_exception(
                      "Non hexa-digit character when reading \\u literal.");
                temp[i] = c;
              }  // for
              int result = std::strtol(temp, nullptr, 16);
              if (result > 0xFF)
                append(static_cast<char>(result >> 8));
              append(static_cast<char>(result & 0xFF));
              break;
            }  // case 'u'
            default:
              append('\\');
              append();
          }  // switch
          break;
        }  // case escape
        default:
          append();
          break;
      }
    }  // while 1
  }

  /**
  \brief Checks whether a ':' follows the read string. If it does, it checks for
  keywords.
  \returns Token string or keyword token.
  */
  Token state_string_end() {
    do {
      if (!read())
        return Token{"string", atr()};
    } while (std::isspace(c));

    // only checking following character, next token begins with it
    roll_back(1);

    switch (c) {
      case ':':
        return check_keywords();
      default:
        return Token{"string", {atr()}};
    }
  }

  /**
  \brief Reading number after '-'.
  \returns Token number.
  */
  Token state_number_minus() {
    if (!read_append())
      throw_exception("Unexpected EOF when reading a number.");
    switch (c) {
      case '0':
        return state_number_zero();
      default:
        if (std::isdigit(c)) {
          return state_number();
        }
    }
    throw_exception("Unexpected " + s(c) + " when reading a number.");
    // to shut compiler up
    return Symbol::EOI();
  }

  /**
  \brief Reading after initial '0'.
  \returns Token number.
  */
  Token state_number_zero() {
    if (!read())
      return Token{"number", atr()};
    switch (c) {
      case '.':
        append();
        return state_number_dec();
      default:
        roll_back(1);
        return Token{"number", atr()};
    }
  }

  /**
  \brief Reads numbers until a dot or non-digit is read.
  \returns Token number.
  */
  Token state_number() {
    while (1) {
      if (!read())
        return Token{"number", atr()};
      switch (c) {
        case '.':
          append();
          return state_number_dec();
        case 'e':
        case 'E':
          append();
          return state_number_e();
        default:
          if (!std::isdigit(c)) {
            roll_back(1);
            return Token{"number", atr()};
          } else {
            append();
          }
      }  // switch
    }    // while 1
  }

  /**
  \brief Reading number after decimal dot.
  \returns Token number.
  */
  Token state_number_dec() {
    if (!read())
      throw_exception("Unexpected EOF after decimal dot when reading number.");
    if (!std::isdigit(c))
      throw_exception("Unexpected " + s(c) +
                      " after decimal dot when reading number.");
    append();
    while (1) {
      if (!read())
        return Token{"number", atr()};
      switch (c) {
        case 'e':
        case 'E':
          append();
          return state_number_e();
        default:
          if (!std::isdigit(c)) {
            roll_back(1);
            return Token{"number", atr()};
          }
          append();
      }  // switch
    }    // while 1
  }

  /**
  \brief Reading number after 'e'
  \returns Token number.
  */
  Token state_number_e() {
    if (!read_append())
      throw_exception("Unexpected EOF after e when reading number.");
    if (c != '+' && c != '-' && !std::isdigit(c))
      throw_exception("Unexpected " + s(c) + " after e when reading number.");
    while (1) {
      if (!read())
        return Token{"number", atr()};
      if (!std::isdigit(c)) {
        roll_back(1);
        return Token{"number", atr()};
      }  // if
      append();
    }  // while 1
  }

  /**
  \brief Reading 'true' literal.
  \returns Token true.
  */
  Token state_true() {
    const static char rue[] = "rue";
    for (auto i = 0; i < 3; ++i) {
      if (!read())
        throw_exception("Unexpected EOF when reading 'true'.");
      if (c != rue[i]) {
        throw_exception("Unexpected " + s(c) + " when reading 'true'.");
      }
    }
    return Token{"true"};
  }

  /**
  \brief Reading 'false' literal.
  \returns Token false.
  */
  Token state_false() {
    const static char alse[] = "alse";
    for (auto i = 0; i < 4; ++i) {
      if (!read())
        throw_exception("Unexpected EOF when reading 'false'.");
      if (c != alse[i]) {
        throw_exception("Unexpected " + s(c) + " when reading 'false'.");
      }
    }
    return Token{"false"};
  }

  /**
  \brief Reading 'null' literal.
  \returns Token null.
  */
  Token state_null() {
    const static char ull[] = "ull";
    for (auto i = 0; i < 3; ++i) {
      if (!read())
        throw_exception("Unexpected EOF when reading null.");
      if (c != ull[i]) {
        throw_exception("Unexpected " + s(c) + " when reading 'null'.");
      }
    }
    return Token{"null"};
  }

  /**
  \brief Checking for keywords in string.
  \returns Token string or one of the keyword tokens.
  */
  Token check_keywords() {  // TODO: all keywords
    auto &a = atr();
    // repetition operators
    if (!a.compare(0, 7, "repeat ", 0, 7)) {
      a.erase(0, 7);
      return keyword_repeat("repeat");
    }
    if (!a.compare(0, 9, "ngrepeat ", 0, 9)) {
      a.erase(0, 9);
      return keyword_repeat("ngrepeat");
    }
    if (a == "set")
      return Token{"set"};
    if (a == "nset" || a == "negated set")
      return Token{"nset"};
    if (a == "alternatives")
      return Token{"alternatives"};
    if (a == "group")
      return Token{"group"};
    if (a == "flags")
      return Token{"flags"};
    if (a == "agroup" || a == "non-capturing group")
      return Token{"agroup"};
    if (!a.compare(0, 5, "flag ", 0, 5)) {
      a.erase(0, 5);
      return keyword_flag();
    }
    if (!a.compare(0, 6, "group ", 0, 6)) {
      a.erase(0, 6);
      return Token{"named group", a};
    }
    if (a == "reference")
      return Token{"reference"};
    if (a == "comment")
      return Token{"comment"};
    if (a == "lookahead")
      return Token{"lookahead"};
    if (a == "nlookahead" || a == "negative lookahead")
      return Token{"nlookahead"};
    if (a == "lookbehind")
      return Token{"lookbehind"};
    if (a == "nlookbehind" || a == "negative lookbehind")
      return Token{"nlookbehind"};
    if (a == "if")
      return Token{"if"};
    if (a == "then")
      return Token{"then"};
    if (a == "else")
      return Token{"else"};
    // no match
    return Token{"string", a};
  }

  /**
  \brief Reading flags in keyword 'flag'.
  \returns Token flag or token string.
  */
  Token keyword_flag() {
    auto &a = atr();
    char c = a[0];
    int phase = 0;
    int firstUsed = 0, secondUsed = 0;
    for (uint_type i = 0; i < a.size(); ++i, c = a[i]) {
      switch (c) {
        case 'i':
        case 's':
        case 'm':
        case 'x':
          switch (phase) {
            case 0:
              firstUsed++;
              break;
            case 1:
              secondUsed++;
              break;
            default:
              return Token{"string", "flag " + a};
          }  // switch
          break;
        case '-':
          phase++;
          break;
        default:
          return Token{"string", "flag " + a};

      }  // switch
    }    // for
    if (firstUsed >= 4 || (phase == 0 && firstUsed == 0) ||
        (phase == 1 && (secondUsed == 0 || secondUsed >= 4)))
      return Token{"string", "flag " + a};
    return Token{"agroup", a};
  }

  /**
  \brief Reads repeats in keyword 'repeat'.
  \returns Token repeat, ngrepeat or string.
  */
  Token keyword_repeat(const string &token) {
    // checks attribute
    enum class State { INIT, FIRST, SECOND, INVALID } state = State::INIT;
    auto &a = atr();
    if (a.length() == 0)
      return Token{"string", token + " " + a};
    if (a == "*" || a == "+" || a == "?")
      return Token{token, a};
    char c = a[0];
    for (uint_type i = 0; i < a.size(); ++i, c = a[i]) {
      switch (state) {
        case State::INIT:
          if (c == '-') {
            state = State::SECOND;
          } else if (!std::isdigit(c)) {
            state = State::INVALID;
            break;
          }
          state = State::FIRST;
          break;
        case State::FIRST:
          if (c == '-') {
            state = State::SECOND;
            break;
          }
        case State::SECOND:
          if (!std::isdigit(c)) {
            state = State::INVALID;
            break;
          }
        case State::INVALID:
          break;
      }  // switch
    }    // for
    if (state == State::INVALID)
      return Token{"string", token + " " + a};
    if (token == "ngrepeat" && state == State::FIRST)
      return Token{"repeat", a};
    return Token{token, a};
  }

 public:
  /**
  \brief Sets stream if changed and gets a token.
  */
  Token operator()(std::istream &is) {
    if (&is != assignedStream_)
      fill_buffer(is);
    return state_init();
  }
};

#endif
/*** End of file reon_lexical_analyzer.h ***/