#ifndef REON_OUTPUT_GENERATOR
#define REON_OUTPUT_GENERATOR

#include <ctf.h>

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

class ReonOutput {
 public:
  using uint_type = size_t;

 protected:
  std::set<string> knownGroups_{};
  uint_type numberGroups_ = 0;

  vector<std::function<void(const Symbol &)>> semanticChecks_{};

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
          {"fixed_length_check", std::bind(&ReonOutput::add_fixed_length_check, this, std::placeholders::_1, std::placeholders::_2)},
          {"end_check", std::bind(&ReonOutput::end_check, this, std::placeholders::_1, std::placeholders::_2)},
      };

  void clear_all() {
    knownGroups_.clear();
    numberGroups_ = 0;

    semanticChecks_.clear();
  }

  void fixed_length_check(const Symbol &symbol) {
    if(symbol.name() == "repeat") {
      //must be a constant length
      for(char c: symbol.attribute()) {
        if(!std::isdigit(c))
          throw SemanticError("RE of non-constant length within a lookbehind assertion.");
      }
    }
    else if (symbol.name() == "ref" || symbol.name() == "nref")
    {
      throw SemanticError("REON currently does not support group references within lookbehind assertions.");
    }
  }

  void add_fixed_length_check(std::ostream &, const Symbol &) {
    semanticChecks_.push_back(std::bind(&ReonOutput::fixed_length_check, this, std::placeholders::_1));
  }

  void end_check(std::ostream &, const Symbol &) {
    semanticChecks_.pop_back();
  }

  void re(std::ostream &out, const Symbol &s) {
    bool lastEscaped = false;
    for (char c : s.attribute()) {
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
          out << '\\' << c;
          break;
        default:
          if (lastEscaped && std::isdigit(c)) {
            out << '\\';
          }
          if (lastEscaped && std::isalpha(c)) {
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
                break;
              default:
                throw SemanticError("Unknown escaped sequence \\" + string{c} +
                                    ".");
            }
          }
          out << c;
      }
      if (c == '\\')
        lastEscaped = true;
      else
        lastEscaped = false;
    }
  }
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
        default:
          finalSet += string{c};
      }  // switch
    }    // for all characters
    if (range)
      finalSet += "-";
    out << finalSet;
  }
  void ref(std::ostream &out, const Symbol &s) {
    if (knownGroups_.count(s.attribute()) == 0)
      throw SemanticError("No group named " + s.attribute() +
                          " is known at this point.");
    out << s.attribute();
  }
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
  void comment(std::ostream &out, const Symbol &s) {
    for (char c : s.attribute()) {
      if (c == ')')
        out << '\\';
      out << c;
    }
  }
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

  void flags(std::ostream &out, const Symbol &s) {
    std::set<char> used{};
    std::set<char> available{'a', 'i', 'L', 'm', 's', 'u', 'x'};
    for (char c : s.attribute()) {
      auto it = available.find(c);
      if (it == available.end())
        throw SemanticError("Unknown flag " + string{c} +
                            "in 'flags' sequence.");
      available.erase(it);
      used.insert(c);
      out << c;
    }
  }

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
    knownGroups_.insert(s.attribute());
  }

  void group(std::ostream &, const Symbol &) { numberGroups_++; }

  void symbol(std::ostream &out, const Symbol &s) {
    // unknown; putting name to output
    out << s.name();
  }

 public:
  void operator()(std::ostream &out, const Symbol &s) {
    if (s == Symbol::EOI()) {
      clear_all();
      return;
    }
    for(auto check: semanticChecks_) {
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