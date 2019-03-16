%namespace Marius.Script.Parsing
%visibility internal

whitespace                      [[:IsWhiteSpace:]]
newline                         [\u000A\u000D\u2028\u2029]
not_newline                     [^\u000A\u000D\u2028\u2029]

comment_singleline              "//"{not_newline}*

comment_asterisk                "*"
comment_not_slashasterisk       [^*/<<EOF>>]
comment_delimited_text          "/"|{comment_asterisk}*{comment_not_slashasterisk}+
comment_delimited               "/*"{comment_delimited_text}*{comment_asterisk}+"/"
comment_invalid_delimited       "/*"{comment_delimited_text}*<<EOF>>

comment_not_slashasterisknl     [^*/\u000A\u000D\u2028\u2029<<EOF>>]
comment_delimited_sl_text       "/"|{comment_asterisk}*{comment_not_slashasterisknl}+
comment_delimited_sl            "/*"{comment_delimited_sl_text}*{comment_asterisk}+"/"
comment_invalid_delimited_sl    "/*"{comment_delimited_sl_text}*<<EOF>>

decimal_digit                   [0-9]
nonzero_digit                   [1-9]
hex_digit                       [0-9a-fA-F]

decimal_exponent_part           [eE]("+"|"-")?{decimal_digit}+
decimal_integer_literal         (0|{nonzero_digit}{decimal_digit}*)
decimal_literal                 ({decimal_integer_literal}"."{decimal_digit}*{decimal_exponent_part}?|"."{decimal_digit}+{decimal_exponent_part}?|{decimal_integer_literal}{decimal_exponent_part}?)

hex_integer_literal             0[xX]{hex_digit}+

numeric_literal                 {decimal_literal}|{hex_integer_literal}

/* unicode/hex/char escape sequences are dealt with in semantic action */
escape_char_sequence            [^\u000A\u000D\u2028\u2029]
escape_sequence                 {escape_char_sequence}

string_cr_no_nl                 \u000D/[^\u000A]
string_line_continuation        (\\[\u000A\u2028\u2029])|(\\\u000D\u000A)|(\\{string_cr_no_nl})

