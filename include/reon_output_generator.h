/**
\file reon_output_generator.h
\brief Implements output generation and semantic analysis for reon.
\author Radek Vít
*/
#ifndef REON_OUTPUT_GENERATOR
#define REON_OUTPUT_GENERATOR

#include <ctf.hpp>

#include <cctype>
#include <cstdlib>
#include <map>
#include <set>

/*
Output terminals with meaning:
        re          -   sequence of characters
        set         -   set characters
        ref         -   name reference
        nref        -   numerical reference
        comment     -   comment body
        repeat      -   repeat string: *, {m}, {-n},...
        flags       -   aiLmsux
        flag        -   ismx-ismx
        named group -   group name
        group       -   group definition for semantic analysis
*/
/**
\brief Callable class for reon output generation and semantic checks.
*/
class ReonOutput {
 public:
  using uint_type = size_t;

 protected:
  /**
  \brief Set of known group names.
  */
  std::set<string> knownGroups_{};
  /**
  \brief Ammount of groups defined thus far.
  */
  uint_type numberGroups_ = 0;

  /**
  \brief Vector of semantic checks callbacks.
  */
  vector<std::function<void(const Symbol &)>> semanticChecks_{};

  /**
  \brief Substitute for a string switch statement for invoking methods
  appropriate for incoming symbols.
  */
  const std::map<string, std::function<void(std::ostream &, const Symbol &)>>
      symbolMap_{
          {"re", std::bind(&ReonOutput::re, this, std::placeholders::_1,
                           std::placeholders::_2)},
          {"set", std::bind(&ReonOutput::set, this, std::placeholders::_1,
                            std::placeholders::_2)},
          {"ref", std::bind(&ReonOutput::ref, this, std::placeholders::_1,
                            std::placeholders::_2)},
          {"nref", std::bind(&ReonOutput::nref, this, std::placeholders::_1,
                             std::placeholders::_2)},
          {"comment", std::bind(&ReonOutput::comment, this,
                                std::placeholders::_1, std::placeholders::_2)},
          {"repeat", std::bind(&ReonOutput::repeat, this, std::placeholders::_1,
                               std::placeholders::_2)},
          {"flags", std::bind(&ReonOutput::flags, this, std::placeholders::_1,
                              std::placeholders::_2)},
          {"flag", std::bind(&ReonOutput::flag, this, std::placeholders::_1,
                             std::placeholders::_2)},
          {"named group",
           std::bind(&ReonOutput::named_group, this, std::placeholders::_1,
                     std::placeholders::_2)},
          {"group", std::bind(&ReonOutput::group, this, std::placeholders::_1,
                              std::placeholders::_2)},
          {"fixed_length_check",
           std::bind(&ReonOutput::add_fixed_length_check, this,
                     std::placeholders::_1, std::placeholders::_2)},
          {"end_check",
           std::bind(&ReonOutput::end_check, this, std::placeholders::_1,
                     std::placeholders::_2)},
      };

  /**
  \brief Resets the output generator.
  */
  void clear_all() {
    knownGroups_.clear();
    numberGroups_ = 0;

    semanticChecks_.clear();
  }

  /**
  \brief Semantic check; checks if this part of the RE has fixed length.
  */
  void fixed_length_check(const Symbol &symbol) {
    if (symbol.name() == "repeat") {
      // must be a constant length
      for (char c : symbol.attribute()) {
        if (!std::isdigit(c))
          throw SemanticError(
              "RE of non-constant length within a lookbehind assertion.");
      }
    } else if (symbol.name() == "ref" || symbol.name() == "nref") {
      throw SemanticError(
          "REON currently does not support group references within lookbehind "
          "assertions.");
    }
  }

  /**
  \brief Adds fixed_length_check to semantic checks.
  */
  void add_fixed_length_check(std::ostream &, const Symbol &) {
    semanticChecks_.push_back(std::bind(&ReonOutput::fixed_length_check, this,
                                        std::placeholders::_1));
  }

