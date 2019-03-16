using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public enum ScriptUnaryOperator
    {
        PostIncrement,
        PostDecrement,
        PreIncrement,
        PreDecrement,
        Delete,
        Void,
        Typeof,
        Plus,
        Minus,
        BitwiseNot,
        LogicalNot,
    }
}
