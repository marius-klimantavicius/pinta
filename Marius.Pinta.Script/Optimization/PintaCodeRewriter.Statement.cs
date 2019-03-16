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
        public virtual Statement Rewrite(Statement statement)
        {
            if (statement == null)
                return null;

            switch (statement.Type)
            {
                case SyntaxNodes.FunctionDeclaration:
                    return statement;

                case SyntaxNodes.BlockStatement:
                    return RewriteBlockStatement(statement.As<BlockStatement>());
                    
                case SyntaxNodes.BreakStatement:
                    return RewriteBreakStatement(statement.As<BreakStatement>());
                    
                case SyntaxNodes.ContinueStatement:
                    return RewriteContinueStatement(statement.As<ContinueStatement>());
                    
                case SyntaxNodes.DoWhileStatement:
                    return RewriteDoWhileStatement(statement.As<DoWhileStatement>());
                    
                case SyntaxNodes.DebuggerStatement:
                    return RewriteDebuggerStatement(statement.As<DebuggerStatement>());
                    
                case SyntaxNodes.EmptyStatement:
                    return RewriteEmptyStatement(statement.As<EmptyStatement>());
                    
                case SyntaxNodes.ExpressionStatement:
                    return RewriteExpressionStatement(statement.As<ExpressionStatement>());
                    
                case SyntaxNodes.ForStatement:
                    return RewriteForStatement(statement.As<ForStatement>());
                    
                case SyntaxNodes.ForInStatement:
                    return RewriteForInStatement(statement.As<ForInStatement>());
                    
                case SyntaxNodes.IfStatement:
                    return RewriteIfStatement(statement.As<IfStatement>());
                    
                case SyntaxNodes.LabeledStatement:
                    return RewriteLabelledStatement(statement.As<LabelledStatement>());
                    
                case SyntaxNodes.ReturnStatement:
                    return RewriteReturnStatement(statement.As<ReturnStatement>());
                    
                case SyntaxNodes.SwitchStatement:
                    return RewriteSwitchStatement(statement.As<SwitchStatement>());
                    
                case SyntaxNodes.ThrowStatement:
                    return RewriteThrowStatement(statement.As<ThrowStatement>());
                    
                case SyntaxNodes.TryStatement:
                    return RewriteTryStatement(statement.As<TryStatement>());
                    
                case SyntaxNodes.VariableDeclaration:
                    return RewriteVariableDeclaration(statement.As<VariableDeclaration>());
                    
                case SyntaxNodes.WhileStatement:
                    return RewriteWhileStatement(statement.As<WhileStatement>());
                    
                case SyntaxNodes.WithStatement:
                    return RewriteWithStatement(statement.As<WithStatement>());
                    
                case SyntaxNodes.Program:
                    return RewriteProgram(statement.As<Program>());
                    
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        public virtual IEnumerable<Statement> Rewrite(IEnumerable<Statement> statements)
        {
            if (statements == null)
                return Enumerable.Empty<Statement>();

            var list = new List<Statement>();
            foreach (var item in statements)
            {
                list.Add(Rewrite(item));
            }
            return list;
        }

        public virtual Statement RewriteBlockStatement(BlockStatement statement)
        {
            if (statement.Body == null)
                return statement;

            statement.Body = Rewrite(statement.Body);
            return statement;
        }

        public virtual Statement RewriteBreakStatement(BreakStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteContinueStatement(ContinueStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteDoWhileStatement(DoWhileStatement statement)
        {
            statement.Test = Rewrite(statement.Test);
            statement.Body = Rewrite(statement.Body);

            return statement;
        }

        public virtual Statement RewriteDebuggerStatement(DebuggerStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteEmptyStatement(EmptyStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteExpressionStatement(ExpressionStatement statement)
        {
            statement.Expression = Rewrite(statement.Expression);
            return statement;
        }

        public virtual Statement RewriteForStatement(ForStatement statement)
        {
            if (statement.Init != null)
            {
                if (statement.Init.Type == SyntaxNodes.VariableDeclaration)
                    statement.Init = Rewrite(statement.Init.As<Statement>());
                else
                    statement.Init = Rewrite(statement.Init.As<Expression>());
            }

            statement.Test = Rewrite(statement.Test);
            statement.Update = Rewrite(statement.Update);
            statement.Body = Rewrite(statement.Body);

            return statement;
        }

        public virtual Statement RewriteForInStatement(ForInStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteIfStatement(IfStatement statement)
        {
            statement.Test = Rewrite(statement.Test);
            statement.Consequent = Rewrite(statement.Consequent);
            statement.Alternate = Rewrite(statement.Alternate);
            return statement;
        }

        public virtual Statement RewriteLabelledStatement(LabelledStatement statement)
        {
            statement.Body = Rewrite(statement.Body);
            return statement;
        }

        public virtual Statement RewriteReturnStatement(ReturnStatement statement)
        {
            statement.Argument = Rewrite(statement.Argument);
            return statement;
        }

        public virtual Statement RewriteSwitchStatement(SwitchStatement statement)
        {
            statement.Discriminant = Rewrite(statement.Discriminant);
            return statement;
        }

        public virtual Statement RewriteThrowStatement(ThrowStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteTryStatement(TryStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteVariableDeclaration(VariableDeclaration statement)
        {
            return statement;
        }

        public virtual Statement RewriteWhileStatement(WhileStatement statement)
        {
            statement.Test = Rewrite(statement.Test);
            statement.Body = Rewrite(statement.Body);
            return statement;
        }

        public virtual Statement RewriteWithStatement(WithStatement statement)
        {
            return statement;
        }

        public virtual Statement RewriteProgram(Program statement)
        {
            statement.Body = Rewrite(statement.Body);
            return statement;
        }
    }
}
