using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public partial class PintaFunctionCompiler
    {
        private static readonly Regex _hexRegex = new Regex("([0-9a-fA-F][0-9a-fA-F])+", RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.ExplicitCapture);

        public virtual void Emit(Expression expression, bool discard = false)
        {
            switch (expression.Type)
            {
                case SyntaxNodes.AssignmentExpression:
                    EmitAssignmentExpression(expression.As<AssignmentExpression>(), discard);
                    break;

                case SyntaxNodes.ArrayExpression:
                    EmitArrayExpression(expression.As<ArrayExpression>(), discard);
                    break;

                case SyntaxNodes.BinaryExpression:
                    EmitBinaryExpression(expression.As<BinaryExpression>(), discard);
                    break;

                case SyntaxNodes.CallExpression:
                    EmitCallExpression(expression.As<CallExpression>(), discard);
                    break;

                case SyntaxNodes.ConditionalExpression:
                    EmitConditionalExpression(expression.As<ConditionalExpression>(), discard);
                    break;

                case SyntaxNodes.FunctionExpression:
                    EmitFunctionExpression(expression.As<FunctionExpression>(), discard);
                    break;

                case SyntaxNodes.Identifier:
                    EmitIdentifier(expression.As<Identifier>(), discard);
                    break;

                case SyntaxNodes.Literal:
                    EmitLiteral(expression.As<Literal>(), discard);
                    break;

                case SyntaxNodes.RegularExpressionLiteral:
                    EmitLiteral(expression.As<Literal>(), discard);
                    break;

                case SyntaxNodes.LogicalExpression:
                    EmitLogicalExpression(expression.As<LogicalExpression>(), discard);
                    break;

                case SyntaxNodes.MemberExpression:
                    EmitMemberExpression(expression.As<MemberExpression>(), false, discard);
                    break;

                case SyntaxNodes.NewExpression:
                    EmitNewExpression(expression.As<NewExpression>(), discard);
                    break;

                case SyntaxNodes.ObjectExpression:
                    EmitObjectExpression(expression.As<ObjectExpression>(), discard);
                    break;

                case SyntaxNodes.SequenceExpression:
                    EmitSequenceExpression(expression.As<SequenceExpression>(), discard);
                    break;

                case SyntaxNodes.ThisExpression:
                    EmitThisExpression(expression.As<ThisExpression>(), discard);
                    break;

                case SyntaxNodes.UpdateExpression:
                    EmitUpdateExpression(expression.As<UpdateExpression>(), discard);
                    break;

                case SyntaxNodes.UnaryExpression:
                    EmitUnaryExpression(expression.As<UnaryExpression>(), discard);
                    break;

                default:
                    throw new PintaCompilerException(expression, string.Format("Unsupported expression type {0}", expression.Type));
            }
        }

        public virtual void Emit(IEnumerable<Expression> expressions)
        {
            foreach (var item in expressions)
            {
                Emit(item);
            }
        }

        public virtual void EmitAssignmentExpression(AssignmentExpression expression, bool discard = false)
        {
            var left = expression.Left;
            var right = expression.Right;

            if (left.Type == SyntaxNodes.MemberExpression)
                EmitAssignmentMember(expression, left.As<MemberExpression>(), right, expression.Operator, discard);
            else if (left.Type == SyntaxNodes.Identifier)
                EmitAssignmentIdentifier(expression, left.As<Identifier>(), right, expression.Operator, discard);
            else
                throw new PintaCompilerException(expression, string.Format("Cannot assign value to expression of type {0}", left.Type));
        }

        public virtual void EmitArrayExpression(ArrayExpression expression, bool discard = false)
        {
            var elements = expression.Elements.ToList();

            _code.Emit(PintaCode.LoadInteger, elements.Count);
            _code.Emit(PintaCode.NewArray);
            if (elements.Count == 0)
                return;

            using (var array = _scope.DeclareTemporaryLocal())
            {
                _code.Emit(PintaCode.StoreLocal, array.Local);

                for (var i = 0; i < elements.Count; i++)
                {
                    var current = elements[i];
                    if (current == null)
                        continue;

                    Emit(current);
                    _code.Emit(PintaCode.LoadInteger, i);
                    _code.Emit(PintaCode.LoadLocal, array.Local);
                    _code.Emit(PintaCode.StoreItem);
                }

                _code.Emit(PintaCode.LoadLocal, array.Local);

                _code.Emit(PintaCode.LoadNull);
                _code.Emit(PintaCode.StoreLocal, array.Local);
            }

            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitBinaryExpression(BinaryExpression expression, bool discard = false)
        {
            Emit(expression.Left);
            Emit(expression.Right);

            switch (expression.Operator)
            {
                case BinaryOperator.Plus:
                    _code.Emit(PintaCode.Add);
                    break;
                case BinaryOperator.Minus:
                    _code.Emit(PintaCode.Subtract);
                    break;
                case BinaryOperator.Times:
                    _code.Emit(PintaCode.Multiply);
                    break;
                case BinaryOperator.Divide:
                    _code.Emit(PintaCode.Divide);
                    break;
                case BinaryOperator.Modulo:
                    _code.Emit(PintaCode.Remainder);
                    break;
                case BinaryOperator.Equal:
                    _code.Emit(PintaCode.CompareEqual);
                    break;
                case BinaryOperator.NotEqual:
                    _code.Emit(PintaCode.CompareEqual);
                    _code.Emit(PintaCode.Not);
                    break;
                case BinaryOperator.Greater:
                    _code.Emit(PintaCode.CompareMoreThan);
                    break;
                case BinaryOperator.GreaterOrEqual:
                    _code.Emit(PintaCode.CompareLessThan);
                    _code.Emit(PintaCode.Not);
                    break;
                case BinaryOperator.Less:
                    _code.Emit(PintaCode.CompareLessThan);
                    break;
                case BinaryOperator.LessOrEqual:
                    _code.Emit(PintaCode.CompareMoreThan);
                    _code.Emit(PintaCode.Not);
                    break;
                case BinaryOperator.StrictlyEqual:
                    throw new PintaCompilerException(expression, "Operator '===' is not supported");
                case BinaryOperator.StricltyNotEqual:
                    throw new PintaCompilerException(expression, "Operator '!==' is not supported");
                case BinaryOperator.BitwiseAnd:
                    _code.Emit(PintaCode.BitwiseAnd);
                    break;
                case BinaryOperator.BitwiseOr:
                    _code.Emit(PintaCode.BitwiseOr);
                    break;
                case BinaryOperator.BitwiseXOr:
                    _code.Emit(PintaCode.BitwiseExclusiveOr);
                    break;
                case BinaryOperator.LeftShift:
                    throw new PintaCompilerException(expression, "Operator '<<' is not supported");
                case BinaryOperator.RightShift:
                    throw new PintaCompilerException(expression, "Operator '>>' is not supported");
                case BinaryOperator.UnsignedRightShift:
                    throw new PintaCompilerException(expression, "Operator '>>>' is not supported");
                case BinaryOperator.InstanceOf:
                    throw new PintaCompilerException(expression, "Operator 'instanceof' is not supported");
                case BinaryOperator.In:
                    throw new PintaCompilerException(expression, "Operator 'in' is not supported");
                default:
                    throw new PintaCompilerException(expression, string.Format("Unsupported binary operator {0}", expression.Operator));
            }

            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitCallExpression(CallExpression expression, bool discard = false)
        {
            if (EmitCallIntrinsic(expression, discard))
                return;

            if (EmitCallSimple(expression, discard))
                return;

            var code = PintaCode.Invoke;
            var callee = expression.Callee;
            if (callee.Type == SyntaxNodes.MemberExpression)
            {
                var member = callee.As<MemberExpression>();
                EmitMemberExpression(member, true, false);

                code = PintaCode.InvokeMember;
            }
            else
            {
                Emit(callee);
            }

            var count = 0U;
            foreach (var item in expression.Arguments)
            {
                Emit(item);
                count++;
            }

            _code.Emit(code, count);
            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitConditionalExpression(ConditionalExpression expression, bool discard = false)
        {
            var falseLabel = _code.DefineLabel();
            var exitLabel = _code.DefineLabel();

            Emit(expression.Test);
            _code.Emit(PintaCode.JumpZero, falseLabel);

            Emit(expression.Consequent);
            _code.Emit(PintaCode.Jump, exitLabel);

            _code.MarkLabel(falseLabel);
            Emit(expression.Alternate);

            _code.MarkLabel(exitLabel);

            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitFunctionExpression(FunctionExpression expression, bool discard = false)
        {
            var function = _module.GetFunction(expression);
            if (!discard)
                _code.Emit(PintaCode.NewFunction, function.Function);
        }

        public virtual void EmitIdentifier(Identifier identifier, bool discard = false)
        {
            var local = _scope.GetLocal(identifier.Name);
            if (local != null)
            {
                if (!discard)
                    _code.Emit(PintaCode.LoadLocal, local.Local);
                return;
            }

            var parameter = _scope.GetParameter(identifier.Name);
            if (parameter != null)
            {
                if (!discard)
                    _code.Emit(PintaCode.LoadArgument, parameter.Parameter);
                return;
            }

            var closure = _scope.GetClosure(identifier.Name);
            if (closure != null)
            {
                if (!discard)
                    _code.Emit(PintaCode.LoadClosure, closure.Closure);

                return;
            }

            var global = _module.Module.GetGlobal(identifier.Name);
            if (!discard)
                _code.Emit(PintaCode.LoadGlobal, global);
        }

        public virtual void EmitLiteral(Literal literal, bool discard = false)
        {
            if (discard)
                return;

            if (literal.Value == null)
            {
                _code.Emit(PintaCode.LoadNull);
            }
            else if (literal.Value is int || literal.Value is long || literal.Value is float || literal.Value is double)
            {
                if (literal.Value is int)
                {
                    _code.Emit(PintaCode.LoadInteger, (int)literal.Value);
                }
                else if (literal.Value is long)
                {
                    _code.Emit(PintaCode.LoadInteger, (int)(long)literal.Value);
                }
                else
                {
                    var intValue = 0;
                    var decimalValue = 0M;
                    if (int.TryParse(literal.Raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out intValue))
                    {
                        _code.Emit(PintaCode.LoadInteger, intValue);
                    }
                    else if (decimal.TryParse(literal.Raw, NumberStyles.Float, CultureInfo.InvariantCulture, out decimalValue))
                    {
                        if (decimalValue == 0M)
                        {
                            _code.Emit(PintaCode.LoadDecimalZero);
                        }
                        else if (decimalValue == 1M)
                        {
                            _code.Emit(PintaCode.LoadDecimalOne);
                        }
                        else if (decimal.Truncate(decimalValue) == decimalValue)
                        {
                            _code.Emit(PintaCode.LoadInteger, (int)decimalValue);
                            _code.Emit(PintaCode.ConvertDecimal);
                        }
                        else
                        {
                            _code.Emit(PintaCode.LoadString, literal.Raw);
                            _code.Emit(PintaCode.ConvertDecimal);
                        }
                    }
                    else
                    {
                        _code.Emit(PintaCode.LoadString, literal.Raw);
                        _code.Emit(PintaCode.ConvertDecimal);
                    }
                }
            }
            else if (literal.Value is string)
            {
                _code.Emit(PintaCode.LoadString, (string)literal.Value);
            }
            else if (literal.Value is bool)
            {
                if ((bool)literal.Value)
                    _code.Emit(PintaCode.LoadIntegerOne);
                else
                    _code.Emit(PintaCode.LoadIntegerZero);
            }
            else
            {
                throw new PintaCompilerException(literal, string.Format("Unsupported literal type '{0}'", literal.Value.GetType()));
            }
        }

        public virtual void EmitLogicalExpression(LogicalExpression expression, bool discard = false)
        {
            var code = PintaCode.JumpZero;
            if (expression.Operator == LogicalOperator.LogicalOr)
                code = PintaCode.JumpNotZero;

            var exitLabel = _code.DefineLabel();

            Emit(expression.Left);

            _code.Emit(PintaCode.Duplicate);
            _code.Emit(code, exitLabel);
            _code.Emit(PintaCode.Pop);

            Emit(expression.Right);

            _code.MarkLabel(exitLabel);

            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitMemberExpression(MemberExpression expression, bool duplicateObject, bool discard)
        {
            if (!expression.Computed && expression.Property.Type == SyntaxNodes.Identifier)
            {
                Emit(expression.Object);
                if (duplicateObject)
                    _code.Emit(PintaCode.Duplicate);

                var property = expression.Property.As<Identifier>();
                if (property.Name == "length")
                {
                    _code.Emit(PintaCode.GetLength);
                }
                else
                {
                    _code.Emit(PintaCode.LoadString, property.Name);
                    _code.Emit(PintaCode.LoadMember);
                }
            }
            else
            {
                var loadItem = false;
                if (expression.Computed && expression.Property.Type == SyntaxNodes.Literal)
                {
                    var literal = expression.Property.As<Literal>();
                    var index = 0;
                    if (int.TryParse(literal.Raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out index))
                    {
                        loadItem = true;

                        if (!duplicateObject)
                        {
                            Emit(expression.Property); // yep that is the correct order :'(
                            Emit(expression.Object);
                            _code.Emit(PintaCode.LoadItem);
                        }
                        else
                        {
                            using (var objectTemp = _scope.DeclareTemporaryLocal())
                            {
                                Emit(expression.Object);
                                _code.Emit(PintaCode.Duplicate);

                                _code.Emit(PintaCode.StoreLocal, objectTemp.Local);

                                Emit(expression.Property); // yep that is the correct order :'(
                                _code.Emit(PintaCode.LoadLocal, objectTemp.Local);
                                _code.Emit(PintaCode.LoadItem);
                            }
                        }
                    }
                }

                if (!loadItem)
                {
                    Emit(expression.Object);
                    if (duplicateObject)
                        _code.Emit(PintaCode.Duplicate);

                    Emit(expression.Property);
                    _code.Emit(PintaCode.LoadMember);
                }
            }

            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitNewExpression(NewExpression expression, bool discard = false)
        {
            if (expression.Callee.Type == SyntaxNodes.Identifier)
            {
                var typeName = expression.Callee.As<Identifier>();
                if (typeName.Name == "array" || typeName.Name == "Array")
                {
                    var arguments = expression.Arguments.ToList();
                    if (arguments.Count == 0)
                        _code.Emit(PintaCode.LoadIntegerZero);
                    else
                        Emit(expression.Arguments);
                    _code.Emit(PintaCode.NewArray);
                    return;
                }
            }

            Emit(expression.Callee);

            var count = 0;
            foreach (var item in expression.Arguments)
            {
                Emit(item);
                count++;
            }

            _code.Emit(PintaCode.New, (uint)count);

            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitObjectExpression(ObjectExpression expression, bool discard = false)
        {
            _code.Emit(PintaCode.NewObject);

            foreach (var item in expression.Properties)
            {
                _code.Emit(PintaCode.Duplicate);
                _code.Emit(PintaCode.LoadString, item.Key.GetKey());

                Emit(item.Value);

                _code.Emit(PintaCode.StoreMember);
            }

            if (discard)
                _code.Emit(PintaCode.Pop);
        }

        public virtual void EmitSequenceExpression(SequenceExpression expression, bool discard = false)
        {
            for (var i = 0; i < expression.Expressions.Count; i++)
            {
                var item = expression.Expressions[i];
                var discardInner = discard;
                if (!discardInner)
                    discardInner = i < expression.Expressions.Count - 1;

                Emit(item, discardInner);
            }
        }

        public virtual void EmitThisExpression(ThisExpression expression, bool discard = false)
        {
            if (!discard)
                _code.Emit(PintaCode.LoadThis);
        }

        public virtual void EmitUpdateExpression(UpdateExpression expression, bool discard = false)
        {
            var left = expression.Argument;
            if (left.Type == SyntaxNodes.MemberExpression)
                EmitUpdateMember(expression, left.As<MemberExpression>(), expression.Prefix, expression.Operator, discard);
            else if (left.Type == SyntaxNodes.Identifier)
                EmitUpdateIdentifier(expression, left.As<Identifier>(), expression.Prefix, expression.Operator, discard);
            else
                throw new PintaCompilerException(expression, string.Format("Cannot assign value to expression of type {0}", left.Type));
        }

        public virtual void EmitUnaryExpression(UnaryExpression expression, bool discard = false)
        {
            Emit(expression.Argument, discard);

            if (!discard)
            {
                switch (expression.Operator)
                {
                    case UnaryOperator.Plus:
                        break;
                    case UnaryOperator.Minus:
                        _code.Emit(PintaCode.Negate);
                        break;
                    case UnaryOperator.BitwiseNot:
                        _code.Emit(PintaCode.BitwiseNot);
                        break;
                    case UnaryOperator.LogicalNot:
                        _code.Emit(PintaCode.Not);
                        break;
                    case UnaryOperator.Void:
                        _code.Emit(PintaCode.Pop);
                        _code.Emit(PintaCode.LoadNull);
                        break;
                    case UnaryOperator.Delete:
                        throw new PintaCompilerException(expression, "Operator 'delete' is not supported");
                    case UnaryOperator.TypeOf:
                        throw new PintaCompilerException(expression, "Operator 'typeof' is not supported");
                    default:
                        throw new PintaCompilerException(expression, string.Format("Unsupported unary operator {0}", expression.Operator));
                }
            }
        }

        protected virtual bool EmitCallSimple(CallExpression expression, bool discard = false)
        {
            if (expression.Callee.Type != SyntaxNodes.Identifier)
                return false;

            var callee = expression.Callee.As<Identifier>();
            var name = callee.Name;

            var local = _scope.GetLocal(name, false);
            if (local != null)
                return false;

            var function = _scope.GetFunction(name);
            if (function == null)
                return false;

            if (function.Function == null)
                return false;

            if (!function.Function.IsSimple)
                return false;

            var count = 0U;
            foreach (var item in expression.Arguments)
            {
                Emit(item);
                count++;
            }

            _code.Emit(PintaCode.Call, function.Function.Function, count);
            if (discard)
                _code.Emit(PintaCode.Pop);
            return true;
        }

        protected virtual bool EmitCallIntrinsic(CallExpression expression, bool discard = false)
        {
            var name = default(string);
            var callee = expression.Callee;
            var arguments = new List<Expression>();

            if (callee.Type == SyntaxNodes.Identifier)
            {
                var identifier = callee.As<Identifier>();
                name = identifier.Name;

                arguments.AddRange(expression.Arguments);
            }
            else if (callee.Type == SyntaxNodes.MemberExpression)
            {
                var member = callee.As<MemberExpression>();
                var property = member.Property;
                if (property.Type == SyntaxNodes.Identifier)
                {
                    var identifier = property.As<Identifier>();
                    name = identifier.Name;

                    arguments.Add(member.Object);
                    arguments.AddRange(expression.Arguments);
                }
            }

            if (string.IsNullOrEmpty(name))
                return false;

            var internalFunction = _module.Module.GetInternalFunction(name);
            if (internalFunction != null)
            {
                Emit(arguments);
                _code.Emit(PintaCode.CallInternal, internalFunction.Id, (uint)arguments.Count);
                if (discard)
                    _code.Emit(PintaCode.Pop);
                return true;
            }

            switch (name)
            {
                case "int":
                case "integer":
                    if (arguments.Count != 1)
                        return false;
                    Emit(arguments);
                    _code.Emit(PintaCode.ConvertInteger);
                    break;
                case "decimal":
                    if (arguments.Count != 1)
                        return false;
                    Emit(arguments);
                    _code.Emit(PintaCode.ConvertDecimal);
                    break;
                case "string":
                    if (arguments.Count != 1)
                        return false;
                    Emit(arguments);
                    _code.Emit(PintaCode.ConvertString);
                    break;
                case "concat":
                    if (arguments.Count == 1)
                    {
                        Emit(arguments);
                        _code.Emit(PintaCode.ConvertString);
                    }
                    else if (arguments.Count > 0)
                    {
                        Emit(arguments[0]);

                        for (var i = 1; i < arguments.Count; i++)
                        {
                            var current = arguments[i];
                            Emit(current);
                            _code.Emit(PintaCode.Concat);
                        }
                    }
                    break;
                case "substr":
                case "substring":
                    if (arguments.Count != 3)
                        return false;
                    using (var local = _scope.DeclareTemporaryLocal())
                    {
                        Emit(arguments[0]);
                        _code.Emit(PintaCode.StoreLocal, local.Local);

                        Emit(arguments[1]); // this is correct order, I know this is stupid :(, cannot change that anymore - compatibility
                        Emit(arguments[2]);
                        _code.Emit(PintaCode.LoadLocal, local.Local);
                        _code.Emit(PintaCode.Substring);
                    }
                    break;
                case "hex":
                    if (arguments.Count != 1)
                        return false;
                    EmitBinary(arguments[0], PintaCodeBinaryType.Hex);
                    break;
                case "base64":
                    if (arguments.Count != 1)
                        return false;
                    EmitBinary(arguments[0], PintaCodeBinaryType.Base64);
                    break;
                case "ascii":
                    if (arguments.Count != 1)
                        return false;
                    EmitBinary(arguments[0], PintaCodeBinaryType.Ascii);
                    break;
                case "utf8":
                    if (arguments.Count != 1)
                        return false;
                    EmitBinary(arguments[0], PintaCodeBinaryType.Utf8);
                    break;
                case "exit":
                    _code.Emit(PintaCode.Exit);
                    break;
                case "$match_simple":
                    if (arguments.Count < 2 || arguments.Count > 4)
                        return false;

                    for (var i = 0; i < arguments.Count; i++)
                        Emit(arguments[i]);

                    _code.Emit(PintaCode.Match, PintaCodeMatchFlags.Simple | GetMatchArgumentsCount(arguments.Count));
                    break;

                case "$match_pan":
                    if (arguments.Count < 2 || arguments.Count > 3)
                        return false;

                    for (var i = 0; i < arguments.Count; i++)
                        Emit(arguments[i]);

                    _code.Emit(PintaCode.Match, PintaCodeMatchFlags.PrimaryAccountNumber | GetMatchArgumentsCount(arguments.Count));
                    break;

                default:
                    return false;
            }

            if (discard)
                _code.Emit(PintaCode.Pop);

            return true;
        }

        private static PintaCodeMatchFlags GetMatchArgumentsCount(int count)
        {
            var argumentsCount = default(PintaCodeMatchFlags); // I know that I could use (arguments.Count << 4)
            switch (count)
            {
                case 1:
                    argumentsCount = PintaCodeMatchFlags.ArgumentCount1;
                    break;
                case 2:
                    argumentsCount = PintaCodeMatchFlags.ArgumentCount2;
                    break;
                case 3:
                    argumentsCount = PintaCodeMatchFlags.ArgumentCount3;
                    break;
                case 4:
                    argumentsCount = PintaCodeMatchFlags.ArgumentCount4;
                    break;
            }

            return argumentsCount;
        }

        protected virtual void EmitUpdateIdentifier(SyntaxNode parentNode, Identifier left, bool isPrefix, UnaryOperator unaryOperator, bool discard = false)
        {
            var local = _scope.GetLocal(left.Name);
            if (local != null)
            {
                _code.Emit(PintaCode.LoadLocal, local.Local);

                EmitUnaryOperator(parentNode, unaryOperator, isPrefix, discard);

                _code.Emit(PintaCode.StoreLocal, local.Local);
                return;
            }

            var parameter = _scope.GetParameter(left.Name);
            if (parameter != null)
            {
                _code.Emit(PintaCode.LoadArgument, parameter.Parameter);

                EmitUnaryOperator(parentNode, unaryOperator, isPrefix, discard);

                _code.Emit(PintaCode.StoreArgument, parameter.Parameter);
                return;
            }

            var closure = _scope.GetClosure(left.Name);
            if (closure != null)
            {
                _code.Emit(PintaCode.LoadClosure, closure.Closure);

                EmitUnaryOperator(parentNode, unaryOperator, isPrefix, discard);

                _code.Emit(PintaCode.StoreClosure, closure.Closure);
                return;
            }

            var global = _module.Module.GetGlobal(left.Name);
            _code.Emit(PintaCode.LoadGlobal, global);

            EmitUnaryOperator(parentNode, unaryOperator, isPrefix, discard);

            _code.Emit(PintaCode.StoreGlobal, global);
        }

        protected virtual void EmitUpdateMember(SyntaxNode parentNode, MemberExpression left, bool isPrefix, UnaryOperator unaryOperator, bool discard = false)
        {
            using (var value = _scope.DeclareTemporaryLocal())
            {
                Emit(left.Object);
                Emit(left.Property);
                _code.Emit(PintaCode.DuplicateMultiple, 2U);
                _code.Emit(PintaCode.LoadMember);

                if (!isPrefix && !discard)
                {
                    _code.Emit(PintaCode.Duplicate);
                    _code.Emit(PintaCode.StoreLocal, value.Local);
                }

                EmitUnaryOperator(parentNode, unaryOperator);

                if (isPrefix && !discard)
                {
                    _code.Emit(PintaCode.Duplicate);
                    _code.Emit(PintaCode.StoreLocal, value.Local);
                }

                _code.Emit(PintaCode.StoreMember);
                _code.Emit(PintaCode.LoadLocal, value.Local);
            }
        }

        protected virtual void EmitAssignmentIdentifier(SyntaxNode parentNode, Identifier left, Expression right, AssignmentOperator assignmentOperator, bool discard = false)
        {
            var isCompound = assignmentOperator != AssignmentOperator.Assign;

            var local = _scope.GetLocal(left.Name);
            if (local != null)
            {
                if (isCompound)
                    _code.Emit(PintaCode.LoadLocal, local.Local);

                Emit(right);
                if (isCompound)
                    EmitAssignmentOperator(parentNode, assignmentOperator);

                if (!discard)
                    _code.Emit(PintaCode.Duplicate);
                _code.Emit(PintaCode.StoreLocal, local.Local);
                return;
            }

            var parameter = _scope.GetParameter(left.Name);
            if (parameter != null)
            {
                if (isCompound)
                    _code.Emit(PintaCode.LoadArgument, parameter.Parameter);

                Emit(right);
                if (isCompound)
                    EmitAssignmentOperator(parentNode, assignmentOperator);

                if (!discard)
                    _code.Emit(PintaCode.Duplicate);
                _code.Emit(PintaCode.StoreArgument, parameter.Parameter);
                return;
            }

            var closure = _scope.GetClosure(left.Name);
            if (closure != null)
            {
                if (isCompound)
                    _code.Emit(PintaCode.LoadClosure, closure.Closure);

                Emit(right);
                if (isCompound)
                    EmitAssignmentOperator(parentNode, assignmentOperator);

                if (!discard)
                    _code.Emit(PintaCode.Duplicate);
                _code.Emit(PintaCode.StoreClosure, closure.Closure);
                return;
            }

            var global = _module.Module.GetGlobal(left.Name);
            if (isCompound)
                _code.Emit(PintaCode.LoadGlobal, global);

            Emit(right);
            if (isCompound)
                EmitAssignmentOperator(parentNode, assignmentOperator);

            if (!discard)
                _code.Emit(PintaCode.Duplicate);
            _code.Emit(PintaCode.StoreGlobal, global);
        }

        protected virtual void EmitAssignmentMember(SyntaxNode parentNode, MemberExpression left, Expression right, AssignmentOperator assignmentOperator, bool discard = false)
        {
            if (left.Computed && left.Property.Type == SyntaxNodes.Literal)
            {
                var literal = left.Property.As<Literal>();
                var index = 0;
                if (int.TryParse(literal.Raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out index))
                {
                    EmitAssignmentItem(parentNode, left, right, assignmentOperator, discard);
                    return;
                }
            }

            if (assignmentOperator == AssignmentOperator.Assign)
            {
                Emit(left.Object);

                if (!left.Computed && left.Property.Type == SyntaxNodes.Identifier)
                {
                    var property = left.Property.As<Identifier>();
                    _code.Emit(PintaCode.LoadString, property.Name);
                }
                else
                {
                    Emit(left.Property);
                }

                Emit(right);
                if (discard)
                {
                    _code.Emit(PintaCode.StoreMember);
                }
                else
                {
                    using (var temp = _scope.DeclareTemporaryLocal())
                    {
                        _code.Emit(PintaCode.Duplicate);
                        _code.Emit(PintaCode.StoreLocal, temp.Local);
                        _code.Emit(PintaCode.StoreMember);
                        _code.Emit(PintaCode.LoadLocal, temp.Local);
                    }
                }
            }
            else
            {
                Emit(left.Object);

                if (!left.Computed && left.Property.Type == SyntaxNodes.Identifier)
                {
                    var property = left.Property.As<Identifier>();
                    _code.Emit(PintaCode.LoadString, property.Name);
                }
                else
                {
                    Emit(left.Property);
                }

                _code.Emit(PintaCode.DuplicateMultiple, 2U);
                _code.Emit(PintaCode.LoadMember);
                Emit(right);
                EmitAssignmentOperator(parentNode, assignmentOperator);
                if (discard)
                {
                    _code.Emit(PintaCode.StoreMember);
                }
                else
                {
                    using (var temp = _scope.DeclareTemporaryLocal())
                    {
                        _code.Emit(PintaCode.Duplicate);
                        _code.Emit(PintaCode.StoreLocal, temp.Local);
                        _code.Emit(PintaCode.StoreMember);
                        _code.Emit(PintaCode.LoadLocal, temp.Local);
                    }
                }
            }
        }

        private void EmitAssignmentItem(SyntaxNode parentNode, MemberExpression left, Expression right, AssignmentOperator assignmentOperator, bool discard)
        {
            if (assignmentOperator == AssignmentOperator.Assign)
            {
                using (var objectTemp = _scope.DeclareTemporaryLocal())
                using (var indexTemp = _scope.DeclareTemporaryLocal())
                {
                    Emit(left.Object);
                    _code.Emit(PintaCode.StoreLocal, objectTemp.Local);

                    Emit(left.Property);
                    _code.Emit(PintaCode.StoreLocal, indexTemp.Local);

                    Emit(right);

                    if (!discard)
                        _code.Emit(PintaCode.Duplicate);

                    _code.Emit(PintaCode.LoadLocal, indexTemp.Local);
                    _code.Emit(PintaCode.LoadLocal, objectTemp.Local);
                    _code.Emit(PintaCode.StoreItem);
                }
            }
            else
            {
                using (var objectTemp = _scope.DeclareTemporaryLocal())
                using (var indexTemp = _scope.DeclareTemporaryLocal())
                {

                    Emit(left.Object);
                    _code.Emit(PintaCode.Duplicate);
                    _code.Emit(PintaCode.StoreLocal, objectTemp.Local);

                    Emit(left.Property);
                    _code.Emit(PintaCode.Duplicate);
                    _code.Emit(PintaCode.StoreLocal, indexTemp.Local);

                    _code.Emit(PintaCode.LoadItem);
                    Emit(right);
                    EmitAssignmentOperator(parentNode, assignmentOperator);

                    if (!discard)
                        _code.Emit(PintaCode.Duplicate);

                    _code.Emit(PintaCode.LoadLocal, indexTemp.Local);
                    _code.Emit(PintaCode.LoadLocal, objectTemp.Local);
                    _code.Emit(PintaCode.StoreItem);
                }
            }
        }

        protected virtual void EmitUnaryOperator(SyntaxNode node, UnaryOperator unaryOperator, bool isPrefix, bool discard = false)
        {
            if (!isPrefix && !discard)
                _code.Emit(PintaCode.Duplicate);

            EmitUnaryOperator(node, unaryOperator);

            if (isPrefix && !discard)
                _code.Emit(PintaCode.Duplicate);
        }

        protected virtual void EmitUnaryOperator(SyntaxNode node, UnaryOperator unaryOperator)
        {
            switch (unaryOperator)
            {
                case UnaryOperator.Increment:
                    _code.Emit(PintaCode.LoadIntegerOne);
                    _code.Emit(PintaCode.Add);
                    break;
                case UnaryOperator.Decrement:
                    _code.Emit(PintaCode.LoadIntegerOne);
                    _code.Emit(PintaCode.Subtract);
                    break;
                default:
                    throw new PintaCompilerException(node, string.Format("Unsupported unary operator {0}", unaryOperator));
            }
        }

        protected virtual void EmitAssignmentOperator(SyntaxNode node, AssignmentOperator assignmentOperator)
        {
            switch (assignmentOperator)
            {
                case AssignmentOperator.PlusAssign:
                    _code.Emit(PintaCode.Add);
                    break;
                case AssignmentOperator.MinusAssign:
                    _code.Emit(PintaCode.Subtract);
                    break;
                case AssignmentOperator.TimesAssign:
                    _code.Emit(PintaCode.Multiply);
                    break;
                case AssignmentOperator.DivideAssign:
                    _code.Emit(PintaCode.Divide);
                    break;
                case AssignmentOperator.ModuloAssign:
                    _code.Emit(PintaCode.Remainder);
                    break;
                case AssignmentOperator.BitwiseAndAssign:
                    _code.Emit(PintaCode.BitwiseAnd);
                    break;
                case AssignmentOperator.BitwiseOrAssign:
                    _code.Emit(PintaCode.BitwiseOr);
                    break;
                case AssignmentOperator.BitwiseXOrAssign:
                    _code.Emit(PintaCode.BitwiseExclusiveOr);
                    break;
                case AssignmentOperator.LeftShiftAssign:
                    throw new PintaCompilerException(node, "Operator '<<=' is not supported");
                case AssignmentOperator.RightShiftAssign:
                    throw new PintaCompilerException(node, "Operator '>>=' is not supported");
                case AssignmentOperator.UnsignedRightShiftAssign:
                    throw new PintaCompilerException(node, "Operator '>>>=' is not supported");
                default:
                    throw new PintaCompilerException(node, string.Format("Unsupported binary operator {0}", assignmentOperator));
            }
        }

        protected virtual void EmitBinary(Expression expression, PintaCodeBinaryType type)
        {
            if (expression.Type != SyntaxNodes.Literal)
                throw new PintaCompilerException(expression, string.Format("Binary constant must be a literal string, got {0}", expression.Type));

            var literal = expression.As<Literal>();
            var stringValue = literal.Value as string;
            if (string.IsNullOrEmpty(stringValue))
                throw new PintaCompilerException(expression, "Binary constant must be non-empty string");

            if (type == PintaCodeBinaryType.Hex)
            {
                if (!_hexRegex.IsMatch(stringValue))
                    throw new PintaCompilerException(expression, "Binary hex constant must be only contain digits or letter a-z or A-Z");
            }
            else if (type == PintaCodeBinaryType.Base64)
            {
                try
                {
                    Convert.FromBase64String(stringValue);
                }
                catch (Exception)
                {
                    throw new PintaCompilerException(expression, "Binary base64 constant is not valid");
                }
            }

            _code.Emit(PintaCode.LoadBinary, type, stringValue);
        }
    }
}
