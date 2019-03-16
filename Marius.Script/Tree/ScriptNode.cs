using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.CodeDom.Compiler;
using System.IO;

namespace Marius.Script.Tree
{
    public abstract class ScriptNode
    {
        public ScriptSourceSpan Location { get; private set; }

        public ScriptNode()
        {
        }

        public ScriptNode(ScriptSourceSpan location)
        {
            Location = location;
        }

        public abstract void Accept(IScriptVisitor visitor);

#if DEBUG
        public abstract void WriteDebugView(IndentedTextWriter writer);

        public string DebugView
        {
            get
            {
                var sb = new StringBuilder();
                var writer = new IndentedTextWriter(new StringWriter(sb));
                WriteDebugView(writer);
                writer.Flush();
                return sb.ToString();
            }
        }
#endif
    }
}
