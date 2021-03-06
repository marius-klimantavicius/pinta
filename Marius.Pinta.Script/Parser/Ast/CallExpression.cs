using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class CallExpression : Expression
    {
        public Expression Callee;
        public IEnumerable<Expression> Arguments;
    }
}