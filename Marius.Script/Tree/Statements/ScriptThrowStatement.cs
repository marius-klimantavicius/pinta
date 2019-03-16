using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptThrowStatement: ScriptStatement
    {
        public ScriptExpression Exception { get; set; }

        public ScriptThrowStatement()
        {
        }

        public ScriptThrowStatement(ScriptExpression exception)
        {
            Exception = exception;
        }

        public ScriptThrowStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptThrowStatement(ScriptExpression exception, ScriptSourceSpan location)
            : base(location)
        {
            Exception = exception;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("throw ");
            Exception.WriteDebugView(writer);
            writer.WriteLine(";");
        }
#endif
    }
}
