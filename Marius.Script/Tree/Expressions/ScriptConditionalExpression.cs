using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptConditionalExpression: ScriptExpression
    {
        public ScriptExpression Condition { get; set; }
        public ScriptExpression TrueBody { get; set; }
        public ScriptExpression FalseBody { get; set; }

        public ScriptConditionalExpression()
        {
        }

        public ScriptConditionalExpression(ScriptExpression condition, ScriptExpression trueBody, ScriptExpression falseBody)
        {
            Condition = condition;
            TrueBody = trueBody;
            FalseBody = falseBody;
        }

        public ScriptConditionalExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptConditionalExpression(ScriptExpression condition, ScriptExpression trueBody, ScriptExpression falseBody, ScriptSourceSpan location)
            : base(location)
        {
            Condition = condition;
            TrueBody = trueBody;
            FalseBody = falseBody;
        }

        public override ScriptType PredictType()
        {
            var trueType = TrueBody.PredictType();
            var falseType = FalseBody.PredictType();

            if (trueType == falseType)
                return trueType;

            return ScriptType.Object;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("((");
            Condition.WriteDebugView(writer);
            writer.Write(") ? (");
            TrueBody.WriteDebugView(writer);
            writer.Write(") : (");
            FalseBody.WriteDebugView(writer);
            writer.Write("))");
        }
#endif
    }
}
