using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptForStatement: ScriptStatement
    {
        public ScriptForTarget Init { get; set; }
        public ScriptExpression Condition { get; set; }
        public ScriptExpression Step { get; set; }
        public ScriptStatement Body { get; set; }

        public ScriptForStatement()
        {
        }

        public ScriptForStatement(ScriptForTarget init, ScriptExpression condition, ScriptExpression step, ScriptStatement body)
        {
            Init = init;
            Condition = condition;
            Step = step;
            Body = body;
        }

        public ScriptForStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptForStatement(ScriptForTarget init, ScriptExpression condition, ScriptExpression step, ScriptStatement body, ScriptSourceSpan location)
            : base(location)
        {
            Init = init;
            Condition = condition;
            Step = step;
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
            if (Init != null)
                Init.WriteDebugView(writer);

            writer.Write("; ");
            if (Condition != null)
                Condition.WriteDebugView(writer);

            writer.Write("; ");
            if (Step != null)
                Step.WriteDebugView(writer);
            writer.WriteLine(")");

            WriteIndentedStatement(Body, writer);
        }
#endif
    }
}
