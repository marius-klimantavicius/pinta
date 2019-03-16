using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script
{
    public sealed class PintaCompilerException : Exception
    {
        private SyntaxNode _node;

        public PintaCompilerException(SyntaxNode node, string message) : base(GetMessage(message, node)) { _node = node; }

        private static string GetMessage(string message, SyntaxNode node)
        {
            return string.Format("Error at ({0}, {1}): {2}", node.Location.Start.Line, node.Location.Start.Column, message);
        }
    }
}
