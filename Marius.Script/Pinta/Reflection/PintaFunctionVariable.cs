using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaFunctionVariable : PintaNode<PintaFunctionVariable.NodeData>
    {
        public class NodeData
        {
            public uint Index;
        }

        public PintaFunctionBuilder Function { get; private set; }

        public string DebugName { get; set; }

        public PintaFunctionVariable(PintaFunctionBuilder function)
        {
            Function = function;
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
