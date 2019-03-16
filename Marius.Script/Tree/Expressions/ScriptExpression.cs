using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public abstract class ScriptExpression: ScriptNode
    {
        public ScriptExpression()
        {
        }

        public ScriptExpression(ScriptSourceSpan location)
            : base(location)
        {
        }

        public abstract ScriptType PredictType();
    }
}
