using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public abstract class ScriptSourceElement: ScriptNode
    {
        public ScriptSourceElement()
        {
        }

        public ScriptSourceElement(ScriptSourceSpan location)
            : base(location)
        {
        }
    }
}
