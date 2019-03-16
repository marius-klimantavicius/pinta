using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using QUT.Gppg;
using System.Text.RegularExpressions;
using Marius.Script.Tree;
using System.Globalization;
using System.Diagnostics;

namespace Marius.Script.Parsing
{
    internal partial class Scanner
    {
        private static readonly Regex _stringEscape = new Regex(@"(\\x(?<hex>[0-9a-fA-F]{2}))|(\\u(?<uni>[0-9a-fA-F]{4}))|(\\(?<line>([\u000A\u2028\u2029]|\u000D|\u000D\u000A)))|(\\(?<char>[^\u000A\u000D\u2028\u2029]))", RegexOptions.Compiled | RegexOptions.ExplicitCapture);
        private static readonly Regex _unicodeEscape = new Regex(@"\\u(?<uni>[0-9a-fA-F]{4})", RegexOptions.Compiled | RegexOptions.ExplicitCapture);

        private bool _isRestricted;
        private bool _lastNumeric;
        private bool _isLineTerminator;
        private bool _isValidAutoSemicolonPosition;

        internal bool IsValidAutoSemicolonPosition
        {
            get { return _isValidAutoSemicolonPosition; }
        }

        public override void yyerror(string format, params object[] args)
        {
            Console.WriteLine("{0}", format);
        }

        internal void PushBack()
        {
            yyless(0);
        }

        private int Token(Tokens token)
        {
            _isRestricted = (token == Tokens.KeyContinue ||
                token == Tokens.KeyBreak ||
                token == Tokens.KeyReturn ||
                token == Tokens.KeyThrow);

            _lastNumeric = token == Tokens.NumericLiteral;

            _isValidAutoSemicolonPosition = _isLineTerminator == true;
            _isLineTerminator = false;

            InitLocation();

            return (int)token;
        }

        private int Token(char token)
        {
            _isRestricted = false;
            _lastNumeric = false;

            _isValidAutoSemicolonPosition = _isLineTerminator == true || token == '}';
            _isLineTerminator = false;

            InitLocation();

            return (int)token;
        }

        private int Identifier()
        {
            var name = yytext;
            if (_lastNumeric)
                Error();

            InitLocation();

            name = _unicodeEscape.Replace(name, m =>
                {
                    var uni = m.Groups["uni"];

                    if (uni.Success)
                    {
                        try
                        {
                            return char.ConvertFromUtf32(Convert.ToInt32(uni.Value, 16));
                        }
                        catch (ArgumentOutOfRangeException)
                        {
                            return new string((char)0xFFFD, 1);
                        }
                    }

                    return m.Value;
                });

            var first = true;
            foreach (var item in name)
            {
                var cat = char.GetUnicodeCategory(item);

                var valid = (cat == UnicodeCategory.UppercaseLetter
                    || cat == UnicodeCategory.LowercaseLetter
                    || cat == UnicodeCategory.TitlecaseLetter
                    || cat == UnicodeCategory.ModifierLetter
                    || cat == UnicodeCategory.OtherLetter
                    || cat == UnicodeCategory.LetterNumber);

                valid |= item == '$';
                valid |= item == '_';

                if (!first)
                {
                    valid |= (cat == UnicodeCategory.NonSpacingMark
                        || cat == UnicodeCategory.SpacingCombiningMark
                        || cat == UnicodeCategory.DecimalDigitNumber
                        || cat == UnicodeCategory.ConnectorPunctuation);

                    valid |= item == '\u200C';
                    valid |= item == '\u200D';
                }

                first = false;

                if (!valid)
                    Error();
            }

            yylval = new ScriptValueType();
            yylval.Identifier = new ScriptIdentifier(name, yylloc);

            return Token(Tokens.Identifier);
        }

        private int String()
        {
            var value = yytext;
            InitLocation();

            if (value.Length > 1 && value[0] == value[value.Length - 1] &&
                (value[0] == '\'' || value[0] == '"'))
                value = value.Substring(1, value.Length - 2);

            value = _stringEscape.Replace(value, m =>
                {
                    var hex = m.Groups["hex"];
                    var uni = m.Groups["uni"];
                    var ch = m.Groups["char"];
                    var line = m.Groups["line"];

                    if (uni.Success || hex.Success)
                    {
                        string code;
                        if (uni.Success)
                            code = uni.Value;
                        else
                            code = hex.Value;

                        try
                        {
                            return char.ConvertFromUtf32(Convert.ToInt32(code, 16));
                        }
                        catch (ArgumentOutOfRangeException)
                        {
                            return new string((char)0xFFFD, 1);
                        }
                    }
                    else if (ch.Success)
                    {
                        Debug.Assert(ch.Length == 1);

                        // ""\\bfnrtv
                        switch (ch.Value[0])
                        {
                            case '"':
                                return "\"";
                            case '\'':
                                return "'";
                            case '\\':
                                return "\\";
                            case 'b':
                                return "\b";
                            case 'f':
                                return "\f";
                            case 'n':
                                return "\n";
                            case 'r':
                                return "\r";
                            case 't':
                                return "\t";
                            case 'v':
                                return "\v";
                            default:
                                Console.WriteLine("Unrecognized character escape");
                                return ch.Value;
                        }
                    }
                    else if (line.Success)
                    {
                        return "";
                    }

                    Console.WriteLine("");
                    return m.Value;
                });

            yylval = new ScriptValueType();
            yylval.Value = value;

            return Token(Tokens.StringLiteral);
        }

        private int Numeric()
        {
            InitLocation();

            yylval = new ScriptValueType();
            yylval.Value = yytext;

            return Token(Tokens.NumericLiteral);
        }

        public int RegexLiteral()
        {
            yyless(0);

            BEGIN(REGEX);

            var result = yylex();

            BEGIN(INITIAL);

            return result;
        }

        private int Regex()
        {
            InitLocation();

            yylval = new ScriptValueType();
            yylval.Value = yytext;

            return Token(Tokens.RegexLiteral);
        }

        private void Error()
        {
            throw new ScriptSyntaxException();
        }

        private void InitLocation()
        {
            yylloc = new ScriptSourceSpan(
                new ScriptSourceLocation(tokLin, tokCol, tokPos),
                new ScriptSourceLocation(tokELin, tokECol, tokEPos));
        }
    }
}
