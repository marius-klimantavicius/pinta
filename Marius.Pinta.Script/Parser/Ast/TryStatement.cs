using System.Collections.Generic;

namespace Marius.Pinta.Script.Parser.Ast
{
    public class TryStatement : Statement
    {
        public Statement Block;
        public IEnumerable<Statement> GuardedHandlers;
        public IEnumerable<CatchClause> Handlers;
        public Statement Finalizer;
    }
}