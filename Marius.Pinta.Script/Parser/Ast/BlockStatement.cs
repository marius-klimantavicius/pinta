using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class BlockStatement : Statement
    {
        public IEnumerable<Statement> Body;
    }
}