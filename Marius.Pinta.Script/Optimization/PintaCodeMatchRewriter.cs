using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script.Optimization
{
    public class PintaCodeMatchRewriter : PintaCodeRewriter
    {
        private class LengthCheck
        {
            public Identifier Identifier;
            public int Length;
        }

        public class Substring
        {
            public Identifier Identifier;
            public int Start;
            public int Length;
            public Expression Value;
        }

        private class PanRangeStart
        {
            public Identifier Identifier;
            public string RangeStart;
        }

        private class PanRangeEnd
        {
            public Identifier Identifier;
            public string RangeEnd;
        }

        private static Regex _digits = new Regex("^[0-9]+$", RegexOptions.Compiled | RegexOptions.Singleline);

        public override Expression RewriteLogicalExpression(LogicalExpression expression)
        {
            if (expression.Operator != LogicalOperator.LogicalAnd)
                return expression;
            /*
             * rewrite:
             *   ((lid:Identifier . 'length':Identifier) >= len:Literal:int) && (((rid: Identifier . 'substring':Identifier ) .call (start:Literal:int, end:Literal:int) ) == value:Expression)
             */
            // (AID.length >= 14) && AID.substring(0, 14) == 'A0000000032020'
            // PAN.length >= 1 && PAN[0] == '3'
            // '5218150000000000000' <= PAN && PAN <= '5218159999999999999'

            var left = expression.Left;
            var right = expression.Right;

            if (left.Type != SyntaxNodes.BinaryExpression)
                return expression;

            if (right.Type != SyntaxNodes.BinaryExpression)
                return expression;

            var result = default(Expression);
            if (CheckPanMatch(left.As<BinaryExpression>(), right.As<BinaryExpression>(), ref result))
                return result;

            var lengthTuple = CheckLength(left.As<BinaryExpression>());
            if (lengthTuple == null)
                return expression;

            var substring = CheckSubstring(right.As<BinaryExpression>());
            if (substring == null)
                return expression;

            if (lengthTuple.Identifier.Name != substring.Identifier.Name)
                return expression;

            var range = substring.Start + substring.Length;
            if (range > lengthTuple.Length)
                return expression;

            var arguments = new List<Expression>()
            {
                lengthTuple.Identifier,
                substring.Value,
            };

            var needOffset = true;
            var needLength = true;
            CheckOffsetAndLength(substring, ref needOffset, ref needLength);

            if (needOffset)
            {
                arguments.Add(new Literal()
                {
                    Type = SyntaxNodes.Literal,
                    Raw = substring.Start.ToString(),
                    Value = substring.Start,
                });
            }
            if (needLength)
            {
                arguments.Add(new Literal()
                {
                    Type = SyntaxNodes.Literal,
                    Raw = substring.Length.ToString(),
                    Value = substring.Length,
                });
            }

            return new CallExpression()
            {
                Type = SyntaxNodes.CallExpression,
                Callee = new Identifier()
                {
                    Type = SyntaxNodes.Identifier,
                    Name = "$match_simple",
                },
                Arguments = arguments,
            };
        }

        private bool CheckPanMatch(BinaryExpression left, BinaryExpression right, ref Expression result)
        {
            // '5218150000000000000' <= PAN && PAN <= '5218159999999999999'

            var start = CheckPanRangeStart(left);
            if (start == null)
                return false;

            var end = CheckPanRangeEnd(right);
            if (end == null)
                return false;

            if (start.Identifier.Name != end.Identifier.Name)
                return false;

            if (!_digits.IsMatch(start.RangeStart) || !_digits.IsMatch(end.RangeEnd))
                return false;

            if (start.RangeStart.Length != end.RangeEnd.Length)
                return false;

            var prefixLength = 0;
            for (var i = 0; i < start.RangeStart.Length; i++)
            {
                if (start.RangeStart[i] != end.RangeEnd[i])
                {
                    prefixLength = i;
                    break;
                }
            }

            for (var i = prefixLength; i < start.RangeStart.Length; i++)
            {
                if (start.RangeStart[i] != '0')
                    return false;

                if (end.RangeEnd[i] != '9')
                    return false;
            }

            var prefix = start.RangeStart.Substring(0, prefixLength);
            result = new CallExpression()
            {
                Type = SyntaxNodes.CallExpression,
                Callee = new Identifier()
                {
                    Type = SyntaxNodes.Identifier,
                    Name = "$match_pan",
                },
                Arguments = new List<Expression>()
                {
                    start.Identifier,
                    new Literal()
                    {
                        Type = SyntaxNodes.Literal,
                        Raw = prefix,
                        Value = prefix,
                    },
                    new Literal()
                    {
                        Type = SyntaxNodes.Literal,
                        Raw = start.RangeStart.Length.ToString(),
                        Value = start.RangeStart.Length,
                    },
                },
            };

            return true;
        }

        private PanRangeStart CheckPanRangeStart(BinaryExpression expression)
        {
            if (expression.Operator != BinaryOperator.LessOrEqual)
                return null;

            if (expression.Left.Type != SyntaxNodes.Literal)
                return null;

            if (expression.Right.Type != SyntaxNodes.Identifier)
                return null;

            var startLiteral = expression.Left.As<Literal>();
            var start = startLiteral.Value as string;
            if (string.IsNullOrEmpty(start))
                return null;

            var identifier = expression.Right.As<Identifier>();

            return new PanRangeStart()
            {
                Identifier = identifier,
                RangeStart = start,
            };
        }

        private PanRangeEnd CheckPanRangeEnd(BinaryExpression expression)
        {
            if (expression.Operator != BinaryOperator.LessOrEqual)
                return null;

            if (expression.Left.Type != SyntaxNodes.Identifier)
                return null;

            if (expression.Right.Type != SyntaxNodes.Literal)
                return null;

            var identifier = expression.Left.As<Identifier>();

            var startLiteral = expression.Right.As<Literal>();
            var start = startLiteral.Value as string;
            if (string.IsNullOrEmpty(start))
                return null;

            return new PanRangeEnd()
            {
                Identifier = identifier,
                RangeEnd = start,
            };
        }

        private void CheckOffsetAndLength(Substring substring, ref bool needOffset, ref bool needLength)
        {
            if (substring.Value.Type != SyntaxNodes.Literal)
                return;

            var literal = substring.Value.As<Literal>();
            var stringValue = literal.Value as string;
            if (stringValue == null)
                return;

            if (stringValue.Length == substring.Length)
                needLength = false;

            if (substring.Start == 0 && !needLength)
                needOffset = false;
        }

        private Substring CheckSubstring(BinaryExpression expression)
        {
            if (expression.Operator != BinaryOperator.Equal)
                return null;

            if (expression.Left.Type == SyntaxNodes.MemberExpression)
            {
                var member = expression.Left.As<MemberExpression>();
                if (!member.Computed)
                    return null;

                if (member.Object.Type != SyntaxNodes.Identifier)
                    return null;

                if (member.Property.Type != SyntaxNodes.Literal)
                    return null;

                var start = 0;
                if (!GetValue(member.Property.As<Literal>(), ref start))
                    return null;

                return new Substring()
                {
                    Identifier = member.Object.As<Identifier>(),
                    Start = start,
                    Length = 1,
                    Value = expression.Right,
                };
            }
            else if (expression.Left.Type == SyntaxNodes.CallExpression)
            {
                var call = expression.Left.As<CallExpression>();

                if (call.Callee.Type != SyntaxNodes.MemberExpression)
                    return null;

                if (call.Arguments == null)
                    return null;

                var member = call.Callee.As<MemberExpression>();
                if (member.Property.Type != SyntaxNodes.Identifier)
                    return null;

                if (member.Object.Type != SyntaxNodes.Identifier)
                    return null;

                var property = member.Property.As<Identifier>();
                if (property.Name != "substring")
                    return null;

                var arguments = call.Arguments.ToList();
                call.Arguments = arguments;

                if (arguments.Count != 2)
                    return null;

                var start = 0;
                if (arguments[0].Type != SyntaxNodes.Literal || !GetValue(arguments[0].As<Literal>(), ref start))
                    return null;

                var length = 0;
                if (arguments[1].Type != SyntaxNodes.Literal || !GetValue(arguments[1].As<Literal>(), ref length))
                    return null;

                return new Substring()
                {
                    Identifier = member.Object.As<Identifier>(),
                    Start = start,
                    Length = length,
                    Value = expression.Right,
                };
            }

            return null;
        }

        private LengthCheck CheckLength(BinaryExpression expression)
        {
            var left = default(Expression);
            var right = default(Expression);
            if (expression.Operator == BinaryOperator.GreaterOrEqual)
            {
                left = expression.Left;
                right = expression.Right;
            }
            else if (expression.Operator == BinaryOperator.LessOrEqual)
            {
                left = expression.Right;
                right = expression.Left;
            }
            else
            {
                return null;
            }

            if (left.Type != SyntaxNodes.MemberExpression)
                return null;

            if (right.Type != SyntaxNodes.Literal)
                return null;

            var member = left.As<MemberExpression>();
            if (member.Computed)
                return null;

            if (member.Property.Type != SyntaxNodes.Identifier)
                return null;

            var property = member.Property.As<Identifier>();
            if (property.Name != "length")
                return null;

            if (member.Object.Type != SyntaxNodes.Identifier)
                return null;

            var length = 0;
            if (!GetValue(right.As<Literal>(), ref length))
                return null;

            return new LengthCheck()
            {
                Identifier = member.Object.As<Identifier>(),
                Length = length
            };
        }

        private bool GetValue(Literal literal, ref int result)
        {
            if (literal == null)
                return false;

            if (literal.Value is int)
            {
                result = (int)literal.Value;
            }
            else if (literal.Value is long)
            {
                result = (int)(long)literal.Value;
            }
            else if (literal.Value is double)
            {
                if (!CheckValidInteger((double)literal.Value, ref result))
                    return false;
            }
            else if (literal.Value is float)
            {
                if (!CheckValidInteger((float)literal.Value, ref result))
                    return false;
            }
            else
            {
                return false;
            }

            return true;
        }

        private bool CheckValidInteger(double value, ref int result)
        {
            if (value < 0)
                return false;

            var diff = value - Math.Floor(value);
            if (diff < 1E-6)
            {
                result = (int)value;
                return true;
            }

            return false;
        }
    }
}
