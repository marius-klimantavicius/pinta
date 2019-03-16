using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    [DebuggerDisplay("{Name} : {Value}")]
    public class ScriptNameValue: ScriptNode
    {
        public ScriptNameValueKind Kind { get; set; }
        public ScriptExpression Name { get; set; }
        public ScriptExpression Value { get; set; }

        public ScriptNameValue()
        {
        }

        public ScriptNameValue(ScriptExpression name, ScriptExpression value)
        {
            Kind = ScriptNameValueKind.Normal;
            Name = name;
            Value = value;
        }

        public ScriptNameValue(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptNameValue(ScriptExpression name, ScriptExpression value, ScriptSourceSpan location)
            : base(location)
        {
            Kind = ScriptNameValueKind.Normal;
            Name = name;
            Value = value;
        }

        public ScriptNameValue(ScriptExpression name, ScriptExpression value, ScriptNameValueKind kind, ScriptSourceSpan location)
            : base(location)
        {
            Kind = kind;
            Name = name;
            Value = value;
        }

        public static ScriptNameValue Getter(ScriptExpression name, ScriptFunctionExpression getter, ScriptSourceSpan location)
        {
            return new ScriptNameValue(name, getter, ScriptNameValueKind.Get, location);
        }

        public static ScriptNameValue Setter(ScriptExpression name, ScriptFunctionExpression setter, ScriptSourceSpan location)
        {
            return new ScriptNameValue(name, setter, ScriptNameValueKind.Set, location);
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Name.WriteDebugView(writer);
            writer.Write(" : ");
            Value.WriteDebugView(writer);
        }
#endif
    }
}
