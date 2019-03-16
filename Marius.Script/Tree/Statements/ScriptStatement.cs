using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public abstract class ScriptStatement: ScriptSourceElement
    {
        public ScriptStatement()
        {
        }

        public ScriptStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

#if DEBUG
        protected void WriteIndentedStatement(ScriptStatement statement, System.CodeDom.Compiler.IndentedTextWriter writer, bool appendNewLine = true)
        {
            if (statement is ScriptBlockStatement)
                statement.WriteDebugView(writer);
            else
            {
                writer.Indent++;
                statement.WriteDebugView(writer);
                writer.Indent--;
            }

            if (appendNewLine)
                writer.WriteLine();
        }
#endif
    }
}
