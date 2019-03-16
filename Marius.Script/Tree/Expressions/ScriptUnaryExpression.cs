using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Dynamic;

namespace Marius.Script.Tree
{
    public class ScriptUnaryExpression: ScriptExpression
    {
        public ScriptExpression Operand { get; set; }
        public ScriptUnaryOperator Operator { get; set; }

        public ScriptUnaryExpression()
        {
        }

        public ScriptUnaryExpression(ScriptExpression operand, ScriptUnaryOperator unaryOperator)
        {
            Operand = operand;
            Operator = unaryOperator;
        }

        public ScriptUnaryExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptUnaryExpression(ScriptExpression operand, ScriptUnaryOperator unaryOperator, ScriptSourceSpan location)
            : base(location)
        {
            Operand = operand;
            Operator = unaryOperator;
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
            if (Operator == ScriptUnaryOperator.PostIncrement)
            {
                writer.Write("(");
                Operand.WriteDebugView(writer);
                writer.Write(")");
                writer.Write("++");
            }
            else if (Operator == ScriptUnaryOperator.PostDecrement)
            {
                writer.Write("(");
                Operand.WriteDebugView(writer);
                writer.Write(")");
                writer.Write("--");
            }
            else
            {
                switch (Operator)
                {
                    case ScriptUnaryOperator.PreIncrement:
                        writer.Write("++");
                        break;
                    case ScriptUnaryOperator.PreDecrement:
                        writer.Write("--");
                        break;
                    case ScriptUnaryOperator.Delete:
                        writer.Write("delete ");
                        break;
                    case ScriptUnaryOperator.Void:
                        writer.Write("void ");
                        break;
                    case ScriptUnaryOperator.Typeof:
                        writer.Write("typeof ");
                        break;
                    case ScriptUnaryOperator.Plus:
                        writer.Write("+");
                        break;
                    case ScriptUnaryOperator.Minus:
                        writer.Write("-");
                        break;
                    case ScriptUnaryOperator.BitwiseNot:
                        writer.Write("~");
                        break;
                    case ScriptUnaryOperator.LogicalNot:
                        writer.Write("!");
                        break;
                }

                writer.Write("(");
                Operand.WriteDebugView(writer);
                writer.Write(")");
            }
        }
#endif
    }
}
