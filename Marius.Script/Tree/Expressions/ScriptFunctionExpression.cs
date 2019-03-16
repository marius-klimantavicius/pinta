using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptFunctionExpression: ScriptExpression
    {
        public ScriptIdentifier Name { get; set; }
        public List<ScriptIdentifier> ParameterList { get; private set; }
        public List<ScriptSourceElement> Body { get; private set; }

        public ScriptFunctionExpression()
        {
            ParameterList = new List<ScriptIdentifier>();
            Body = new List<ScriptSourceElement>();
        }

        public ScriptFunctionExpression(ScriptIdentifier name, IEnumerable<ScriptIdentifier> parameterList, IEnumerable<ScriptSourceElement> body)
        {
            Name = name;

            if (parameterList != null)
                ParameterList = new List<ScriptIdentifier>(parameterList);
            else
                ParameterList = new List<ScriptIdentifier>();

            if (body != null)
                Body = new List<ScriptSourceElement>(body);
            else
                Body = new List<ScriptSourceElement>();
        }

        public ScriptFunctionExpression(ScriptSourceSpan location)
            : base(location)
        {
            ParameterList = new List<ScriptIdentifier>();
            Body = new List<ScriptSourceElement>();
        }

        public ScriptFunctionExpression(ScriptIdentifier name, IEnumerable<ScriptIdentifier> parameterList, IEnumerable<ScriptSourceElement> body, ScriptSourceSpan location)
            : base(location)
        {
            Name = name;

            if (parameterList != null)
                ParameterList = new List<ScriptIdentifier>(parameterList);
            else
                ParameterList = new List<ScriptIdentifier>();

            if (body != null)
                Body = new List<ScriptSourceElement>(body);
            else
                Body = new List<ScriptSourceElement>();
        }

        public override ScriptType PredictType()
        {
            return ScriptType.Function;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("function");

            if (Name != null)
            {
                writer.Write(" ");
                Name.WriteDebugView(writer);
            }

            writer.Write("(");

            var first = true;
            foreach (var item in ParameterList)
            {
                if (!first)
                    writer.Write(", ");

                item.WriteDebugView(writer);
                first = false;
            }

            writer.WriteLine(") {");

            writer.Indent++;
            foreach (var item in Body)
            {
                item.WriteDebugView(writer);
            }
            writer.Indent--;

            writer.WriteLine("}");
        }
#endif
    }
}
