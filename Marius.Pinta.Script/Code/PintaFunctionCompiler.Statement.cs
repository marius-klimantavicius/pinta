using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public partial class PintaFunctionCompiler
    {
        public virtual void Emit(Statement statement)
        {
            switch (statement.Type)
            {
                case SyntaxNodes.FunctionDeclaration:
                    break;

                case SyntaxNodes.BlockStatement:
                    EmitBlockStatement(statement.As<BlockStatement>());
                    break;

                case SyntaxNodes.BreakStatement:
                    EmitBreakStatement(statement.As<BreakStatement>());
                    break;

                case SyntaxNodes.ContinueStatement:
                    EmitContinueStatement(statement.As<ContinueStatement>());
                    break;

                case SyntaxNodes.DoWhileStatement:
                    EmitDoWhileStatement(statement.As<DoWhileStatement>());
                    break;

                case SyntaxNodes.DebuggerStatement:
                    EmitDebuggerStatement(statement.As<DebuggerStatement>());
                    break;

                case SyntaxNodes.EmptyStatement:
                    EmitEmptyStatement(statement.As<EmptyStatement>());
                    break;

                case SyntaxNodes.ExpressionStatement:
                    EmitExpressionStatement(statement.As<ExpressionStatement>());
                    break;

                case SyntaxNodes.ForStatement:
                    EmitForStatement(statement.As<ForStatement>());
                    break;

                case SyntaxNodes.ForInStatement:
                    EmitForInStatement(statement.As<ForInStatement>());
                    break;

                case SyntaxNodes.IfStatement:
                    EmitIfStatement(statement.As<IfStatement>());
                    break;

                case SyntaxNodes.LabeledStatement:
                    EmitLabelledStatement(statement.As<LabelledStatement>());
                    break;

                case SyntaxNodes.ReturnStatement:
                    EmitReturnStatement(statement.As<ReturnStatement>());
                    break;

                case SyntaxNodes.SwitchStatement:
                    EmitSwitchStatement(statement.As<SwitchStatement>());
                    break;

                case SyntaxNodes.ThrowStatement:
                    EmitThrowStatement(statement.As<ThrowStatement>());
                    break;

                case SyntaxNodes.TryStatement:
                    EmitTryStatement(statement.As<TryStatement>());
                    break;

                case SyntaxNodes.VariableDeclaration:
                    EmitVariableDeclaration(statement.As<VariableDeclaration>());
                    break;

                case SyntaxNodes.WhileStatement:
                    EmitWhileStatement(statement.As<WhileStatement>());
                    break;

                case SyntaxNodes.WithStatement:
                    EmitWithStatement(statement.As<WithStatement>());
                    break;

                case SyntaxNodes.Program:
                    EmitProgram(statement.As<Program>());
                    break;

                default:
                    throw new PintaCompilerException(statement, string.Format("Unsupported statement type {0}", statement.Type));
            }
        }

        public virtual void Emit(IEnumerable<Statement> statements)
        {
            if (statements == null)
                return;

            var last = default(Statement);
            foreach (var item in statements)
            {
                Emit(item);
                last = item;
            }

            if (last == null || last.Type != SyntaxNodes.ReturnStatement)
            {
                _code.Emit(PintaCode.LoadNull);
                _code.Emit(PintaCode.Return);
            }
        }

        public virtual void EmitFunctionDeclaration(PintaFunctionLocal local, PintaFunction inner)
        {
            var mode = _code.Mode;

            _code.Mode = PintaCodeGeneratorMode.Prologue;
            _code.Emit(PintaCode.NewFunction, inner.Function);
            _code.Emit(PintaCode.StoreLocal, local.Local);
            _code.Mode = mode;
        }

        public virtual void EmitBlockStatement(BlockStatement statement)
        {
            if (statement.Body == null)
                return;

            foreach (var item in statement.Body)
            {
                Emit(item);
            }
        }

        public virtual void EmitBreakStatement(BreakStatement statement)
        {
            var label = default(PintaCodeLabel);

            if (statement.Label != null && statement.Label.Name != null)
                label = _labelScope.GetBreak(statement.Label.Name);
            else
                label = _labelScope.GetBreak();

            if (label == null)
                throw new PintaCompilerException(statement, "'break' must have a valid target - either must be in a loop, 'switch' statement or reference a valid label within function");

            _code.Emit(PintaCode.Jump, label);
        }

        public virtual void EmitContinueStatement(ContinueStatement statement)
        {
            var label = default(PintaCodeLabel);

            if (statement.Label != null && statement.Label.Name != null)
                label = _labelScope.GetContinue(statement.Label.Name);
            else
                label = _labelScope.GetContinue();

            if (label == null)
                throw new PintaCompilerException(statement, "'continue' must have a valid target - either must be in a loop or reference a valid label within function");

            _code.Emit(PintaCode.Jump, label);
        }

        public virtual void EmitDoWhileStatement(DoWhileStatement statement)
        {
            var startLabel = _code.DefineLabel();
            var breakLabel = _code.DefineLabel();
            var continueLabel = _code.DefineLabel();

            using (_labelScope.Push(statement.LabelSet, breakLabel, continueLabel))
            {

                /*
                 * do { body } while(test);
                 * start:
                 * body
                 * continue:
                 * if (test)
                 *  goto start
                 * break:
                 */

                _code.MarkLabel(startLabel);
                Emit(statement.Body);

                _code.MarkLabel(continueLabel);
                Emit(statement.Test);

                _code.Emit(PintaCode.JumpNotZero, startLabel);
                _code.MarkLabel(breakLabel);
            }
        }

        public virtual void EmitDebuggerStatement(DebuggerStatement statement)
        {
        }

        public virtual void EmitEmptyStatement(EmptyStatement statement)
        {
        }

        public virtual void EmitExpressionStatement(ExpressionStatement statement)
        {
            Emit(statement.Expression, true);
        }

        public virtual void EmitForStatement(ForStatement statement)
        {
            var startLabel = _code.DefineLabel();
            var breakLabel = _code.DefineLabel();
            var continueLabel = _code.DefineLabel();

            using (_labelScope.Push(statement.LabelSet, breakLabel, continueLabel))
            {
                /*
                 * for (init; test; update) body
                 * 
                 * init
                 * 
                 * start:
                 * if (!test)
                 * goto break;
                 * 
                 * body
                 * 
                 * continue:
                 * update
                 * goto start;
                 * 
                 * break:
                 * 
                 */

                if (statement.Init != null)
                {
                    if (statement.Init.Type == SyntaxNodes.VariableDeclaration)
                    {
                        Emit(statement.Init.As<Statement>());
                    }
                    else
                    {
                        Emit(statement.Init.As<Expression>());
                        _code.Emit(PintaCode.Pop);
                    }
                }

                _code.MarkLabel(startLabel);

                Emit(statement.Test);
                _code.Emit(PintaCode.JumpZero, breakLabel);

                Emit(statement.Body);

                _code.MarkLabel(continueLabel);
                Emit(statement.Update);
                _code.Emit(PintaCode.Pop);
                _code.Emit(PintaCode.Jump, startLabel);

                _code.MarkLabel(breakLabel);
            }
        }

        public virtual void EmitForInStatement(ForInStatement statement)
        {
            throw new PintaCompilerException(statement, "Statement 'for(... in ...)' is not supported");
        }

        public virtual void EmitIfStatement(IfStatement statement)
        {
            if (statement.Alternate == null)
            {
                var falseLabel = _code.DefineLabel();

                Emit(statement.Test);
                _code.Emit(PintaCode.JumpZero, falseLabel);

                Emit(statement.Consequent);

                _code.MarkLabel(falseLabel);
            }
            else
            {
                var falseLabel = _code.DefineLabel();
                var exitLabel = _code.DefineLabel();

                Emit(statement.Test);
                _code.Emit(PintaCode.JumpZero, falseLabel);

                Emit(statement.Consequent);
                _code.Emit(PintaCode.Jump, exitLabel);

                _code.MarkLabel(falseLabel);
                Emit(statement.Alternate);

                _code.MarkLabel(exitLabel);
            }
        }

        public virtual void EmitLabelledStatement(LabelledStatement statement)
        {
            if (statement.Body != null)
            {
                statement.Body.LabelSet = statement.LabelSet + ":" + statement.Label.Name;
                Emit(statement.Body);
            }
        }

        public virtual void EmitReturnStatement(ReturnStatement statement)
        {
            if (statement.Argument == null)
                _code.Emit(PintaCode.LoadNull);
            else
                Emit(statement.Argument);

            _code.Emit(PintaCode.Return);
        }

        public virtual void EmitSwitchStatement(SwitchStatement statement)
        {
            /*
             * switch(disc)
             * {
             *   case test1:
             *   body1;
             *   case test2:
             *   body2;
             *   default:
             *   bodyDefault;
             *   case test3:
             *   body3;
             * }
             * 
             * temp = disc;
             * 
             * if (test1)
             *   goto case1;
             * if (test2)
             *   goto case2;
             * if (test3)
             *   goto case3;
             * goto default || break;
             * 
             * case1:
             * body1
             * 
             * case2:
             * body2
             * 
             * default:
             * bodyDefault
             * 
             * case3:
             * body3
             * 
             * break;
             * 
             */

            var breakLabel = _code.DefineLabel();
            using (_labelScope.Push(null, breakLabel, null))
            {
                var body = new List<Tuple<SwitchCase, PintaCodeLabel>>();
                var defaultLabel = breakLabel;

                using (var local = _scope.DeclareTemporaryLocal())
                {
                    Emit(statement.Discriminant);
                    _code.Emit(PintaCode.StoreLocal, local.Local);

                    foreach (var item in statement.Cases)
                    {
                        var currentLabel = _code.DefineLabel();
                        var currentCase = Tuple.Create(item, currentLabel);

                        body.Add(currentCase);
                        if (item.Test == null)
                        {
                            defaultLabel = currentLabel;
                        }
                        else
                        {
                            _code.Emit(PintaCode.LoadLocal, local.Local);
                            Emit(item.Test);
                            _code.Emit(PintaCode.CompareEqual);
                            _code.Emit(PintaCode.JumpNotZero, currentLabel);
                        }
                    }
                }

                _code.Emit(PintaCode.Jump, defaultLabel);

                foreach (var item in body)
                {
                    var switchCase = item.Item1;
                    var label = item.Item2;

                    _code.MarkLabel(label);
                    Emit(switchCase.Consequent);
                }

                _code.MarkLabel(breakLabel);

            }
        }

        public virtual void EmitThrowStatement(ThrowStatement statement)
        {
            throw new PintaCompilerException(statement, "Statement 'throw ...' is not supported");
        }

        public virtual void EmitTryStatement(TryStatement statement)
        {
            throw new PintaCompilerException(statement, "Statement 'try/catch' is not supported");
        }

        public virtual void EmitVariableDeclaration(VariableDeclaration variable)
        {
            if (variable.Kind != "var")
                throw new PintaCompilerException(variable, string.Format("'{0}' variables are not supported. Only 'var' variables are supported.", variable.Kind));

            foreach (var item in variable.Declarations)
            {
                if (item.Init != null)
                {
                    var local = _scope.GetLocal(item.Id.Name);

                    Emit(item.Init);
                    _code.Emit(PintaCode.StoreLocal, local.Local);
                }
            }
        }

        public virtual void EmitWhileStatement(WhileStatement statement)
        {
            /*
             * while(test) body;
             * 
             * continue:
             * if (!test)
             *   goto break
             * body
             * goto continue;
             * break:
             */

            var breakLabel = _code.DefineLabel();
            var continueLabel = _code.DefineLabel();

            using (_labelScope.Push(statement.LabelSet, breakLabel, continueLabel))
            {
                _code.MarkLabel(continueLabel);

                Emit(statement.Test);
                _code.Emit(PintaCode.JumpZero, breakLabel);

                Emit(statement.Body);
                _code.Emit(PintaCode.Jump, continueLabel);

                _code.MarkLabel(breakLabel);
            }

        }

        public virtual void EmitWithStatement(WithStatement statement)
        {
            throw new PintaCompilerException(statement, "Statement 'with...' is not supported");
        }

        public virtual void EmitProgram(Program program)
        {
            Emit(program.Body);
        }
    }
}
