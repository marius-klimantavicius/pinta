using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptReturnStatement: ScriptStatement
    {
        public ScriptExpression Value { get; set; }

        public ScriptReturnStatement()
        {
        }

        public ScriptReturnStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptReturnStatement(ScriptExpression value)
        {
            Value = value;
        }

        public ScriptReturnStatement(ScriptExpression value, ScriptSourceSpan location)
            : base(location)
        {
            Value = value;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("return");
            if (Value != null)
            {
                writer.Write(" ");
                Value.WriteDebugView(writer);
            }
            writer.WriteLine(";");
        }
#endif
    }
}
