using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptTryStatement: ScriptStatement
    {
        public ScriptStatement TryBody { get; set; }
        public ScriptIdentifier Exception { get; set; }
        public ScriptStatement CatchBody { get; set; }
        public ScriptStatement FinallyBody { get; set; }

        public ScriptTryStatement()
        {
        }

        public ScriptTryStatement(ScriptStatement tryBody, ScriptIdentifier exception, ScriptStatement catchBody, ScriptStatement finallyBody)
        {
            TryBody = tryBody;
            Exception = exception;
            CatchBody = catchBody;
            FinallyBody = finallyBody;
        }

        public ScriptTryStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptTryStatement(ScriptStatement tryBody, ScriptIdentifier exception, ScriptStatement catchBody, ScriptStatement finallyBody, ScriptSourceSpan location)
            : base(location)
        {
            TryBody = tryBody;
            Exception = exception;
            CatchBody = catchBody;
            FinallyBody = finallyBody;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.WriteLine("try");
            WriteIndentedStatement(TryBody, writer, false);

            if (Exception != null && CatchBody != null)
            {
                writer.Write("catch (");
                Exception.WriteDebugView(writer);
                writer.WriteLine(")");
                WriteIndentedStatement(CatchBody, writer, false);
            }

            if (FinallyBody != null)
            {
                writer.Write("finally");
                WriteIndentedStatement(FinallyBody, writer, false);
            }

            writer.WriteLine();
        }
#endif
    }
}
