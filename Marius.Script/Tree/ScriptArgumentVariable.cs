using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Dynamic;

namespace Marius.Script.Tree
{
    public class ScriptArgumentVariable: ScriptVariable
    {
        public int ArgumentIndex { get; set; }
        
        public override bool HasInitializer
        {
            get { return true; }
        }

        public ScriptArgumentVariable()
        {
        }

        public ScriptArgumentVariable(ScriptIdentifier name, int argumentIndex)
            : base(name)
        {
            ArgumentIndex = argumentIndex;
        }

        public ScriptArgumentVariable(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptArgumentVariable(ScriptIdentifier name, int argumentIndex, ScriptSourceSpan location)
            : base(name, location)
        {
            ArgumentIndex = argumentIndex;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
        }
#endif
    }
}
