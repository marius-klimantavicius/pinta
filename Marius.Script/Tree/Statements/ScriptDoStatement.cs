using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptDoStatement: ScriptStatement
    {
        public ScriptStatement Body { get; set; }
        public ScriptExpression Condition { get; set; }

        public ScriptDoStatement()
        {
        }

        public ScriptDoStatement(ScriptStatement body, ScriptExpression condition)
        {
            Body = body;
            Condition = condition;
        }

        public ScriptDoStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptDoStatement(ScriptStatement body, ScriptExpression condition, ScriptSourceSpan location)
            : base(location)
        {
            Body = body;
            Condition = condition;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.WriteLine("do");
            WriteIndentedStatement(Body, writer, false);
            writer.Write("while (");
            Condition.WriteDebugView(writer);
            writer.WriteLine(");");
        }
#endif
    }
}
