using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public class ScriptArrayGapElement: ScriptArrayElement
    {
        public int Count { get; set; }

        public ScriptArrayGapElement()
        {
        }

        public ScriptArrayGapElement(int count)
        {
            Count = count;
        }
        
#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write(new string(',', Count));
        }
#endif

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
