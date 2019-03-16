using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Marius.Script.Tree;
using System.Runtime.CompilerServices;
using Marius.Script.Pinta.Reflection;
using System.Globalization;
using System.Text.RegularExpressions;

namespace Marius.Script.Pinta
{
    public class PintaGenerateCode : ScriptVisitor
    {
        private static readonly Regex _hexRegex = new Regex("([0-9a-fA-F][0-9a-fA-F])+", RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.ExplicitCapture);

        private PintaProgramBuilder _builder;
        private PintaFunctionBuilder _function;

        private Dictionary<string, uint> _internalFunctions;

        private Stack<PintaLoopLabel> _labels = new Stack<PintaLoopLabel>();

        private PintaTemporaryFunctionVariable _switchValue;

        public PintaProgramBuilder Program { get { return _builder; } }
        public PintaFunctionBuilder Function { get { return _function; } }

        public PintaGenerateCode(PintaProgramBuilder builder, Dictionary<string, uint> internalFunctions)
        {
            _builder = builder;
            _function = _builder.StartFunction;
            _internalFunctions = internalFunctions;
        }

        public override void Visit(ScriptArrayAccessExpression expression)
        {
            if (expression.Array == null)
                throw new ScriptSyntaxException("Invalid array");

            if (expression.Indexes == null || expression.Indexes.Count == 0)
                throw new ScriptSyntaxException("Invalid array index");

            for (var i = 0; i < expression.Indexes.Count; i++)
            {
                var current = expression.Indexes[i];

                if (i != 0)
                    _function.Emit(PintaCode.Pop);
                Accept(current);
            }

            Accept(expression.Array);
            _function.Emit(PintaCode.LoadItem);
        }

        public override void Visit(ScriptArrayExpression expression)
        {
            var elements = new List<Tuple<int, ScriptExpression>>();
            var index = 0;

            for (var i = 0; i < expression.Elements.Count; i++)
            {
                var el = expression.Elements[i];
                if (el is ScriptArrayGapElement)
                {
                    index += ((ScriptArrayGapElement)el).Count;
                }
                else if (el is ScriptArrayExpressionElement)
                {
                    elements.Add(Tuple.Create(index++, ((ScriptArrayExpressionElement)el).Element));
                }
            }

            _function.Emit(PintaCode.LoadInteger, index);
            _function.Emit(PintaCode.NewArray);

            if (elements.Count > 0)
            {
                using (var temp = _function.GetTemporary())
                {
                    _function.Emit(PintaCode.StoreLocal, temp.Variable);

                    foreach (var item in elements)
                    {
                        Accept(item.Item2);
                        _function.Emit(PintaCode.LoadInteger, item.Item1);
                        _function.Emit(PintaCode.LoadLocal, temp.Variable);
                        _function.Emit(PintaCode.StoreItem);
                    }

                    _function.Emit(PintaCode.LoadLocal, temp.Variable);

                    _function.Emit(PintaCode.LoadNull);
                    _function.Emit(PintaCode.StoreLocal, temp.Variable);
                }
            }
        }

