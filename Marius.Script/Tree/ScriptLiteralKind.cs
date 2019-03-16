using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public enum ScriptLiteralKind
    {
        This,
        String,
        Numeric,
        Regex,
        True,
        False,
        Null,
    }
}
