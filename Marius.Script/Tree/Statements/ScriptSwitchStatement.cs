using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptSwitchStatement: ScriptStatement
    {
        public ScriptExpression Condition { get; set; }
        public List<ScriptSwitchCase> Cases { get; private set; }

        public ScriptSwitchStatement()
        {
            Cases = new List<ScriptSwitchCase>();
        }

        public ScriptSwitchStatement(ScriptExpression condition, IEnumerable<ScriptSwitchCase> cases)
        {
            Condition = condition;
            Cases = new List<ScriptSwitchCase>(cases);
        }

        public ScriptSwitchStatement(ScriptSourceSpan location)
            : base(location)
        {
            Cases = new List<ScriptSwitchCase>();
        }

        public ScriptSwitchStatement(ScriptExpression condition, IEnumerable<ScriptSwitchCase> cases, ScriptSourceSpan location)
            : base(location)
        {
            Condition = condition;
            Cases = new List<ScriptSwitchCase>(cases);
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("switch (");
            Condition.WriteDebugView(writer);
            writer.WriteLine(")");
            writer.WriteLine("{");

            writer.Indent++;

            foreach (var item in Cases)
            {
                item.WriteDebugView(writer);
            }

            writer.Indent--;

            writer.WriteLine("}");
            writer.WriteLine();
        }
#endif
    }
}