  /**
  \brief Pops the stack of semantic checks.
  */
  void end_check(std::ostream &, const Symbol &) { semanticChecks_.pop_back(); }

  /**
  \brief Outputs a 're' terminal. Escapes all appropriate characters, unescapes
  ., $, ^.
  Checks escapes for validity.
  */
  void re(std::ostream &out, const Symbol &s) {
    bool lastEscaped = false;
    for (char c : s.attribute()) {
      /* regular character output */
      if (!lastEscaped) {
        switch (c) {
          case '*':
          case '+':
          case '?':
          case '{':
          case '}':
          case '[':
          case ']':
          case '|':
          case '(':
          case ')':
          case '$':
          case '^':
          case '.':
            out << '\\' << c;
            break;
          case '\\':
            lastEscaped = true;
            break;
          default:
            out << c;
            break;
        }
        /* escaped character output */
      } else {
        lastEscaped = false;
        if (std::isdigit(c)) {
          throw SemanticError(
              "Numbers cannot be escaped in string. To reference a group, an "
              "explicit reference must be used.");
        }
        if (std::isalpha(c)) {
          switch (c) {
            case 'A':
            case 'b':
            case 'B':
            case 'd':
            case 'D':
            case 'f':
            case 'n':
            case 'r':
            case 's':
            case 'S':
            case 't':
            case 'u':
            case 'U':
            case 'v':
            case 'w':
            case 'W':
            case 'x':
            case 'Z':
              out << '\\';
              break;
            default:
              throw SemanticError("Unknown escaped sequence \\" +
                                  std::string{c} + ".");
          }
        }
        if (c != '.' && c != '^' && c != '$' && c != '\\') {
          throw SemanticError(
              "Character " + std::string{c} +
              " cannot be escaped in a string character sequence.");
        }
        if (c == '\\')
          out << '\\';
        out << c;
      }
    }
  }

  /**
  \brief Outputs 'set' terminal. Escapes appropriate characters. Checks
  character ranges.
  */
  void set(std::ostream &out, const Symbol &s) {
    string finalSet;
    bool escape = false;
    bool range = false;
    char last = '\0';
    for (char c : s.attribute()) {
      if (escape) {
        escape = false;
        finalSet += "\\c";
        continue;
      }
      if (range) {
        range = false;
        if (last >= c)
          throw SemanticError("Invalid char range " + string{last} + "-" +
                              string{c} + ".");
        finalSet += "-";
      }
      if (c != '-')
        last = c;
      switch (c) {
        case '\\':
          escape = true;
          break;
        case '-':
          range = true;
          break;
        case ']':
        case '^':
          finalSet += "\\" + string{c};
          break;
        default:
          finalSet += string{c};
      }  // switch
    }    // for all characters
    if (range)
      finalSet += "-";
    out << finalSet;
  }

