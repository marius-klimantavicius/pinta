using System.Collections.Generic;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script.Parser
{
    public interface IFunctionDeclaration : IFunctionScope
    {
        Identifier Id { get; }
        IEnumerable<Identifier> Parameters { get; }
        Statement Body { get; }
        bool Strict { get; }
    }
}