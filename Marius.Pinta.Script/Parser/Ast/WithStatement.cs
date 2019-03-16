using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta
{
    public class WithStatement : Statement
    {
        public Expression Object;
        public Statement Body;
    }
}