using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptNameExpression: ScriptExpression, IScriptReferenceExpression
    {
        public ScriptIdentifier Name { get; set; }

        public ScriptNameExpression()
        {
        }

        public ScriptNameExpression(ScriptIdentifier name)
        {
            Name = name;
        }

        public ScriptNameExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptNameExpression(ScriptIdentifier name, ScriptSourceSpan location)
            : base(location)
        {
            Name = name;
        }

        public override ScriptType PredictType()
        {
            return ScriptType.Object;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Name.WriteDebugView(writer);
        }
#endif
    }
}
