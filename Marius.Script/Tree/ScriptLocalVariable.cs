using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptLocalVariable: ScriptVariable
    {
        public ScriptExpression Initializer { get; set; }

        public override bool HasInitializer
        {
            get { return Initializer != null; }
        }

        public ScriptLocalVariable()
        {
        }

        public ScriptLocalVariable(ScriptIdentifier name, ScriptExpression initializer)
            : base(name)
        {
            Initializer = initializer;
        }

        public ScriptLocalVariable(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptLocalVariable(ScriptIdentifier name, ScriptExpression initializer, ScriptSourceSpan location)
            : base(name, location)
        {
            Initializer = initializer;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Name.WriteDebugView(writer);

            if (Initializer != null)
            {
                writer.Write(" = ");
                Initializer.WriteDebugView(writer);
            }
        }
#endif
    }
}
