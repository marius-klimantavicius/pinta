using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptForinStatement: ScriptStatement
    {
        public ScriptForTarget Target { get; set; }
        public ScriptExpression Object { get; set; }
        public ScriptStatement Body { get; set; }

        public ScriptForinStatement()
        {
        }

        public ScriptForinStatement(ScriptForTarget target, ScriptExpression expression, ScriptStatement body)
        {
            Target = target;
            Object = expression;
            Body = body;
        }

        public ScriptForinStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptForinStatement(ScriptForTarget target, ScriptExpression expression, ScriptStatement body, ScriptSourceSpan location)
            : base(location)
        {
            Target = target;
            Object = expression;
            Body = body;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("for (");
            Target.WriteDebugView(writer);
            writer.Write(" in ");
            Object.WriteDebugView(writer);
            writer.WriteLine(")");

            WriteIndentedStatement(Body, writer);
        }
#endif
    }
}
