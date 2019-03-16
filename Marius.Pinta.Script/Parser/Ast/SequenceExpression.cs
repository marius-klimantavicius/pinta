using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class SequenceExpression : Expression
    {
        public IList<Expression> Expressions;
    }
}