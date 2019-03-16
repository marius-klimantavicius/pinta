using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptProgram: ScriptNode
    {
        public List<ScriptSourceElement> SourceElements { get; private set; }

        public ScriptProgram()
        {
            SourceElements = new List<ScriptSourceElement>();
        }

        public ScriptProgram(ScriptSourceSpan location)
            : base(location)
        {
            SourceElements = new List<ScriptSourceElement>();
        }

        public ScriptProgram(IEnumerable<ScriptSourceElement> sourceElements)
        {
            SourceElements = new List<ScriptSourceElement>(sourceElements);
        }

        public ScriptProgram(IEnumerable<ScriptSourceElement> sourceElements, ScriptSourceSpan location)
            : base(location)
        {
            SourceElements = new List<ScriptSourceElement>(sourceElements);
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            foreach (var item in SourceElements)
            {
                item.WriteDebugView(writer);
            }
        }
#endif
    }
}
