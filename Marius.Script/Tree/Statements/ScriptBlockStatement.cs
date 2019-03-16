using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptBlockStatement: ScriptStatement
    {
        public List<ScriptStatement> Statements { get; private set; }

        public ScriptBlockStatement()
        {
            Statements = new List<ScriptStatement>();
        }

        public ScriptBlockStatement(ScriptSourceSpan location)
            : base(location)
        {
            Statements = new List<ScriptStatement>();
        }

        public ScriptBlockStatement(IEnumerable<ScriptStatement> statements)
        {
            Statements = new List<ScriptStatement>(statements);
        }

        public ScriptBlockStatement(IEnumerable<ScriptStatement> statements, ScriptSourceSpan location)
            : base(location)
        {
            Statements = new List<ScriptStatement>(statements);
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

            foreach (var item in Statements)
            {
                item.WriteDebugView(writer);
            }

            writer.Indent--;
            writer.WriteLine("}");
        }
#endif
    }
}