  /**
  \brief Outputs a 'ref' terminal. Checks if a group with this name exists.
  */
  void ref(std::ostream &out, const Symbol &s) {
    if (knownGroups_.count(s.attribute()) == 0)
      throw SemanticError("No group named " + s.attribute() +
                          " is known at this point.");
    out << s.attribute();
  }
  /**
  \brief Outputs a 'nref' terminal. Checks if a group with this number exists.
  */
  void nref(std::ostream &out, const Symbol &s) {
    char *endptr;
    long x = std::strtol(s.attribute().c_str(), &endptr, 10);
    if (*endptr != '\0' || x < 1)
      throw SemanticError(
          "Only positive integers are permitted as references.");
    if (static_cast<uint_type>(x) > numberGroups_)
      throw SemanticError("No group with number " + s.attribute() + ".");

    out << s.attribute();
  }
  /**
  \brief Outputs a comment. Escapes ')'.
  */
  void comment(std::ostream &out, const Symbol &s) {
    for (char c : s.attribute()) {
      if (c == ')')
        out << '\\';
      out << c;
    }
  }
  /**
  \brief Outputs a 'repeat' terminal. Checks the repetition validity.
  */
  void repeat(std::ostream &out, const Symbol &s) {
    // most of validity is assured by lexical analysis
    // check if m is larger than n
    if (s.attribute().length() == 1) {
      switch (s.attribute()[0]) {
        case '*':
        case '+':
        case '?':
          out << s.attribute();
          return;
        default:
          break;
      }
    }
    out << "{";
    if (s.attribute().back() != '-') {
      uint_type first = 0;
      uint_type second = 0;

      uint_type *current = &first;
      for (char c : s.attribute()) {
        if (c == '-') {
          current = &second;
          continue;
        }
        *current = *current * 10 + (c - '0');
      }
      if (current == &second && first >= second)
        throw SemanticError("Maximum repeats are larger than minimum repeats.");
    }
    out << s.attribute();
    out << "}";
  }
  /**
  \brief Outputs the 'flags' terminal. Checks if the flags given are available.
  */
  void flags(std::ostream &out, const Symbol &s) {
    std::set<char> used{};
    static const std::set<char> available{'a', 'i', 'L', 'm', 's', 'u', 'x'};
    for (char c : s.attribute()) {
      auto it = available.find(c);
      if (it == available.end())
        throw SemanticError("Unknown flag " + string{c} +
                            "in 'flags' sequence.");
      if (used.find(c) == used.end()) {
        out << c;
        used.insert(c);
      }
    }
  }
  /**
  \brief Outputs the 'flag' terminal. Checks the flag sequence validity.
  */
  void flag(std::ostream &out, const Symbol &s) {
    std::set<char> used{};
    std::set<char> available{'i', 's', 'm', 'x', '-'};
    for (char c : s.attribute()) {
      auto it = available.find(c);
      if (it == available.end())
        throw SemanticError("Unknown flag " + string{c} +
                            "in 'flag ismx-ismx' sequence.");
      available.erase(it);
      used.insert(c);
      out << c;
    }
  }
  /**
  \brief Outputs the 'named_group' terminal. Validates the group's name. Adds
  the group name to the set of known group names.
  */
  void named_group(std::ostream &out, const Symbol &s) {
    numberGroups_++;
    // checking validity of identifier
    if (s.attribute().length() == 0)
      throw SemanticError(
          "Identifier of a named group cannot have a length of 0.");
    char first = s.attribute()[0];
    if (!std::isalpha(first) && first != '_') {
      throw SemanticError("Identifier of a named group cannot start with " +
                          string{first} + ".");
    }
    for (char c : s.attribute()) {
      if (!std::isalnum(c) && c != '_')
        throw SemanticError("Identifier of a named group cannot contain " +
                            string{first} + ".");
    }
    out << s.attribute();
    if (knownGroups_.find(s.attribute()) != knownGroups_.end()) {
      throw SemanticError("Multiple definitions of a group with name " +
                          s.attribute() + ".");
    }
    knownGroups_.insert(s.attribute());
  }
  /**
  \brief Marks the presence of a group.
  */
  void group(std::ostream &, const Symbol &) { numberGroups_++; }

  /**
  \brief Outputs a terminal's name.
  */
  void symbol(std::ostream &out, const Symbol &s) {
    // unknown; putting name to output
    out << s.name();
  }

 public:
  /**
  \brief Outputs the incoming symbol. Resets on receiving Symbol::EOI().
  Performs semantic checks.
  \param[out] out Output stream.
  \param[in] s Incoming symbol.
  */
  void operator()(std::ostream &out, const Symbol &s) {
    if (s == Symbol::EOI()) {
      clear_all();
      return;
    }
    for (auto check : semanticChecks_) {
      check(s);
    }
    // runs name specific method
    auto it = symbolMap_.find(s.name());
    if (it == symbolMap_.end())
      return symbol(out, s);
    return it->second(out, s);
  }
};

#endif
/*** End of file reon_output_generator.h ***/