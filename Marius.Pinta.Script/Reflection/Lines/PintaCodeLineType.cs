using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public enum PintaCodeLineType
    {
        Binary,
        Call,
        CallInternal,
        Closure,
        Code,
        Global,
        Integer,
        Invoke,
        Label,
        LabelMark,
        Local,
        NewFunction,
        Parameter,
        String,
        Match,
    }
}
