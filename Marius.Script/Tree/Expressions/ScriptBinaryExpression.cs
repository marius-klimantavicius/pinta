using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Runtime.CompilerServices;

namespace Marius.Script.Tree
{
    public class ScriptBinaryExpression: ScriptExpression
    {
        public ScriptExpression Left { get; set; }
        public ScriptExpression Right { get; set; }
        public ScriptBinaryOperator Operator { get; set; }

        public ScriptBinaryExpression()
        {
        }

        public ScriptBinaryExpression(ScriptExpression left, ScriptExpression right, ScriptBinaryOperator binaryOperator)
        {
            Left = left;
            Right = right;
            Operator = binaryOperator;
        }

        public ScriptBinaryExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptBinaryExpression(ScriptExpression left, ScriptExpression right, ScriptBinaryOperator binaryOperator, ScriptSourceSpan location)
            : base(location)
        {
            Left = left;
            Right = right;
            Operator = binaryOperator;
        }

        public override ScriptType PredictType()
        {
            var left = Left.PredictType();
            var right = Right.PredictType();

            if (left == ScriptType.String && right == ScriptType.String && Operator == ScriptBinaryOperator.Add)
                return ScriptType.String;

            if (left == ScriptType.Numeric || right == ScriptType.Numeric)
                return ScriptType.Numeric;

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
            Left.WriteDebugView(writer);
            writer.Write(") ");

            switch (Operator)
            {
                case ScriptBinaryOperator.Multiply:
                    writer.Write("*");
                    break;
                case ScriptBinaryOperator.Divide:
                    writer.Write("/");
                    break;
                case ScriptBinaryOperator.Modulus:
                    writer.Write("%");
                    break;
                case ScriptBinaryOperator.Add:
                    writer.Write("+");
                    break;
                case ScriptBinaryOperator.Subtract:
                    writer.Write("-");
                    break;
                case ScriptBinaryOperator.ShiftLeft:
                    writer.Write("<<");
                    break;
                case ScriptBinaryOperator.ShiftRight:
                    writer.Write(">>");
                    break;
                case ScriptBinaryOperator.ShiftArithmeticRight:
                    writer.Write(">>>");
                    break;
                case ScriptBinaryOperator.More:
                    writer.Write(">");
                    break;
                case ScriptBinaryOperator.Less:
                    writer.Write("<");
                    break;
                case ScriptBinaryOperator.MoreOrEqual:
                    writer.Write(">=");
                    break;
                case ScriptBinaryOperator.LessOrEqual:
                    writer.Write("<=");
                    break;
                case ScriptBinaryOperator.InstanceOf:
                    writer.Write("instanceof");
                    break;
                case ScriptBinaryOperator.In:
                    writer.Write("in");
                    break;
                case ScriptBinaryOperator.Equals:
                    writer.Write("==");
                    break;
                case ScriptBinaryOperator.NotEquals:
                    writer.Write("!=");
                    break;
                case ScriptBinaryOperator.StrictEquals:
                    writer.Write("===");
                    break;
                case ScriptBinaryOperator.StrictNotEquals:
                    writer.Write("!==");
                    break;
                case ScriptBinaryOperator.BitwiseAnd:
                    writer.Write("&");
                    break;
                case ScriptBinaryOperator.BitwiseOr:
                    writer.Write("|");
                    break;
                case ScriptBinaryOperator.ExclusiveOr:
                    writer.Write("^");
                    break;
                case ScriptBinaryOperator.LogicalAnd:
                    writer.Write("&&");
                    break;
                case ScriptBinaryOperator.LogicalOr:
                    writer.Write("|");
                    break;
            }

            writer.Write(" (");
            Right.WriteDebugView(writer);
            writer.Write(")");
        }
#endif
    }
}
