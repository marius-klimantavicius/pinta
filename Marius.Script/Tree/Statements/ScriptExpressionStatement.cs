using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptExpressionStatement: ScriptStatement
    {
        public ScriptExpression Expression { get; set; }

        public ScriptExpressionStatement()
        {
        }

        public ScriptExpressionStatement(ScriptExpression expression)
        {
            Expression = expression;
        }

        public ScriptExpressionStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptExpressionStatement(ScriptExpression expression, ScriptSourceSpan location)
            : base(location)
        {
            Expression = expression;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Expression.WriteDebugView(writer);
            writer.WriteLine(";");
        }
#endif
    }
}