        public override void Visit(ScriptAssignmentExpression expression)
        {
            using (var assignment = Assignment(expression.Left))
            {
                if (expression.Operator != ScriptAssignmentOperator.Assign)
                    assignment.Load();

                Accept(expression.Right);

                if (expression.Operator != ScriptAssignmentOperator.Assign)
                {
                    switch (expression.Operator)
                    {
                        case ScriptAssignmentOperator.MultiplyAssign:
                            _function.Emit(PintaCode.Multiply);
                            break;
                        case ScriptAssignmentOperator.DivideAssign:
                            _function.Emit(PintaCode.Divide);
                            break;
                        case ScriptAssignmentOperator.ModulusAssign:
                            _function.Emit(PintaCode.Remainder);
                            break;
                        case ScriptAssignmentOperator.AddAssign:
                            _function.Emit(PintaCode.Add);
                            break;
                        case ScriptAssignmentOperator.SubtractAssign:
                            _function.Emit(PintaCode.Subtract);
                            break;
                        case ScriptAssignmentOperator.ShiftLeftAssign:
                            throw new ScriptSyntaxException("'<<=' is not supported");
                        case ScriptAssignmentOperator.ShiftRightAssign:
                            throw new ScriptSyntaxException("'>>=' is not supported");
                        case ScriptAssignmentOperator.ShiftArithmeticRightAssign:
                            throw new ScriptSyntaxException("'>>=' is not supported");
                        case ScriptAssignmentOperator.BitwiseAndAssign:
                            _function.Emit(PintaCode.BitwiseAnd);
                            break;
                        case ScriptAssignmentOperator.BitwiseOrAssign:
                            _function.Emit(PintaCode.BitwiseOr);
                            break;
                        case ScriptAssignmentOperator.ExclusiveOrAssign:
                            _function.Emit(PintaCode.ExclusiveOr);
                            break;
                        default:
                            throw new NotImplementedException();
                    }
                }
                _function.Emit(PintaCode.Duplicate);
                assignment.Assign();
                assignment.Cleanup();
            }
        }

        public override void Visit(ScriptBinaryExpression expression)
        {
            if (expression.Operator == ScriptBinaryOperator.LogicalAnd)
            {
                var exitLabel = _function.DefineLabel();

                Accept(expression.Left);

                _function.Emit(PintaCode.Duplicate);
                _function.Emit(PintaCode.JumpZero, exitLabel);
                _function.Emit(PintaCode.Pop);

                Accept(expression.Right);

                _function.Emit(exitLabel);
            }
            else if (expression.Operator == ScriptBinaryOperator.LogicalOr)
            {
                var exitLabel = _function.DefineLabel();

                Accept(expression.Left);

                _function.Emit(PintaCode.Duplicate);
                _function.Emit(PintaCode.JumpNotZero, exitLabel);
                _function.Emit(PintaCode.Pop);

                Accept(expression.Right);

                _function.Emit(exitLabel);
            }
            else
            {
                Accept(expression.Left);
                Accept(expression.Right);

                switch (expression.Operator)
                {
                    case ScriptBinaryOperator.Multiply:
                        _function.Emit(PintaCode.Multiply);
                        break;
                    case ScriptBinaryOperator.Divide:
                        _function.Emit(PintaCode.Divide);
                        break;
                    case ScriptBinaryOperator.Modulus:
                        _function.Emit(PintaCode.Remainder);
                        break;
                    case ScriptBinaryOperator.Add:
                        _function.Emit(PintaCode.Add);
                        break;
                    case ScriptBinaryOperator.Subtract:
                        _function.Emit(PintaCode.Subtract);
                        break;
                    case ScriptBinaryOperator.ShiftLeft:
                        throw new ScriptSyntaxException("'<<' is not supported");
                    case ScriptBinaryOperator.ShiftRight:
                        throw new ScriptSyntaxException("'>>' is not supported");
                    case ScriptBinaryOperator.ShiftArithmeticRight:
                        throw new ScriptSyntaxException("'>>>' right is not supported");
                    case ScriptBinaryOperator.More:
                        _function.Emit(PintaCode.CompareMoreThan);
                        break;
                    case ScriptBinaryOperator.Less:
                        _function.Emit(PintaCode.CompareLessThan);
                        break;
                    case ScriptBinaryOperator.MoreOrEqual:
                        _function.Emit(PintaCode.CompareLessThan);
                        _function.Emit(PintaCode.Not);
                        break;
                    case ScriptBinaryOperator.LessOrEqual:
                        _function.Emit(PintaCode.CompareMoreThan);
                        _function.Emit(PintaCode.Not);
                        break;
                    case ScriptBinaryOperator.InstanceOf:
                        throw new ScriptSyntaxException("'instanceof' is not supported");
                    case ScriptBinaryOperator.In:
                        throw new ScriptSyntaxException("'in' is not supported");
                    case ScriptBinaryOperator.Equals:
                        _function.Emit(PintaCode.CompareEqual);
                        break;
                    case ScriptBinaryOperator.NotEquals:
                        _function.Emit(PintaCode.CompareEqual);
                        _function.Emit(PintaCode.Not);
                        break;
                    case ScriptBinaryOperator.StrictEquals:
                        throw new ScriptSyntaxException("'===' is not supported");
                    case ScriptBinaryOperator.StrictNotEquals:
                        throw new ScriptSyntaxException("'!==' is not supported");
                    case ScriptBinaryOperator.BitwiseAnd:
                        _function.Emit(PintaCode.BitwiseAnd);
                        break;
                    case ScriptBinaryOperator.BitwiseOr:
                        _function.Emit(PintaCode.BitwiseOr);
                        break;
                    case ScriptBinaryOperator.ExclusiveOr:
                        _function.Emit(PintaCode.BitwiseExclusiveOr);
                        break;
                    default:
                        throw new NotImplementedException();
                }
            }
        }

