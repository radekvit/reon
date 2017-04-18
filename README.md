# REON
REON (Regular Expression Object Notation) is a JSON inspired language for describing regular expressions.

## Regular expression
A regular expression RE is one of the following:
 * `true` - matches anything
 * `false` - never matches
 * `null` - never matches
 * `"string"` - matches the string contents - see [specials](#special)
 * `[ RE, ...]` - a list of regular expressions, they are appended in order of appearance
 * `Object` - One of the following:
  * `{ "repeat X": RE }` - repetition of RE X times. X may be a number, a range or one of `*` (zero to infinite), `+` (one to infinite) and `?` (zero or one)
  * `{ "non-greedy repeat X": RE }` - the same as above, but matches the least repetitions possible
  * `{ "set": "string" }` - matches one of the characters in the string. The string may contain ranges of characters in the form `x-y`, where the value of `x` must be lower than that of `y`.
  * `{ "!set": "string" }` - matches any character but the ones in the string. The string contents are the same as above.
  * `{ "alternatives": [RE, ...] }` - matches one of the regular expressions in the list of regular expressions. REON does not guarantee which will be matched if there are more expressions that could be matched in the list.
  * `{ "group X": RE }` - creates a group which can be referenced later. The optional name `X` or the group's sequential number serve as reference to it.
  * `{ "match group": X }` - matches the same string the group with "name" or sequential number `X` matched. This must be placed after the referenced group definition.
  * `{ "comment": "string" }` - a comment
  * `{ "lookahead": RE }` - matches if RE is matched, but does not consume the string matched by RE
  * `{ "!lookahead": RE }` - same as above, but matches if RE is not matched
  * `{ "lookbehind": RE }` - matches if the string is preceded by RE. This does not consume RE. RE must have a fixed length (alternatives are allowed, repeats with a variable range are not)
  * `{ "!lookbehind": RE }` - same as above, but matches if the string is not preceded by RE.
  * `{ "if": X, "then": RE1, "else": RE2}` - If a group with "name" or number `X` was matched, this will match `RE1`, and if not will match `RE2`. The else clause is optional.

## <a name="special"></a>Special characters in RE strings
There are a few characters that, when escaped with `\`, have a special meaning.

 * `\\` matches the character `\`
 * `\^`, `\A` matches the beginning of the string
 * `\$`, `\Z` matches the end of the string
 * `\.` matches any character
 * `\b` matches the empty string at the beginning or end of a word
 * `\B` matches the empty string anywhere but at the beginning or end of a word
 * `\d` matches any Unicode decimal digit
 * `\D` matches any character but a Unicode decimal digit
 * `\s` matches any Unicode whitespace character
 * `\S` matches any character but a Unicode whitespace character
 * `\w` matches Unicode alphabetical characters, Unicode decimal characters and an underscore
 * `\W` matches any character that would not be matched by `\w`