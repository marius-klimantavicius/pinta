using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public struct ScriptSourceLocation
    {
        public readonly int Line;
        public readonly int Column;
        public readonly int Position;

        public ScriptSourceLocation(int line, int column, int position)
        {
            Line = line;
            Column = column;
            Position = position;
        }

        public override string ToString()
        {
            return string.Format("{0}:{1}", Line, Column);
        }
    }
}
