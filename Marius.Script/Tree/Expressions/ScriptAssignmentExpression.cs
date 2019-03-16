using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptAssignmentExpression: ScriptExpression
    {
        public ScriptExpression Left { get; set; }
        public ScriptExpression Right { get; set; }
        public ScriptAssignmentOperator Operator { get; set; }

        public ScriptAssignmentExpression()
        {
        }

        public ScriptAssignmentExpression(ScriptExpression left, ScriptExpression right, ScriptAssignmentOperator assignmentOperator)
        {
            Left = left;
            Right = right;
            Operator = assignmentOperator;
        }

        public ScriptAssignmentExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptAssignmentExpression(ScriptExpression left, ScriptExpression right, ScriptAssignmentOperator assignmentOperator, ScriptSourceSpan location)
            : base(location)
        {
            Left = left;
            Right = right;
            Operator = assignmentOperator;
        }

        public override ScriptType PredictType()
        {
            return Right.PredictType();
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("(");
            Left.WriteDebugView(writer);
            writer.Write(") ");

            switch (Operator)
            {
                case ScriptAssignmentOperator.Assign:
                    writer.Write("=");
                    break;
                case ScriptAssignmentOperator.MultiplyAssign:
                    writer.Write("*=");
                    break;
                case ScriptAssignmentOperator.DivideAssign:
                    writer.Write("/=");
                    break;
                case ScriptAssignmentOperator.ModulusAssign:
                    writer.Write("%=");
                    break;
                case ScriptAssignmentOperator.AddAssign:
                    writer.Write("+=");
                    break;
                case ScriptAssignmentOperator.SubtractAssign:
                    writer.Write("-=");
                    break;
                case ScriptAssignmentOperator.ShiftLeftAssign:
                    writer.Write("<<=");
                    break;
                case ScriptAssignmentOperator.ShiftRightAssign:
                    writer.Write(">>=");
                    break;
                case ScriptAssignmentOperator.ShiftArithmeticRightAssign:
                    writer.Write(">>>=");
                    break;
                case ScriptAssignmentOperator.BitwiseAndAssign:
                    writer.Write("&=");
                    break;
                case ScriptAssignmentOperator.BitwiseOrAssign:
                    writer.Write("|=");
                    break;
                case ScriptAssignmentOperator.ExclusiveOrAssign:
                    writer.Write("^=");
                    break;
            }

            writer.Write(" (");
            Right.WriteDebugView(writer);
            writer.Write(")");
        }
#endif
    }
}
