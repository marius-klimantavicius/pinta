using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Runtime.CompilerServices;
using System.Reflection;
using System.Dynamic;

namespace Marius.Script.Tree
{
    public class ScriptArrayAccessExpression: ScriptExpression, IScriptReferenceExpression
    {
        public ScriptExpression Array { get; set; }
        public List<ScriptExpression> Indexes { get; private set; }

        public ScriptArrayAccessExpression()
        {
            Indexes = new List<ScriptExpression>();
        }

        public ScriptArrayAccessExpression(ScriptExpression array, IEnumerable<ScriptExpression> indexes)
        {
            Array = array;
            Indexes = new List<ScriptExpression>(indexes);
        }

        public ScriptArrayAccessExpression(ScriptExpression array, ScriptExpression possibleIndexes)
        {
            Array = array;
            if (possibleIndexes is ScriptListExpression)
            {
                Indexes = new List<ScriptExpression>(((ScriptListExpression)possibleIndexes).Expressions);
            }
            else
            {
                Indexes = new List<ScriptExpression>() { possibleIndexes };
            }
        }

        public ScriptArrayAccessExpression(ScriptSourceSpan location)
            : base(location)
        {
            Indexes = new List<ScriptExpression>();
        }

        public ScriptArrayAccessExpression(ScriptExpression array, IEnumerable<ScriptExpression> indexes, ScriptSourceSpan location)
            : base(location)
        {
            Array = array;
            Indexes = new List<ScriptExpression>(indexes);
        }

        public ScriptArrayAccessExpression(ScriptExpression array, ScriptExpression possibleIndexes, ScriptSourceSpan location)
            : base(location)
        {
            Array = array;
            if (possibleIndexes is ScriptListExpression)
            {
                Indexes = new List<ScriptExpression>(((ScriptListExpression)possibleIndexes).Expressions);
            }
            else
            {
                Indexes = new List<ScriptExpression>() { possibleIndexes };
            }
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
            writer.Write("(");
            Array.WriteDebugView(writer);
            writer.Write(")[");

            var first = true;
            foreach (var item in Indexes)
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
