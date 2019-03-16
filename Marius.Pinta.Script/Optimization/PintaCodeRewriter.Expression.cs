using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script.Optimization
{
    public partial class PintaCodeRewriter
    {
        public virtual Expression Rewrite(Expression expression)
        {
            if (expression == null)
                return null;

            switch (expression.Type)
            {
                case SyntaxNodes.AssignmentExpression:
                    return RewriteAssignmentExpression(expression.As<AssignmentExpression>());

                case SyntaxNodes.ArrayExpression:
                    return RewriteArrayExpression(expression.As<ArrayExpression>());

                case SyntaxNodes.BinaryExpression:
                    return RewriteBinaryExpression(expression.As<BinaryExpression>());

                case SyntaxNodes.CallExpression:
                    return RewriteCallExpression(expression.As<CallExpression>());

                case SyntaxNodes.ConditionalExpression:
                    return RewriteConditionalExpression(expression.As<ConditionalExpression>());

                case SyntaxNodes.FunctionExpression:
                    return RewriteFunctionExpression(expression.As<FunctionExpression>());

                case SyntaxNodes.Identifier:
                    return RewriteIdentifier(expression.As<Identifier>());

                case SyntaxNodes.Literal:
                    return RewriteLiteral(expression.As<Literal>());

                case SyntaxNodes.RegularExpressionLiteral:
                    return RewriteLiteral(expression.As<Literal>());

                case SyntaxNodes.LogicalExpression:
                    return RewriteLogicalExpression(expression.As<LogicalExpression>());

                case SyntaxNodes.MemberExpression:
                    return RewriteMemberExpression(expression.As<MemberExpression>());

                case SyntaxNodes.NewExpression:
                    return RewriteNewExpression(expression.As<NewExpression>());

                case SyntaxNodes.ObjectExpression:
                    return RewriteObjectExpression(expression.As<ObjectExpression>());

                case SyntaxNodes.SequenceExpression:
                    return RewriteSequenceExpression(expression.As<SequenceExpression>());

                case SyntaxNodes.ThisExpression:
                    return RewriteThisExpression(expression.As<ThisExpression>());

                case SyntaxNodes.UpdateExpression:
                    return RewriteUpdateExpression(expression.As<UpdateExpression>());

                case SyntaxNodes.UnaryExpression:
                    return RewriteUnaryExpression(expression.As<UnaryExpression>());

                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        public virtual IEnumerable<Expression> Rewrite(IEnumerable<Expression> expressions)
        {
            if (expressions == null)
                return Enumerable.Empty<Expression>();

            var list = new List<Expression>();
            foreach (var item in expressions)
            {
                list.Add(Rewrite(item));
            }
            return list;
        }

        public virtual Expression RewriteAssignmentExpression(AssignmentExpression expression)
        {
            expression.Left = Rewrite(expression.Left);
            expression.Right = Rewrite(expression.Right);
            return expression;
        }

        public virtual Expression RewriteArrayExpression(ArrayExpression expression)
        {
            expression.Elements = Rewrite(expression.Elements);
            return expression;
        }

        public virtual Expression RewriteBinaryExpression(BinaryExpression expression)
        {
            expression.Left = Rewrite(expression.Left);
            expression.Right = Rewrite(expression.Right);
            return expression;
        }

        public virtual Expression RewriteCallExpression(CallExpression expression)
        {
            expression.Callee = Rewrite(expression.Callee);
            expression.Arguments = Rewrite(expression.Arguments);
            return expression;
        }

        public virtual Expression RewriteConditionalExpression(ConditionalExpression expression)
        {
            expression.Test = Rewrite(expression.Test);
            expression.Consequent = Rewrite(expression.Consequent);
            expression.Alternate = Rewrite(expression.Alternate);
            return expression;
        }

        public virtual Expression RewriteFunctionExpression(FunctionExpression expression)
        {
            expression.Body = Rewrite(expression.Body);
            return expression;
        }

        public virtual Expression RewriteIdentifier(Identifier identifier)
        {
            return identifier;
        }

        public virtual Expression RewriteLiteral(Literal literal)
        {
            return literal;
        }

        public virtual Expression RewriteLogicalExpression(LogicalExpression expression)
        {
            expression.Left = Rewrite(expression.Left);
            expression.Right = Rewrite(expression.Right);
            return expression;
        }

        public virtual Expression RewriteMemberExpression(MemberExpression expression)
        {
            expression.Object = Rewrite(expression.Object);
            expression.Property = Rewrite(expression.Property);
            return expression;
        }

        public virtual Expression RewriteNewExpression(NewExpression expression)
        {
            expression.Callee = Rewrite(expression.Callee);
            expression.Arguments = Rewrite(expression.Arguments);
            return expression;
        }

        public virtual Expression RewriteObjectExpression(ObjectExpression expression)
        {
            return expression;
        }

        public virtual Expression RewriteSequenceExpression(SequenceExpression expression)
        {
            expression.Expressions = Rewrite(expression.Expressions).ToList();
            return expression;
        }

        public virtual Expression RewriteThisExpression(ThisExpression expression)
        {
            return expression;
        }

        public virtual Expression RewriteUpdateExpression(UpdateExpression expression)
        {
            expression.Argument = Rewrite(expression.Argument);
            return expression;
        }

        public virtual Expression RewriteUnaryExpression(UnaryExpression expression)
        {
            expression.Argument = Rewrite(expression.Argument);
            return expression;
        }
    }
}
