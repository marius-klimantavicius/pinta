using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public class ScriptForVarTarget: ScriptForTarget
    {
        public List<ScriptVariable> Variables { get; private set; }

        public ScriptForVarTarget()
        {
            Variables = new List<ScriptVariable>();
        }

        public ScriptForVarTarget(IEnumerable<ScriptVariable> variables)
        {
            Variables = new List<ScriptVariable>(variables);
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
        }
#endif

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
