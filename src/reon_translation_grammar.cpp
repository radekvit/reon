#include <reon_translation_grammar.h>

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

const TranslationGrammar reonGrammar{
    // Rules
    {
        {"E"_nt, {"RE"_nt}, {"re = r\""_t, "RE"_nt, "\"\n"_t}},
        {"RE"_nt, {}},
        {"RE"_nt, {"REFULL"_nt}},
        {"REFULL"_nt, {"string"_t}, {"re"_t}},
        {"REFULL"_nt, {"["_t, "RE-listE"_nt, "]"_t}, {"RE-listE"_nt}},
        {"REFULL"_nt, {"{"_t, "OBJ"_nt, "}"_t}, {"OBJ"_nt}},
        {"OBJ"_nt,
         {"repeat"_t, ":"_t, "RE"_nt},
         {"(?:"_t, "RE"_nt, ")"_t, "repeat"_t},
         {{3}, {}}},
        {"OBJ"_nt,
         {"ngrepeat"_t, ":"_t, "RE"_nt},
         {"(?:"_t, "RE"_nt, ")"_t, "repeat"_t, "?"_t},
         {{3}, {}}},
        {"OBJ"_nt,
         {"set"_t, ":"_t, "string"_t},
         {"["_t, "set"_t, "]"_t},
         {{}, {}, {1}}},
        {"OBJ"_nt,
         {"nset"_t, ":"_t, "string"_t},
         {"[^"_t, "set"_t, "]"_t},
         {{}, {}, {1}}},
        {"OBJ"_nt,
         {"alternatives"_t, ":"_t, "["_t, "RE-AlistE"_nt, "]"_t},
         {"RE-AlistE"_nt}},
        {"OBJ"_nt, {"group"_t, ":"_t, "RE"_nt}, {"("_t, "group"_s , "RE"_nt, ")"_t}},
        {"OBJ"_nt,
         {"flags"_t, ":"_t, "string"_t},
         {"(?"_t, "flags"_t, ")"_t},
         {{}, {}, {1}}},
        {"OBJ"_nt, {"agroup"_t, ":"_t, "RE"_nt}, {"(?:"_t, "RE"_nt, ")"_t}},
        {"OBJ"_nt,
         {"flag"_t, ":"_t, "RE"_nt},
         {"(?"_t, "flag"_t, ":"_t, "RE"_nt, ")"_t},
         {{1}, {}}},
        {"OBJ"_nt,
         {"named group"_t, ":"_t, "RE"_nt},
         {"(?P<"_t, "named group"_t, ">"_t, "RE"_nt, ")"_t},
         {{1}, {}}},
        {"OBJ"_nt, {"reference"_t, ":"_t, "Ref"_nt}, {"Ref"_nt}},
        {"OBJ"_nt,
         {"comment"_t, ":"_t, "string"_t},
         {"(?#"_t, "comment"_t, ")"_t},
         {{}, {}, {1}}},
        {"OBJ"_nt, {"lookahead"_t, ":"_t, "RE"_nt}, {"(?="_t, "RE"_nt, ")"_t}},
        {"OBJ"_nt, {"nlookahead"_t, ":"_t, "RE"_nt}, {"(?!"_t, "RE"_nt, ")"_t}},
        {"OBJ"_nt,
         {"lookbehind"_t, ":"_t, "RE"_nt},
         {"(?<="_t, "fixed_length_check"_s, "RE"_nt, "end_check"_s , ")"_t}},
        {"OBJ"_nt,
         {"nlookbehind"_t, ":"_t, "RE"_nt},
         {"(?<!"_t, "RE"_nt, ")"_t}},
        {"OBJ"_nt,
         {"if"_t, ":"_t, "IfRef"_nt, ","_t, "then"_t, ":"_t, "RE"_nt, "Else"_nt},
         {"(?("_t, "IfRef"_nt, ")"_t, "RE"_nt, "Else"_nt, ")"_t}},
        {"Ref"_nt, {"number"_t}, {"\\"_t, "nref"_t}, {{1}}},
        {"Ref"_nt, {"string"_t}, {"(?P="_t, "ref"_t, ")"_t}, {{1}}},
        {"IfRef"_nt, {"number"_t}, {"nref"_t}, {{0}}},
        {"IfRef"_nt, {"string"_t}, {"ref"_t}, {{0}}},
        {"Else"_nt, {","_t, "else"_t, ":"_t, "RE"_nt}, {"|"_t, "RE"_nt}},
        {"Else"_nt, {}},
        {"RE-listE"_nt, {"REFULL"_nt, "RE-list"_nt}},
        {"RE-listE"_nt, {}},
        {"RE-list"_nt, {","_t, "RE-list-comma"_nt}, {"RE-list-comma"_nt}},
        {"RE-list-comma"_nt, {}},
        {"RE-list-comma"_nt, {"REFULL"_nt, "RE-list"_nt}},
        {"RE-list"_nt, {}},
        {"RE-AlistE"_nt, {}},
        {"RE-AlistE"_nt,
         {"REFULL"_nt, "RE-Alist"_nt},
         {"(?:"_t, "REFULL"_nt, "RE-Alist"_nt, ")"_t}},
        {"RE-Alist"_nt, {}},
        {"RE-Alist"_nt, {","_t, "RE-Alist-comma"_nt}, {"RE-Alist-comma"_nt}},
        {"RE-Alist-comma"_nt, {}},
        {"RE-Alist-comma"_nt,
         {"REFULL"_nt, "RE-Alist"_nt},
         {"|"_t, "REFULL"_nt, "RE-Alist"_nt}},
    },
    // Starting nonterminal
    "E"_nt};
