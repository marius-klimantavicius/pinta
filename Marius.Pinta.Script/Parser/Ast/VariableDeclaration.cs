using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class VariableDeclaration : Statement
    {
        public IEnumerable<VariableDeclarator> Declarations;
        public string Kind;
    }
}