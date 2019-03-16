using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaFunctionParameter : PintaNode<PintaFunctionParameter.NodeData>
    {
        public class NodeData
        {
            public uint Index;
        }

        public PintaFunctionBuilder Function { get; private set; }
        public string DebugName { get; set; }

        public PintaFunctionParameter(PintaFunctionBuilder function)
        {
            Function = function;
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
