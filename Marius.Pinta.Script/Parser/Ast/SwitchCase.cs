using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class SwitchCase : SyntaxNode
    {
        public Expression Test;
        public IEnumerable<Statement> Consequent;
    }
}