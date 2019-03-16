using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class SwitchStatement : Statement
    {
        public Expression Discriminant;
        public IEnumerable<SwitchCase> Cases;
    }
}