string_double_char              ([^"\\\u000A\u000D\u2028\u2029]|\\{escape_sequence}|{string_line_continuation})
string_double                   \"{string_double_char}*\"

string_single_char              ([^'\\\u000A\u000D\u2028\u2029]|\\{escape_sequence}|{string_line_continuation})
string_single                   \'{string_single_char}*\'

string_invalid_single           \'{string_single_char}*(<<EOF>>|{newline})
string_invalid_double           \"{string_double_char}*(<<EOF>>|{newline})

string_literal                  {string_double}|{string_single}
string_invalid                  {string_invalid_single}|{string_invalid_double}

/* identifier start and part do not include {escape_sequence} consume invalid identifiers as well (error should be reported at sematic action level) */
identifier_start                [[:IdentifierStartCharacter:]]|"$"|"_"|\\
identifier_part                 [[:IdentifierPartCharacter:]]|"$"|"_"|\\|[\u200C\u200D]

regex_nonterminal               {not_newline}
regex_backslash_sequence        \\{regex_nonterminal}
regex_class_char                [^\u000A\u000D\u2028\u2029\]\\]|{regex_backslash_sequence}
regex_class                     "["{regex_class_char}*"]"
regex_first                     [^\u000A\u000D\u2028\u2029*\\/\[]|{regex_backslash_sequence}|{regex_class}
regex_char                      [^\u000A\u000D\u2028\u2029\\/\[]|{regex_backslash_sequence}|{regex_class}
regex_body                      {regex_first}{regex_char}*
regex_flags                     {identifier_part}*
regex                           "/"{regex_body}"/"{regex_flags}

%x REGEX

%%

{comment_singleline}            _lastNumeric = false;
{comment_delimited_sl}          _lastNumeric = false;
{comment_delimited}             _isLineTerminator = true; _lastNumeric = false; if (_isRestricted) return Token(';');

{comment_invalid_delimited}     Error();
{comment_invalid_delimited_sl}  Error();

{numeric_literal}               return Numeric();
{string_literal}                return String();
{string_invalid}                Error();

"{"                             return Token('{');
"}"                             return Token('}');
"["                             return Token('[');
"]"                             return Token(']');
"("                             return Token('(');
")"                             return Token(')');
"."                             return Token('.');
","                             return Token(',');
":"                             return Token(':');
";"                             return Token(';');
"+"                             return Token('+');
"-"                             return Token('-');
"*"                             return Token('*');
"/"                             return Token('/');
"%"                             return Token('%');
"&"                             return Token('&');
"|"                             return Token('|');
"^"                             return Token('^');
"!"                             return Token('!');
"~"                             return Token('~');
"="                             return Token('=');
"<"                             return Token('<');
">"                             return Token('>');
"?"                             return Token('?');
"++"                            if (_isLineTerminator) { yyless(0); return Token(';'); } return Token(Tokens.Increment);
"--"                            if (_isLineTerminator) { yyless(0); return Token(';'); } return Token(Tokens.Decrement);
"&&"                            return Token(Tokens.LogicalAnd);
"||"                            return Token(Tokens.LogicalOr);
"=="                            return Token(Tokens.LogicalEqual);
"!="                            return Token(Tokens.LogicalNotEqual);
"<="                            return Token(Tokens.LessEqual);
">="                            return Token(Tokens.MoreEqual);
"+="                            return Token(Tokens.AddAssign);
"-="                            return Token(Tokens.SubtractAssign);
"*="                            return Token(Tokens.MultiplyAssign);
"/="                            return Token(Tokens.DivideAssign);
"%="                            return Token(Tokens.RemainderAssign);
"&="                            return Token(Tokens.AndAssign);
"|="                            return Token(Tokens.OrAssign);
"^="                            return Token(Tokens.ExclusiveOrAssign);
"<<"                            return Token(Tokens.ShiftLeft);
"<<="                           return Token(Tokens.ShiftLeftAssign);
"=>"                            return Token(Tokens.Arrow);
">>"                            return Token(Tokens.ShiftRight);
">>>"                           return Token(Tokens.ShiftArithmeticRight);
">>="                           return Token(Tokens.ShiftRightAssign);
">>>="                          return Token(Tokens.ShiftArithmeticRightAssign);
"==="                           return Token(Tokens.ExactEqual);
"!=="                           return Token(Tokens.ExactNotEqual);

"true"                          return Token(Tokens.KeyTrue);
"false"                         return Token(Tokens.KeyFalse);
"null"                          return Token(Tokens.KeyNull);

"break"		                    return Token(Tokens.KeyBreak);
"for"		                    return Token(Tokens.KeyFor);
"throw"		                    return Token(Tokens.KeyThrow);
"case"		                    return Token(Tokens.KeyCase);
"function"		                return Token(Tokens.KeyFunction);
"try"		                    return Token(Tokens.KeyTry);
"catch"		                    return Token(Tokens.KeyCatch);
"if"		                    return Token(Tokens.KeyIf);
"typeof"		                return Token(Tokens.KeyTypeof);
"continue"		                return Token(Tokens.KeyContinue);
"in"		                    return Token(Tokens.KeyIn);
"var"		                    return Token(Tokens.KeyVar);
"default"		                return Token(Tokens.KeyDefault);
"instanceof"	                return Token(Tokens.KeyInstanceof);
"void"		                    return Token(Tokens.KeyVoid);
"delete"		                return Token(Tokens.KeyDelete);
"new"		                    return Token(Tokens.KeyNew);
"do"		                    return Token(Tokens.KeyDo);
"return"		                return Token(Tokens.KeyReturn);
"while"		                    return Token(Tokens.KeyWhile);
"else"		                    return Token(Tokens.KeyElse);
"switch"		                return Token(Tokens.KeySwitch);
"with"		                    return Token(Tokens.KeyWith);
"finally"		                return Token(Tokens.KeyFinally);
"this"		                    return Token(Tokens.KeyThis);
"debugger"		                return Token(Tokens.KeyDebugger);

"class"		                    return Token(Tokens.KeyClass);
"enum"		                    return Token(Tokens.KeyEnum);
"extends"		                return Token(Tokens.KeyExtends);
"super"		                    return Token(Tokens.KeySuper);
"const"		                    return Token(Tokens.KeyConst);
"import"		                return Token(Tokens.KeyImport);
"export"		                return Token(Tokens.KeyExport);
"implements"	                return Token(Tokens.KeyImplements);
"interface"		                return Token(Tokens.KeyInterface);
"package"		                return Token(Tokens.KeyPackage);
"yield"                         return Token(Tokens.KeyYield);
"let"                           return Token(Tokens.KeyLet);
"private"		                return Token(Tokens.KeyPrivate);
"protected"		                return Token(Tokens.KeyProtected);
"public"		                return Token(Tokens.KeyPublic);
"static"		                return Token(Tokens.KeyStatic);

"set"                           return Token(Tokens.KeySet);
"get"                           return Token(Tokens.KeyGet);

{identifier_start}{identifier_part}* return Identifier();

{newline}+                      _lastNumeric = false; _isLineTerminator = true; if (_isRestricted) return Token(';');
{whitespace}                    _lastNumeric = false;

<<EOF>>                         _isLineTerminator = true; _isValidAutoSemicolonPosition = true; if (_isRestricted) return Token(';');

.                               Error();

<REGEX>
{

{regex}                         return Regex();
.                               Error();

}