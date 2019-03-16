using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public enum ScriptBinaryOperator
    {
        Multiply,
        Divide,
        Modulus,
        Add,
        Subtract,
        ShiftLeft,
        ShiftRight,
        ShiftArithmeticRight,
        More,
        Less,
        MoreOrEqual,
        LessOrEqual,
        InstanceOf,
        In,
        Equals,
        NotEquals,
        StrictEquals,
        StrictNotEquals,
        BitwiseAnd,
        BitwiseOr,
        ExclusiveOr,
        LogicalAnd,
        LogicalOr,
    }
}
