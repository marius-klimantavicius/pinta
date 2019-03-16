using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptFunctionDeclaration: ScriptSourceElement
    {
        public ScriptFunctionExpression Function { get; set; }

        public ScriptFunctionDeclaration()
        {
        }

        public ScriptFunctionDeclaration(ScriptIdentifier name, IEnumerable<ScriptIdentifier> parameterList, IEnumerable<ScriptSourceElement> body)
        {
            Function = new ScriptFunctionExpression(name, parameterList, body);
        }

        public ScriptFunctionDeclaration(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptFunctionDeclaration(ScriptIdentifier name, IEnumerable<ScriptIdentifier> parameterList, IEnumerable<ScriptSourceElement> body, ScriptSourceSpan location)
            : base(location)
        {
            Function = new ScriptFunctionExpression(name, parameterList, body);
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Function.WriteDebugView(writer);
        }
#endif
    }
}
