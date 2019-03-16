using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class ArrayExpression : Expression
    {
        public IEnumerable<Expression> Elements;
    }
}