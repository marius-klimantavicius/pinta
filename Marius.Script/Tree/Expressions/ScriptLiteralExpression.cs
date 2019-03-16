using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Globalization;

namespace Marius.Script.Tree
{
    public class ScriptLiteralExpression: ScriptExpression
    {
        public static readonly ScriptLiteralExpression This = new ScriptLiteralExpression(ScriptLiteralKind.This);
        public static readonly ScriptLiteralExpression True = new ScriptLiteralExpression(ScriptLiteralKind.True);
        public static readonly ScriptLiteralExpression False = new ScriptLiteralExpression(ScriptLiteralKind.False);
        public static readonly ScriptLiteralExpression Null = new ScriptLiteralExpression(ScriptLiteralKind.Null);

        public ScriptLiteralKind Kind { get; private set; }
        public object Value { get; set; }

        public ScriptLiteralExpression()
        {
        }

        public ScriptLiteralExpression(ScriptLiteralKind kind)
        {
            Kind = kind;
        }

        public ScriptLiteralExpression(ScriptLiteralKind kind, object value)
        {
            Kind = kind;
            Value = value;
        }

        public ScriptLiteralExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptLiteralExpression(ScriptLiteralKind kind, ScriptSourceSpan location)
            : base(location)
        {
            Kind = kind;
        }

        public ScriptLiteralExpression(ScriptLiteralKind kind, object value, ScriptSourceSpan location)
            : base(location)
        {
            Kind = kind;
            Value = value;
        }

        public static ScriptLiteralExpression Numeric(string value, ScriptSourceSpan location)
        {
            return new ScriptLiteralExpression(ScriptLiteralKind.Numeric, value, location);
        }

        public static ScriptLiteralExpression String(string value, ScriptSourceSpan location)
        {
            return new ScriptLiteralExpression(ScriptLiteralKind.String, value, location);
        }

        public static ScriptLiteralExpression Regex(string value, ScriptSourceSpan location)
        {
            return new ScriptLiteralExpression(ScriptLiteralKind.Regex, value, location);
        }

        public override ScriptType PredictType()
        {
            switch (Kind)
            {
                case ScriptLiteralKind.This:
                    return ScriptType.Object;
                case ScriptLiteralKind.String:
                    return ScriptType.String;
                case ScriptLiteralKind.Numeric:
                    return ScriptType.Numeric;
                case ScriptLiteralKind.Regex:
                    return ScriptType.Object;
                case ScriptLiteralKind.True:
                    return ScriptType.Boolean;
                case ScriptLiteralKind.False:
                    return ScriptType.Boolean;
                case ScriptLiteralKind.Null:
                    return ScriptType.Object;
            }

            return ScriptType.Object;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            switch (Kind)
            {
                case ScriptLiteralKind.This:
                    writer.Write("this");
                    break;
                case ScriptLiteralKind.String:
                    writer.Write(Escape((string)Value));
                    break;
                case ScriptLiteralKind.Numeric:
                    writer.Write(Value);
                    break;
                case ScriptLiteralKind.True:
                    writer.Write("true");
                    break;
                case ScriptLiteralKind.False:
                    writer.Write("false");
                    break;
                case ScriptLiteralKind.Null:
                    writer.Write("null");
                    break;
                case ScriptLiteralKind.Regex:
                    writer.Write(Value);
                    break;
            }
        }

        public string Escape(string value)
        {
            var sb = new StringBuilder();
            sb.Append("'");

            foreach (var item in value)
            {
                switch (item)
                {
                    case '"':
                        sb.Append("\\\"");
                        break;
                    case '\'':
                        sb.Append("\\'");
                        break;
                    case '\\':
                        sb.Append("\\\\");
                        break;
                    case '\b':
                        sb.Append("\\b");
                        break;
                    case '\f':
                        sb.Append("\\f");
                        break;
                    case '\n':
                        sb.Append("\\n");
                        break;
                    case '\r':
                        sb.Append("\\r");
                        break;
                    case '\t':
                        sb.Append("\\t");
                        break;
                    case '\v':
                        sb.Append("\\v");
                        break;
                    case ' ':
                        sb.Append(' ');
                        break;
                    default:

                        if (item < 0x7F && !char.IsControl(item))
                            sb.Append(item);
                        else
                            sb.AppendFormat("\\u{0:X4}", (int)item);

                        break;
                }
            }

            sb.Append("'");

            return sb.ToString();
        }
#endif
    }
}
