using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public class ScriptArrayExpressionElement: ScriptArrayElement
    {
        public ScriptExpression Element { get; set; }

        public ScriptArrayExpressionElement()
        {
        }

        public ScriptArrayExpressionElement(ScriptExpression element)
        {
            Element = element;
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Element.WriteDebugView(writer);
        }
#endif

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
