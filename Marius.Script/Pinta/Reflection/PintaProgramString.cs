using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaProgramString : PintaNode<PintaProgramString.NodeData>
    {
        public class NodeData
        {
            public uint Index;
            public uint Offset;
        }

        public string Value { get; private set; }

        public PintaProgramString(string value)
        {
            Value = value;
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
