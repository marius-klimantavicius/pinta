using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Dynamic;

namespace Marius.Script.Tree
{
    public class ScriptNewExpression: ScriptExpression
    {
        public ScriptExpression TypeExpression { get; set; }
        public List<ScriptExpression> Arguments { get; private set; }

        public ScriptNewExpression()
        {
            Arguments = new List<ScriptExpression>();
        }

        public ScriptNewExpression(ScriptExpression typeExpression)
        {
            TypeExpression = typeExpression;
            Arguments = new List<ScriptExpression>();
        }

        public ScriptNewExpression(ScriptExpression typeExpression, IEnumerable<ScriptExpression> arguments)
        {
            TypeExpression = typeExpression;
            Arguments = new List<ScriptExpression>(arguments);
        }

        public ScriptNewExpression(ScriptSourceSpan location)
            : base(location)
        {
            Arguments = new List<ScriptExpression>();
        }

        public ScriptNewExpression(ScriptExpression typeExpression, ScriptSourceSpan location)
            : base(location)
        {
            TypeExpression = typeExpression;
            Arguments = new List<ScriptExpression>();
        }

        public ScriptNewExpression(ScriptExpression typeExpression, IEnumerable<ScriptExpression> arguments, ScriptSourceSpan location)
            : base(location)
        {
            TypeExpression = typeExpression;
            Arguments = new List<ScriptExpression>(arguments);
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
            writer.Write("new ");
            TypeExpression.WriteDebugView(writer);
            writer.Write("(");

            var first = true;
            foreach (var item in Arguments)
            {
                if (!first)
                    writer.Write(", ");

                item.WriteDebugView(writer);
                first = false;
            }
            writer.Write(")");
        }
#endif
    }
}
