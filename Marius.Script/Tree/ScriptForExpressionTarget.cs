using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public class ScriptForExpressionTarget: ScriptForTarget
    {
        public ScriptExpression Expression { get; set; }

        public ScriptForExpressionTarget()
        {
        }

        public ScriptForExpressionTarget(ScriptExpression expression)
        {
            Expression = expression;
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Expression.WriteDebugView(writer);
        }
#endif

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
