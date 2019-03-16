using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class NewExpression : Expression
    {
        public Expression Callee;
        public IEnumerable<Expression> Arguments;
    }
}