        public override void Visit(ScriptUnaryExpression expression)
        {
            if (expression.Operator == ScriptUnaryOperator.PostDecrement || expression.Operator == ScriptUnaryOperator.PostIncrement)
            {
                using (var assignment = Assignment(expression.Operand))
                {
                    assignment.Prepare(true);
                    assignment.Load();

                    _function.Emit(PintaCode.Duplicate);
                    _function.Emit(PintaCode.LoadIntegerOne);

                    if (expression.Operator == ScriptUnaryOperator.PostDecrement)
                        _function.Emit(PintaCode.Subtract);
                    else
                        _function.Emit(PintaCode.Add);

                    assignment.Assign();
                    assignment.Cleanup();
                }

            }
            else if (expression.Operator == ScriptUnaryOperator.PreDecrement || expression.Operator == ScriptUnaryOperator.PreIncrement)
            {
                using (var assignment = Assignment(expression.Operand))
                {
                    assignment.Prepare(true);
                    assignment.Load();

                    _function.Emit(PintaCode.LoadIntegerOne);

                    if (expression.Operator == ScriptUnaryOperator.PreDecrement)
                        _function.Emit(PintaCode.Subtract);
                    else
                        _function.Emit(PintaCode.Add);

                    _function.Emit(PintaCode.Duplicate);

                    assignment.Assign();
                    assignment.Cleanup();
                }
            }
            else
            {
                Accept(expression.Operand);
                switch (expression.Operator)
                {
                    case ScriptUnaryOperator.Delete:
                        throw new ScriptSyntaxException("'delete' operator is not supported");
                    case ScriptUnaryOperator.Void:
                        break;
                    case ScriptUnaryOperator.Typeof:
                        throw new ScriptSyntaxException("'typeof' operator is not supported");
                    case ScriptUnaryOperator.Plus:
                        break;
                    case ScriptUnaryOperator.Minus:
                        _function.Emit(PintaCode.Negate);
                        break;
                    case ScriptUnaryOperator.BitwiseNot:
                        _function.Emit(PintaCode.BitwiseNot);
                        break;
                    case ScriptUnaryOperator.LogicalNot:
                        _function.Emit(PintaCode.Not);
                        break;
                    default:
                        throw new NotImplementedException();
                }
            }
        }

        public override void Visit(ScriptConditionalExpression expression)
        {
            if (expression.Condition == null)
                throw new ScriptSyntaxException("Invalid conditional statement");

            var exitLabel = _function.DefineLabel();
            var falseLabel = _function.DefineLabel();

            Accept(expression.Condition);

            _function.Emit(PintaCode.JumpZero, falseLabel);

            Accept(expression.TrueBody);

            if (expression.FalseBody != null)
                _function.Emit(PintaCode.Jump, exitLabel);

            _function.Emit(falseLabel);

            if (expression.FalseBody != null)
                Accept(expression.FalseBody);
            else
                _function.Emit(PintaCode.LoadNull);

            _function.Emit(exitLabel);
        }

