using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Marius.Script.Tree;

namespace Marius.Script.Parsing
{
    internal partial class Parser
    {
        public ScriptProgram Program { get; private set; }

        public Parser(Scanner scanner)
            : base(scanner)
        {
        }

        protected override void CustomParserAction(ref int nextToken, ref int action, QUT.Gppg.State state)
        {
            if (action == 0 && Scanner is Scanner)
            {
                var s = (Scanner)Scanner;

                var token = (int)Tokens.AutoSemicolon;
                if (s.IsValidAutoSemicolonPosition
                    && state.ParserTable.ContainsKey(token)
                    && state.ParserTable[token] != 0)
                {
                    nextToken = token;
                    s.PushBack();
                    action = state.ParserTable[token];
                }
            }
        }

        private ScriptExpression MakeListExpression(ScriptExpression first, ScriptExpression second, ScriptSourceSpan location)
        {
            var sle = first as ScriptListExpression;
            if (sle == null)
                return new ScriptListExpression(new[] { first, second }, location);
            else
                return new ScriptListExpression(sle.Expressions, second, location);
        }

        private ScriptLiteralExpression RegexLiteral()
        {
            var s = Scanner as Scanner;

            if (s == null)
                throw new ScriptSyntaxException();

            var token = (Tokens)s.RegexLiteral();
            if (token != Tokens.RegexLiteral)
                throw new ScriptSyntaxException();

            return ScriptLiteralExpression.Regex(s.yytext, s.yylloc);
        }
    }
}
