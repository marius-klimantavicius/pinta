using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptMemberAccessExpression: ScriptExpression, IScriptReferenceExpression
    {
        public ScriptExpression Object { get; set; }
        public ScriptIdentifier Member { get; set; }

        public ScriptMemberAccessExpression()
        {
        }

        public ScriptMemberAccessExpression(ScriptExpression objectExpression, ScriptIdentifier member)
        {
            Object = objectExpression;
            Member = member;
        }

        public ScriptMemberAccessExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptMemberAccessExpression(ScriptExpression objectExpression, ScriptIdentifier member, ScriptSourceSpan location)
            : base(location)
        {
            Object = objectExpression;
            Member = member;
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
            writer.Write("(");
            Object.WriteDebugView(writer);
            writer.Write(").");
            Member.WriteDebugView(writer);
        }
#endif
    }
}
