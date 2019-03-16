using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class ObjectExpression : Expression
    {
        public IEnumerable<Property> Properties;
    }
}