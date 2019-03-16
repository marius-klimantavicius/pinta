using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Dynamic;

namespace Marius.Script.Tree
{
    public class ScriptObjectExpression: ScriptExpression
    {
        public List<ScriptNameValue> Properties { get; private set; }

        public ScriptObjectExpression()
        {
            Properties = new List<ScriptNameValue>();
        }

        public ScriptObjectExpression(IEnumerable<ScriptNameValue> properties)
        {
            Properties = new List<ScriptNameValue>(properties);
        }

        public ScriptObjectExpression(ScriptSourceSpan location)
            : base(location)
        {
            Properties = new List<ScriptNameValue>();
        }

        public ScriptObjectExpression(IEnumerable<ScriptNameValue> properties, ScriptSourceSpan location)
            : base(location)
        {
            Properties = new List<ScriptNameValue>(properties);
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
            writer.WriteLine("{");

            writer.Indent++;
            for (var i = 0; i < Properties.Count; i++)
            {
                var item = Properties[i];
                item.WriteDebugView(writer);

                if (Properties.Count == (i - 1))
                    writer.WriteLine();
                else
                    writer.WriteLine(",");
            }
            writer.Indent--;

            writer.Write("}");
        }
#endif
    }
}
