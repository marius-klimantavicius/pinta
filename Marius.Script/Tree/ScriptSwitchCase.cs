using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public class ScriptSwitchCase: ScriptNode
    {
        public ScriptExpression CaseExpression { get; set; }
        public List<ScriptStatement> Statements { get; private set; }

        public ScriptSwitchCase()
        {
            Statements = new List<ScriptStatement>();
        }

        public ScriptSwitchCase(ScriptExpression caseExpression, IEnumerable<ScriptStatement> statements)
        {
            CaseExpression = caseExpression;
            Statements = new List<ScriptStatement>(statements);
        }

        public ScriptSwitchCase(ScriptSourceSpan location)
            : base(location)
        {
            Statements = new List<ScriptStatement>();
        }

        public ScriptSwitchCase(ScriptExpression caseExpression, IEnumerable<ScriptStatement> statements, ScriptSourceSpan location)
            : base(location)
        {
            CaseExpression = caseExpression;
            Statements = new List<ScriptStatement>(statements);
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            if (CaseExpression == null)
                writer.WriteLine("default:");
            else
            {
                writer.Write("case ");
                CaseExpression.WriteDebugView(writer);
                writer.WriteLine(":");
            }

            writer.Indent++;

            foreach (var item in Statements)
            {
                item.WriteDebugView(writer);
            }

            writer.Indent--;
        }
#endif
    }
}
