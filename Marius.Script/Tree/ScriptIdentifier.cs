using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    [DebuggerDisplay("{Name}")]
    public class ScriptIdentifier: ScriptNode
    {
        public string Name { get; private set; }

        public ScriptIdentifier(string name)
        {
            Name = name;
        }

        public ScriptIdentifier(string name, ScriptSourceSpan location)
            : base(location)
        {
            Name = name;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write(Name);
        }
#endif
    }
}
