using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script.Code
{
    public partial class PintaCodeWalker
    {
        public virtual void Walk(Expression expression, bool discard = false)
        {
            if (expression == null)
                return;

            switch (expression.Type)
            {
                case SyntaxNodes.AssignmentExpression:
                    WalkAssignmentExpression(expression.As<AssignmentExpression>(), discard);
                    break;

                case SyntaxNodes.ArrayExpression:
                    WalkArrayExpression(expression.As<ArrayExpression>(), discard);
                    break;

                case SyntaxNodes.BinaryExpression:
                    WalkBinaryExpression(expression.As<BinaryExpression>(), discard);
                    break;

                case SyntaxNodes.CallExpression:
                    WalkCallExpression(expression.As<CallExpression>(), discard);
                    break;

                case SyntaxNodes.ConditionalExpression:
                    WalkConditionalExpression(expression.As<ConditionalExpression>(), discard);
                    break;

                case SyntaxNodes.FunctionExpression:
                    WalkFunctionExpression(expression.As<FunctionExpression>(), discard);
                    break;

                case SyntaxNodes.Identifier:
                    WalkIdentifier(expression.As<Identifier>(), discard);
                    break;

                case SyntaxNodes.Literal:
                    WalkLiteral(expression.As<Literal>(), discard);
                    break;

                case SyntaxNodes.RegularExpressionLiteral:
                    WalkLiteral(expression.As<Literal>(), discard);
                    break;

                case SyntaxNodes.LogicalExpression:
                    WalkLogicalExpression(expression.As<LogicalExpression>(), discard);
                    break;

                case SyntaxNodes.MemberExpression:
                    WalkMemberExpression(expression.As<MemberExpression>(), discard);
                    break;

                case SyntaxNodes.NewExpression:
                    WalkNewExpression(expression.As<NewExpression>(), discard);
                    break;

                case SyntaxNodes.ObjectExpression:
                    WalkObjectExpression(expression.As<ObjectExpression>(), discard);
                    break;

                case SyntaxNodes.SequenceExpression:
                    WalkSequenceExpression(expression.As<SequenceExpression>(), discard);
                    break;

                case SyntaxNodes.ThisExpression:
                    WalkThisExpression(expression.As<ThisExpression>(), discard);
                    break;

                case SyntaxNodes.UpdateExpression:
                    WalkUpdateExpression(expression.As<UpdateExpression>(), discard);
                    break;

                case SyntaxNodes.UnaryExpression:
                    WalkUnaryExpression(expression.As<UnaryExpression>(), discard);
                    break;

                default:
                    throw new PintaCompilerException(expression, string.Format("Unsupported expression type {0}", expression.Type));
            }
        }

        public virtual void Walk(IEnumerable<Expression> expressions)
        {
            if (expressions == null)
                return;

            foreach (var item in expressions)
                Walk(item);
        }

        public virtual void WalkAssignmentExpression(AssignmentExpression expression, bool discard = false)
        {
            Walk(expression.Left);
            Walk(expression.Right);
        }

        public virtual void WalkArrayExpression(ArrayExpression expression, bool discard = false)
        {
            foreach (var item in expression.Elements)
                Walk(item);
        }

        public virtual void WalkBinaryExpression(BinaryExpression expression, bool discard = false)
        {
            Walk(expression.Left);
            Walk(expression.Right);
        }

        public virtual void WalkCallExpression(CallExpression expression, bool discard = false)
        {
            Walk(expression.Callee);

            foreach (var item in expression.Arguments)
                Walk(item);
        }

        public virtual void WalkConditionalExpression(ConditionalExpression expression, bool discard = false)
        {
            Walk(expression.Test);
            Walk(expression.Consequent);
            Walk(expression.Alternate);
        }

        public virtual void WalkFunctionExpression(FunctionExpression expression, bool discard = false)
        {
        }

        public virtual void WalkIdentifier(Identifier identifier, bool discard = false)
        {
        }

        public virtual void WalkLiteral(Literal literal, bool discard = false)
        {
        }

        public virtual void WalkLogicalExpression(LogicalExpression expression, bool discard = false)
        {
            Walk(expression.Left);
            Walk(expression.Right);
        }

        public virtual void WalkMemberExpression(MemberExpression expression, bool discard)
        {
            Walk(expression.Object);
            Walk(expression.Property);
        }

        public virtual void WalkNewExpression(NewExpression expression, bool discard = false)
        {
            Walk(expression.Callee);

            foreach (var item in expression.Arguments)
                Walk(item);
        }

        public virtual void WalkObjectExpression(ObjectExpression expression, bool discard = false)
        {
            foreach (var item in expression.Properties)
                Walk(item.Value);
        }

        public virtual void WalkSequenceExpression(SequenceExpression expression, bool discard = false)
        {
            for (var i = 0; i < expression.Expressions.Count; i++)
            {
                var item = expression.Expressions[i];
                var discardInner = discard;
                if (!discardInner)
                    discardInner = i < expression.Expressions.Count - 1;

                Walk(item, discardInner);
            }
        }

        public virtual void WalkThisExpression(ThisExpression expression, bool discard = false)
        {
        }

        public virtual void WalkUpdateExpression(UpdateExpression expression, bool discard = false)
        {
            Walk(expression.Argument);
        }

        public virtual void WalkUnaryExpression(UnaryExpression expression, bool discard = false)
        {
            Walk(expression.Argument, discard);
        }
    }
}
