using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;
using System.Diagnostics;

namespace Marius.Script.Tree
{
    [DebuggerDisplay("{Name}")]
    public abstract class ScriptVariable : ScriptNode
    {
        public ScriptIdentifier Name { get; set; }
        public abstract bool HasInitializer { get; }

        public ScriptVariable()
        {
        }

        public ScriptVariable(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptVariable(ScriptIdentifier name)
        {
            Name = name;
        }

        public ScriptVariable(ScriptIdentifier name, ScriptSourceSpan location)
            : base(location)
        {
            Name = name;
        }
    }
}
