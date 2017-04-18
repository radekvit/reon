/**
\file reon_translation_grammar.cpp
\brief Defines reonGrammar.
\author Radek Vít
*/
#include <reon_translation_grammar.h>

/*
Output terminals with special meaning:
  re          -   sequence of characters
  set         -   set characters
  ref         -   name reference
  nref        -   numerical reference
  comment     -   comment body
  repeat      -   repeat string: *, {m}, {-n},...
  named group -   group name

Output special symbols:
  group       -   group definition for semantic analysis
  fixed_length_check  -   checks lookbehind length
  end_check   -   pops one check
  variable    -   Python variable with name set in namespace global
*/

/**
\brief Defines the translation from reon to Python 3 RE.

This is an LL grammar for the input.
*/
const TranslationGrammar reonGrammar{
    // terminals are taken from rules
    // nonterminals are taken from rules
    // rules
    {
        // first derivation
        {"E"_nt, {"RE"_nt}, {"variable"_s, " = r\"(?m)"_t, "RE"_nt, "\"\n"_t}},
        // empty regular expression
        {"RE"_nt, {}},
        // regular expression with some reon content
        {"RE"_nt, {"REFULL"_nt}},
        // match empty string
        {"REFULL"_nt, {"true"_t}, {"re"_t}},
        // don't match ever
        {"REFULL"_nt, {"false"_t}, {"(?!)"_t}},
        // don't match ever
        {"REFULL"_nt, {"null"_t}, {"(?!)"_t}},
        // string only RE
        {"REFULL"_nt, {"string"_t}, {"re"_t}, {{0}}},
        // append list
        {"REFULL"_nt, {"["_t, "RE-listE"_nt, "]"_t}, {"RE-listE"_nt}},
        // object
        {"REFULL"_nt, {"{"_t, "OBJ"_nt, "}"_t}, {"OBJ"_nt}},
        // repeat object
        {"OBJ"_nt,
         {"repeat"_t, ":"_t, "RE"_nt},
         {"(?:"_t, "RE"_nt, ")"_t, "repeat"_t},
         {{3}, {}}},
        // non-greedy repeat object
        {"OBJ"_nt,
         {"non-greedy repeat"_t, ":"_t, "RE"_nt},
         {"(?:"_t, "RE"_nt, ")"_t, "repeat"_t, "?"_t},
         {{3}, {}}},
        // character set
        {"OBJ"_nt,
         {"set"_t, ":"_t, "string"_t},
         {"["_t, "set"_t, "]"_t},
         {{}, {}, {1}}},
        // negated character set
        {"OBJ"_nt,
         {"!set"_t, ":"_t, "string"_t},
         {"[^"_t, "set"_t, "]"_t},
         {{}, {}, {1}}},
        // alternation list
        {"OBJ"_nt,
         {"alternatives"_t, ":"_t, "["_t, "RE-AlistE"_nt, "]"_t},
         {"RE-AlistE"_nt}},
        // group
        {"OBJ"_nt,
         {"group"_t, ":"_t, "RE"_nt},
         {"("_t, "group"_s, "RE"_nt, ")"_t}},
        // group with identifier
        {"OBJ"_nt,
         {"named group"_t, ":"_t, "RE"_nt},
         {"(?P<"_t, "named group"_t, ">"_t, "RE"_nt, ")"_t},
         {{1}, {}}},
        // reference
        {"OBJ"_nt, {"match group"_t, ":"_t, "Ref"_nt}, {"Ref"_nt}},
        // comment
        {"OBJ"_nt,
         {"comment"_t, ":"_t, "string"_t},
         {"(?#"_t, "comment"_t, ")"_t},
         {{}, {}, {1}}},
        // lookahead
        {"OBJ"_nt, {"lookahead"_t, ":"_t, "RE"_nt}, {"(?="_t, "RE"_nt, ")"_t}},
        // negative lookahead
        {"OBJ"_nt, {"!lookahead"_t, ":"_t, "RE"_nt}, {"(?!"_t, "RE"_nt, ")"_t}},
        // lookbehind
        {"OBJ"_nt,
         {"lookbehind"_t, ":"_t, "RE"_nt},
         {"(?<="_t, "fixed_length_check"_s, "RE"_nt, "end_check"_s, ")"_t}},
        // negative lookbehind
        {"OBJ"_nt,
         {"!lookbehind"_t, ":"_t, "RE"_nt},
         {"(?<!"_t, "RE"_nt, ")"_t}},
        // if-then[-else]
        {"OBJ"_nt,
         {"if"_t, ":"_t, "IfRef"_nt, ","_t, "then"_t, ":"_t, "RE"_nt,
          "Else"_nt},
         {"(?("_t, "IfRef"_nt, ")"_t, "RE"_nt, "Else"_nt, ")"_t}},
        // number reference
        {"Ref"_nt, {"number"_t}, {"\\"_t, "nref"_t}, {{1}}},
        // identifier reference
        {"Ref"_nt, {"string"_t}, {"(?P="_t, "ref"_t, ")"_t}, {{1}}},
        // number reference in if-then[-else]
        {"IfRef"_nt, {"number"_t}, {"nref"_t}, {{0}}},
        // identifier reference in if-then[-else]
        {"IfRef"_nt, {"string"_t}, {"ref"_t}, {{0}}},
        // no else
        {"Else"_nt, {}},
        // optional else
        {"Else"_nt, {","_t, "else"_t, ":"_t, "RE"_nt}, {"|"_t, "RE"_nt}},
        // empty append list
        {"RE-listE"_nt, {}},
        // first element in append list
        {"RE-listE"_nt, {"REFULL"_nt, "RE-list"_nt}},
        // no more elements in append list
        {"RE-list"_nt, {}},
        // elements in append list
        {"RE-list"_nt, {","_t, "RE-list-comma"_nt}, {"RE-list-comma"_nt}},
        // no element after trailing comma in append list
        {"RE-list-comma"_nt, {}},
        // element after last comma in append list
        {"RE-list-comma"_nt, {"REFULL"_nt, "RE-list"_nt}},
        // empty alternation list
        {"RE-AlistE"_nt, {}},
        // first element in alternation list
        {"RE-AlistE"_nt,
         {"REFULL"_nt, "RE-Alist"_nt},
         {"(?:"_t, "REFULL"_nt, "RE-Alist"_nt, ")"_t}},
        // no more elements in alternation list
        {"RE-Alist"_nt, {}},
        // elements in alternation list
        {"RE-Alist"_nt, {","_t, "RE-Alist-comma"_nt}, {"RE-Alist-comma"_nt}},
        // no element after trailing comma in alternation list
        {"RE-Alist-comma"_nt, {}},
        // element after comma in alternation list
        {"RE-Alist-comma"_nt,
         {"REFULL"_nt, "RE-Alist"_nt},
         {"|"_t, "REFULL"_nt, "RE-Alist"_nt}},
    },
    // starting nonterminal
    "E"_nt};

/*** End of file reon_translation_grammar.cpp ***/