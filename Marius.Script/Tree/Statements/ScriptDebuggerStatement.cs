using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptDebuggerStatement: ScriptStatement
    {
        public ScriptDebuggerStatement()
        {
        }

        public ScriptDebuggerStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.WriteLine("debugger;");
        }
#endif
    }
}
