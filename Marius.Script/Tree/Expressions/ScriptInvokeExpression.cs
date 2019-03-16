using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Dynamic;

namespace Marius.Script.Tree
{
    public class ScriptInvokeExpression: ScriptExpression
    {
        public ScriptExpression Function { get; set; }
        public List<ScriptExpression> Arguments { get; private set; }

        public ScriptInvokeExpression()
        {
            Arguments = new List<ScriptExpression>();
        }

        public ScriptInvokeExpression(ScriptExpression function, IEnumerable<ScriptExpression> arguments)
        {
            Function = function;
            Arguments = new List<ScriptExpression>(arguments);
        }

        public ScriptInvokeExpression(ScriptSourceSpan location)
            : base(location)
        {
            Arguments = new List<ScriptExpression>();
        }

        public ScriptInvokeExpression(ScriptExpression function, IEnumerable<ScriptExpression> arguments, ScriptSourceSpan location)
            : base(location)
        {
            Function = function;
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
            Function.WriteDebugView(writer);
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
