using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaProgramVariable : PintaNode<PintaProgramVariable.NodeData>
    {
        public class NodeData
        {
            public uint Index;

            public PintaProgramString String;
        }

        public PintaProgramBuilder Program { get; set; }
        public string Name { get; set; }

        public PintaProgramVariable(PintaProgramBuilder program, string name)
        {
            Program = program;
            Name = name;
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
