using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptListExpression: ScriptExpression
    {
        public List<ScriptExpression> Expressions { get; private set; }

        public ScriptListExpression()
        {
            Expressions = new List<ScriptExpression>();
        }

        public ScriptListExpression(IEnumerable<ScriptExpression> expressions)
        {
            Expressions = new List<ScriptExpression>(expressions);
        }

        public ScriptListExpression(IEnumerable<ScriptExpression> expressions, ScriptExpression next)
        {
            Expressions = new List<ScriptExpression>(expressions);
            Expressions.Add(next);
        }

        public ScriptListExpression(ScriptSourceSpan location)
            : base(location)
        {
            Expressions = new List<ScriptExpression>();
        }

        public ScriptListExpression(IEnumerable<ScriptExpression> expressions, ScriptSourceSpan location)
            : base(location)
        {
            Expressions = new List<ScriptExpression>(expressions);
        }

        public ScriptListExpression(IEnumerable<ScriptExpression> expressions, ScriptExpression next, ScriptSourceSpan location)
            : base(location)
        {
            Expressions = new List<ScriptExpression>(expressions);
            Expressions.Add(next);
        }

        public override ScriptType PredictType()
        {
            if (Expressions.Count > 0)
                return Expressions[Expressions.Count - 1].PredictType();
            return ScriptType.Object;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            var first = true;

            foreach (var item in Expressions)
            {
                if (!first)
                    writer.Write(", ");

                item.WriteDebugView(writer);
                first = false;
            }
        }
#endif
    }
}
