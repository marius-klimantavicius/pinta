using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptVarStatement: ScriptStatement
    {
        public List<ScriptVariable> Variables { get; private set; }

        public ScriptVarStatement()
        {
            Variables = new List<ScriptVariable>();
        }

        public ScriptVarStatement(IEnumerable<ScriptVariable> variables)
        {
            Variables = new List<ScriptVariable>(variables);
        }

        public ScriptVarStatement(ScriptSourceSpan location)
            : base(location)
        {
            Variables = new List<ScriptVariable>();
        }

        public ScriptVarStatement(IEnumerable<ScriptVariable> variables, ScriptSourceSpan location)
            : base(location)
        {
            Variables = new List<ScriptVariable>(variables);
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            var first = true;
            writer.Write("var ");

            foreach (var item in Variables)
            {
                if (!first)
                    writer.Write(", ");

                item.WriteDebugView(writer);

                first = false;
            }

            writer.WriteLine(";");
        }
#endif
    }
}