        public override void Visit(ScriptFunctionExpression expression)
        {
            throw new ScriptSyntaxException("Function expressions are not supported");
        }

        public override void Visit(ScriptInvokeExpression expression)
        {
            var member = expression.Function as ScriptMemberAccessExpression;
            if (member != null)
            {
                var args = new List<ScriptExpression>(1 + expression.Arguments.Count);
                args.Add(member.Object);
                args.AddRange(expression.Arguments);
                EmitInvoke(member.Member.Name, args);

                return;
            }

            var functionName = expression.Function as ScriptNameExpression;
            if (functionName == null)
                throw new ScriptSyntaxException("Dynamic function invocation is not supported");
            EmitInvoke(functionName.Name.Name, expression.Arguments);
        }

        public override void Visit(ScriptListExpression expression)
        {
            for (var i = 0; i < expression.Expressions.Count; i++)
            {
                if (i != 0)
                    _function.Emit(PintaCode.Pop);

                var current = expression.Expressions[i];
                Accept(current);
            }
        }

        public override void Visit(ScriptLiteralExpression expression)
        {
            switch (expression.Kind)
            {
                case ScriptLiteralKind.This:
                    throw new ScriptSyntaxException("'this' is not supported");
                case ScriptLiteralKind.String:
                    _function.Emit(PintaCode.LoadString, (string)expression.Value);
                    break;
                case ScriptLiteralKind.Numeric:
                    var value = (string)expression.Value;
                    if (value.StartsWith("0x") || value.StartsWith("0X"))
                    {
                        var number = Convert.ToInt32(value, 16);
                        if (number == 0)
                            _function.Emit(PintaCode.LoadIntegerZero);
                        else if (number == 1)
                            _function.Emit(PintaCode.LoadIntegerOne);
                        else
                            _function.Emit(PintaCode.LoadInteger, number);
                    }
                    else
                    {
                        var intNumber = 0;
                        if (int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out intNumber))
                        {
                            if (intNumber == 0)
                                _function.Emit(PintaCode.LoadIntegerZero);
                            else if (intNumber == 1)
                                _function.Emit(PintaCode.LoadIntegerOne);
                            else
                                _function.Emit(PintaCode.LoadInteger, intNumber);
                        }
                        else
                        {
                            var number = 0m;
                            if (decimal.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out number))
                            {
                                if (number == 0m)
                                {
                                    _function.Emit(PintaCode.LoadDecimalZero);
                                }
                                else if (number == 1m)
                                {
                                    _function.Emit(PintaCode.LoadDecimalZero);
                                }
                                else
                                {
                                    _function.Emit(PintaCode.LoadString, value);
                                    _function.Emit(PintaCode.ConvertDecimal);
                                }
                            }
                            else
                            {
                                _function.Emit(PintaCode.LoadString, value);
                                _function.Emit(PintaCode.ConvertDecimal);
                            }
                        }
                    }
                    break;
                case ScriptLiteralKind.Regex:
                    throw new ScriptSyntaxException("regular expressions are not supported");
                case ScriptLiteralKind.True:
                    _function.Emit(PintaCode.LoadIntegerOne);
                    break;
                case ScriptLiteralKind.False:
                    _function.Emit(PintaCode.LoadIntegerZero);
                    break;
                case ScriptLiteralKind.Null:
                    _function.Emit(PintaCode.LoadNull);
                    break;
                default:
                    throw new NotImplementedException();
            }
        }

        public override void Visit(ScriptMemberAccessExpression expression)
        {
            var member = expression.Member.Name;

            switch (member)
            {
                case "length":
                    Accept(expression.Object);
                    _function.Emit(PintaCode.GetLength);
                    break;
                default:
                    throw new ScriptSyntaxException(string.Format("undefined memeber '{0}'", member));
            }
        }

        public override void Visit(ScriptNameExpression expression)
        {
            var name = new PintaNameReference(expression);
            name.Load(this);
        }

        public override void Visit(ScriptNewExpression expression)
        {
            var typeName = expression.TypeExpression as ScriptNameExpression;
            if (typeName == null)
                throw new ScriptSyntaxException("unsupported type");

            var name = typeName.Name.Name;
            switch (name)
            {
                case "array":
                case "Array":
                    if (expression.Arguments.Count > 1)
                        throw new ScriptSyntaxException("invalid arguments for new array");

                    if (expression.Arguments.Count == 0)
                        _function.Emit(PintaCode.LoadIntegerZero);
                    else
                        Accept(expression.Arguments[0]);

                    _function.Emit(PintaCode.NewArray);
                    break;
                case "buffer":
                case "Buffer":
                    break;
                default:
                    throw new ScriptSyntaxException("unsupported type");
            }
        }

        public override void Visit(ScriptObjectExpression expression)
        {
            throw new ScriptSyntaxException("Object literals are not supported");
        }

        public override void Visit(ScriptLocalVariable variable)
        {
            var local = _function.GetVariable(variable.Name.Name);
            if (variable.Initializer != null)
            {
                Accept(variable.Initializer);
                _function.Emit(PintaCode.StoreLocal, local);
            }
        }

        public override void Visit(ScriptProgram program)
        {
            base.Visit(program);

            _function.Emit(PintaCode.Exit);
        }

        public override void Visit(ScriptFunctionDeclaration functionDeclaration)
        {
            var function = functionDeclaration.Function;
            var name = function.Name;

            _function = _builder.GetFunction(name.Name);
            if (_function == null)
                throw new ScriptSyntaxException();

            Accept(function.Body);
            _function.Emit(PintaCode.LoadNull);
            _function.Emit(PintaCode.Return);

            _function = _builder.StartFunction;
        }

        public override void Visit(ScriptExpressionStatement statement)
        {
            Accept(statement.Expression);
            _function.Emit(PintaCode.Pop);
        }

        public override void Visit(ScriptWhileStatement statement)
        {
            if (statement.Condition == null)
                throw new ScriptSyntaxException("Invalid while statement");

            if (statement.Body == null)
                throw new ScriptSyntaxException("Invalid while statement");

            var continueLabel = _function.DefineLabel();
            var breakLabel = _function.DefineLabel();

            _labels.Push(new PintaLoopLabel(breakLabel, continueLabel));

            _function.Emit(continueLabel);
            Accept(statement.Condition);
            _function.Emit(PintaCode.JumpZero, breakLabel);

            Accept(statement.Body);
            _function.Emit(PintaCode.Jump, continueLabel);
            _function.Emit(breakLabel);

            _labels.Pop();
        }

        public override void Visit(ScriptForStatement statement)
        {
            var startLabel = _function.DefineLabel();
            var conditionLabel = _function.DefineLabel();
            var continueLabel = _function.DefineLabel();
            var breakLabel = _function.DefineLabel();

            _labels.Push(new PintaLoopLabel(breakLabel, continueLabel));

            if (statement.Init != null)
                Accept(statement.Init);

            _function.Emit(PintaCode.Jump, conditionLabel);
            _function.Emit(startLabel);

            Accept(statement.Body);

            _function.Emit(continueLabel);

            Accept(statement.Step);

            _function.Emit(PintaCode.Pop);
            _function.Emit(conditionLabel);

            Accept(statement.Condition);

            _function.Emit(PintaCode.JumpNotZero, startLabel);
            _function.Emit(breakLabel);

            _labels.Pop();
        }

        public override void Visit(ScriptForExpressionTarget target)
        {
            if (target.Expression != null)
            {
                Accept(target.Expression);
                _function.Emit(PintaCode.Pop);
            }
        }

        public override void Visit(ScriptDoStatement statement)
        {
            if (statement.Condition == null)
                throw new ScriptSyntaxException("Invalid do-while statement");

            if (statement.Body == null)
                throw new ScriptSyntaxException("Invalid do-while statement");

            var startLabel = _function.DefineLabel();
            var continueLabel = _function.DefineLabel();
            var breakLabel = _function.DefineLabel();

            _labels.Push(new PintaLoopLabel(breakLabel, continueLabel));

            _function.Emit(PintaCode.Jump, startLabel);
            _function.Emit(continueLabel);

            Accept(statement.Condition);

            _function.Emit(PintaCode.JumpZero, breakLabel);
            _function.Emit(startLabel);

            Accept(statement.Body);

            _function.Emit(PintaCode.Jump, continueLabel);
            _function.Emit(breakLabel);

            _labels.Pop();
        }

        public override void Visit(ScriptBreakStatement statement)
        {
            if (_labels.Count == 0)
                throw new ScriptSyntaxException("Break can only be used within switch or loop statement");

            var current = _labels.Peek();
            _function.Emit(PintaCode.Jump, current.BreakLabel);
        }

        public override void Visit(ScriptContinueStatement statement)
        {
            if (_labels.Count == 0)
                throw new ScriptSyntaxException("Continue can only be used within loop statement");

            var current = _labels.Peek();
            _function.Emit(PintaCode.Jump, current.ContinueLabel);
        }

        public override void Visit(ScriptReturnStatement statement)
        {
            if (statement.Value == null)
                _function.Emit(PintaCode.LoadNull);
            else
                Accept(statement.Value);

            if (Program.StartFunction == Function)
                _function.Emit(PintaCode.Exit);
            else
                _function.Emit(PintaCode.Return);
        }

        public override void Visit(ScriptIfStatement statement)
        {
            var exitLabel = _function.DefineLabel();
            var falseLabel = _function.DefineLabel();

            Accept(statement.Condition);

            _function.Emit(PintaCode.JumpZero, falseLabel);

            Accept(statement.TrueBody);

            if (statement.FalseBody != null)
                _function.Emit(PintaCode.Jump, exitLabel);

            _function.Emit(falseLabel);

            if (statement.FalseBody != null)
            {
                Accept(statement.FalseBody);
                _function.Emit(exitLabel);
            }
        }

        public override void Visit(ScriptSwitchStatement statement)
        {
            var breakLabel = _function.DefineLabel();

            _labels.Push(new PintaLoopLabel(breakLabel, null));

            using (var temp = _function.GetTemporary())
            {
                var currentSwitch = _switchValue;
                _switchValue = temp;

                Accept(statement.Condition);
                _function.Emit(PintaCode.StoreLocal, _switchValue.Variable);

                Accept(statement.Cases);

                _switchValue = currentSwitch;

                _function.Emit(PintaCode.LoadNull);
                _function.Emit(PintaCode.StoreLocal, temp.Variable);
            }

            _labels.Pop();
        }

        public override void Visit(ScriptSwitchCase switchCase)
        {
            // TODO: fix fall-through if default is not the last case
            var label = _function.DefineLabel();

            if (switchCase.CaseExpression != null)
            {
                _function.Emit(PintaCode.LoadLocal, _switchValue.Variable);

                Accept(switchCase.CaseExpression);

                _function.Emit(PintaCode.CompareEqual);
                _function.Emit(PintaCode.JumpNotZero, label);
            }

            Accept(switchCase.Statements);

            _function.Emit(label);
        }

        public override void Visit(ScriptDebuggerStatement statement)
        {
        }

        public override void Visit(ScriptWithStatement statement)
        {
            throw new ScriptSyntaxException("With statement is not supported");
        }

        public override void Visit(ScriptThrowStatement statement)
        {
            throw new ScriptSyntaxException("Throw statement is not supported");
        }

        public override void Visit(ScriptTryStatement statement)
        {
            throw new ScriptSyntaxException("Try-catch-finally is not supported");
        }

        public override void Visit(ScriptForinStatement statement)
        {
            throw new ScriptSyntaxException("for-in statement is not supported");
        }

        private void EmitInvoke(string name, List<ScriptExpression> arguments)
        {
            var function = Program.GetFunction(name);
            if (function != null)
            {
                Accept(arguments);
                _function.EmitCall(function, (uint)arguments.Count);
            }
            else
            {
                var index = 0U;
                if (_internalFunctions.TryGetValue(name, out index))
                {
                    Accept(arguments);
                    _function.EmitCallInternal(index, (uint)arguments.Count);
                }
                else
                {
                    EmitIntrinsic(name, arguments);
                }
            }
        }

        private void EmitIntrinsic(string name, List<ScriptExpression> arguments)
        {
            switch (name)
            {
                case "int":
                case "integer":
                    if (arguments.Count != 1)
                        throw new ScriptSyntaxException("invalid number of arguments for int/integer function");
                    Accept(arguments[0]);
                    _function.Emit(PintaCode.ConvertInteger);
                    break;
                case "decimal":
                    if (arguments.Count != 1)
                        throw new ScriptSyntaxException("invalid number of arguments for decimal function");
                    Accept(arguments[0]);
                    _function.Emit(PintaCode.ConvertDecimal);
                    break;
                case "string":
                    if (arguments.Count != 1)
                        throw new ScriptSyntaxException("invalid number of arguments for string function");
                    Accept(arguments[0]);
                    _function.Emit(PintaCode.ConvertString);
                    break;
                case "concat":
                    if (arguments.Count == 1)
                    {
                        Accept(arguments[0]);
                        _function.Emit(PintaCode.ConvertString);
                    }
                    else if (arguments.Count > 0)
                    {
                        Accept(arguments[0]);

                        for (var i = 1; i < arguments.Count; i++)
                        {
                            var current = arguments[i];
                            Accept(current);
                            _function.Emit(PintaCode.Concat);
                        }
                    }
                    break;
                case "substr":
                case "substring":
                    if (arguments.Count != 3)
                        throw new ScriptSyntaxException("invalid number of arguments for substr/substring function");
                    Accept(arguments[1]); // this is correct order
                    Accept(arguments[2]);
                    Accept(arguments[0]);
                    _function.Emit(PintaCode.Substring);
                    break;
                case "hex":
                    if (arguments.Count != 1)
                        throw new ScriptSyntaxException("invalid number of arguments for hex function");
                    EmitBlob(arguments[0], PintaProgramBlobType.Hex);
                    break;
                case "base64":
                    if (arguments.Count != 1)
                        throw new ScriptSyntaxException("invalid number of arguments for hex function");
                    EmitBlob(arguments[0], PintaProgramBlobType.Base64);
                    break;
                case "ascii":
                    if (arguments.Count != 1)
                        throw new ScriptSyntaxException("invalid number of arguments for hex function");
                    EmitBlob(arguments[0], PintaProgramBlobType.Ascii);
                    break;
                case "utf8":
                    if (arguments.Count != 1)
                        throw new ScriptSyntaxException("invalid number of arguments for hex function");
                    EmitBlob(arguments[0], PintaProgramBlobType.Utf8);
                    break;
                default:
                    throw new ScriptSyntaxException(string.Format("undefined function '{0}'", name));
            }
        }

        private void EmitBlob(ScriptExpression data, PintaProgramBlobType type)
        {
            var literal = data as ScriptLiteralExpression;
            if (literal == null || literal.Kind != ScriptLiteralKind.String)
                throw new ScriptSyntaxException("hex function only accepts literal strings");

            var stringValue = (string)literal.Value;

            switch (type)
            {
                case PintaProgramBlobType.Hex:
                    if (!_hexRegex.IsMatch(stringValue))
                        throw new ScriptSyntaxException("hex string contains invalid characters, valid 0-9, a-f, A-F in pairs");
                    break;
                case PintaProgramBlobType.Base64:
                    try
                    {
                        Convert.FromBase64String(stringValue);
                    }
                    catch (Exception)
                    {
                        throw new ScriptSyntaxException("base64 string is invalid");
                    }
                    break;
            }

            _function.Emit(PintaCode.LoadBlob, type, stringValue);
        }

        private PintaAssignment Assignment(ScriptExpression target)
        {
            var assignment = new PintaAssignment(this, target);
            assignment.Prepare();
            return assignment;
        }
    }
}
