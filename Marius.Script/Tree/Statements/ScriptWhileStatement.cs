using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptWhileStatement: ScriptStatement
    {
        public ScriptExpression Condition { get; set; }
        public ScriptStatement Body { get; set; }

        public ScriptWhileStatement()
        {
        }

        public ScriptWhileStatement(ScriptExpression condition, ScriptStatement body)
        {
            Condition = condition;
            Body = body;
        }

        public ScriptWhileStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptWhileStatement(ScriptExpression condition, ScriptStatement body, ScriptSourceSpan location)
            : base(location)
        {
            Condition = condition;
            Body = body;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("while (");
            Condition.WriteDebugView(writer);
            writer.WriteLine(")");

            WriteIndentedStatement(Body, writer);
        }
#endif
    }
}
