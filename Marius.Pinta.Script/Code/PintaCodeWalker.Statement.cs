using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script.Code
{
    public partial class PintaCodeWalker
    {
        public virtual void Walk(Statement statement)
        {
            if (statement == null)
                return;

            switch (statement.Type)
            {
                case SyntaxNodes.FunctionDeclaration:
                    WalkFunctionDeclaration(statement.As<FunctionDeclaration>());
                    break;

                case SyntaxNodes.BlockStatement:
                    WalkBlockStatement(statement.As<BlockStatement>());
                    break;

                case SyntaxNodes.BreakStatement:
                    WalkBreakStatement(statement.As<BreakStatement>());
                    break;

                case SyntaxNodes.ContinueStatement:
                    WalkContinueStatement(statement.As<ContinueStatement>());
                    break;

                case SyntaxNodes.DoWhileStatement:
                    WalkDoWhileStatement(statement.As<DoWhileStatement>());
                    break;

                case SyntaxNodes.DebuggerStatement:
                    WalkDebuggerStatement(statement.As<DebuggerStatement>());
                    break;

                case SyntaxNodes.EmptyStatement:
                    WalkEmptyStatement(statement.As<EmptyStatement>());
                    break;

                case SyntaxNodes.ExpressionStatement:
                    WalkExpressionStatement(statement.As<ExpressionStatement>());
                    break;

                case SyntaxNodes.ForStatement:
                    WalkForStatement(statement.As<ForStatement>());
                    break;

                case SyntaxNodes.ForInStatement:
                    WalkForInStatement(statement.As<ForInStatement>());
                    break;

                case SyntaxNodes.IfStatement:
                    WalkIfStatement(statement.As<IfStatement>());
                    break;

                case SyntaxNodes.LabeledStatement:
                    WalkLabelledStatement(statement.As<LabelledStatement>());
                    break;

                case SyntaxNodes.ReturnStatement:
                    WalkReturnStatement(statement.As<ReturnStatement>());
                    break;

                case SyntaxNodes.SwitchStatement:
                    WalkSwitchStatement(statement.As<SwitchStatement>());
                    break;

                case SyntaxNodes.ThrowStatement:
                    WalkThrowStatement(statement.As<ThrowStatement>());
                    break;

                case SyntaxNodes.TryStatement:
                    WalkTryStatement(statement.As<TryStatement>());
                    break;

                case SyntaxNodes.VariableDeclaration:
                    WalkVariableDeclaration(statement.As<VariableDeclaration>());
                    break;

                case SyntaxNodes.WhileStatement:
                    WalkWhileStatement(statement.As<WhileStatement>());
                    break;

                case SyntaxNodes.WithStatement:
                    WalkWithStatement(statement.As<WithStatement>());
                    break;

                case SyntaxNodes.Program:
                    WalkProgram(statement.As<Program>());
                    break;

                default:
                    throw new PintaCompilerException(statement, string.Format("Unsupported statement type {0}", statement.Type));
            }
        }

        public virtual void WalkFunctionDeclaration(FunctionDeclaration functionDeclaration)
        {
        }

        public virtual void Walk(IEnumerable<Statement> statements)
        {
            if (statements == null)
                return;

            foreach (var item in statements)
                Walk(item);
        }

        public virtual void WalkBlockStatement(BlockStatement statement)
        {
            if (statement.Body == null)
                return;

            foreach (var item in statement.Body)
                Walk(item);
        }

        public virtual void WalkBreakStatement(BreakStatement statement)
        {
        }

        public virtual void WalkContinueStatement(ContinueStatement statement)
        {
        }

        public virtual void WalkDoWhileStatement(DoWhileStatement statement)
        {
            Walk(statement.Body);
            Walk(statement.Test);
        }

        public virtual void WalkDebuggerStatement(DebuggerStatement statement)
        {
        }

        public virtual void WalkEmptyStatement(EmptyStatement statement)
        {
        }

        public virtual void WalkExpressionStatement(ExpressionStatement statement)
        {
            Walk(statement.Expression, true);
        }

        public virtual void WalkForStatement(ForStatement statement)
        {
            if (statement.Init != null)
            {
                if (statement.Init.Type == SyntaxNodes.VariableDeclaration)
                    Walk(statement.Init.As<Statement>());
                else
                    Walk(statement.Init.As<Expression>());
            }

            Walk(statement.Test);
            Walk(statement.Body);
            Walk(statement.Update);
        }

        public virtual void WalkForInStatement(ForInStatement statement)
        {
        }

        public virtual void WalkIfStatement(IfStatement statement)
        {
            Walk(statement.Test);
            Walk(statement.Consequent);
            if (statement.Alternate != null)
                Walk(statement.Alternate);
        }

        public virtual void WalkLabelledStatement(LabelledStatement statement)
        {
            if (statement.Body != null)
                Walk(statement.Body);
        }

        public virtual void WalkReturnStatement(ReturnStatement statement)
        {
            if (statement.Argument != null)
                Walk(statement.Argument);
        }

        public virtual void WalkSwitchStatement(SwitchStatement statement)
        {
            Walk(statement.Discriminant);
            foreach (var item in statement.Cases)
            {
                if (item.Test != null)
                    Walk(item.Test);
                Walk(item.Consequent);
            }
        }

        public virtual void WalkThrowStatement(ThrowStatement statement)
        {
        }

        public virtual void WalkTryStatement(TryStatement statement)
        {
        }

        public virtual void WalkVariableDeclaration(VariableDeclaration variable)
        {
            foreach (var item in variable.Declarations)
            {
                if (item.Init != null)
                    Walk(item.Init);
            }
        }

        public virtual void WalkWhileStatement(WhileStatement statement)
        {
            Walk(statement.Test);
            Walk(statement.Body);
        }

        public virtual void WalkWithStatement(WithStatement statement)
        {
        }

        public virtual void WalkProgram(Program program)
        {
            Walk(program.Body);
        }
    }
}
