using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptIfStatement: ScriptStatement
    {
        public ScriptExpression Condition { get; set; }
        public ScriptStatement TrueBody { get; set; }
        public ScriptStatement FalseBody { get; set; }

        public ScriptIfStatement()
        {
        }

        public ScriptIfStatement(ScriptExpression condition, ScriptStatement trueBody, ScriptStatement falseBody)
        {
            Condition = condition;
            TrueBody = trueBody;
            FalseBody = falseBody;
        }

        public ScriptIfStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptIfStatement(ScriptExpression condition, ScriptStatement trueBody, ScriptStatement falseBody, ScriptSourceSpan location)
            : base(location)
        {
            Condition = condition;
            TrueBody = trueBody;
            FalseBody = falseBody;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("if (");
            Condition.WriteDebugView(writer);
            writer.WriteLine(")");

            WriteIndentedStatement(TrueBody, writer, false);

            if (FalseBody != null)
            {
                writer.WriteLine("else");
                WriteIndentedStatement(FalseBody, writer, false);
            }

            writer.WriteLine();
        }
#endif
    }
}
