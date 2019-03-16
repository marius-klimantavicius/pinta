using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptArrayExpression: ScriptExpression
    {
        public List<ScriptArrayElement> Elements { get; private set; }

        public ScriptArrayExpression()
        {
            Elements = new List<ScriptArrayElement>();
        }

        public ScriptArrayExpression(IEnumerable<ScriptArrayElement> elements)
        {
            Elements = new List<ScriptArrayElement>(elements);
        }

        public ScriptArrayExpression(ScriptSourceSpan location)
            : base(location)
        {
            Elements = new List<ScriptArrayElement>();
        }

        public ScriptArrayExpression(IEnumerable<ScriptArrayElement> elements, ScriptSourceSpan location)
            : base(location)
        {
            Elements = new List<ScriptArrayElement>(elements);
        }

        public override ScriptType PredictType()
        {
            return ScriptType.Object;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("[");

            var first = true;
            foreach (var item in Elements)
            {
                if (!first)
                    writer.Write(", ");

                item.WriteDebugView(writer);
                first = false;
            }
            writer.Write("]");
        }
#endif
    }
}
