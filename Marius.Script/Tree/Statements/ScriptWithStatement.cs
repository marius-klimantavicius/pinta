using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptWithStatement: ScriptStatement
    {
        public ScriptExpression Object { get; set; }
        public ScriptStatement Body { get; set; }

        public ScriptWithStatement()
        {
        }

        public ScriptWithStatement(ScriptExpression @object, ScriptStatement body)
        {
            Object = @object;
            Body = body;
        }

        public ScriptWithStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptWithStatement(ScriptExpression @object, ScriptStatement body, ScriptSourceSpan location)
            : base(location)
        {
            Object = @object;
            Body = body;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("with (");
            Object.WriteDebugView(writer);
            writer.WriteLine(")");

            WriteIndentedStatement(Body, writer);
        }
#endif
    }
}
