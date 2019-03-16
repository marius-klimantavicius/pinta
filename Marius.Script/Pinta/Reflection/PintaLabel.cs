using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaLabel : PintaNode<PintaLabel.NodeData>
    {
        public class NodeData
        {
            public int Offset;

            public int OffsetMin;
            public int OffsetMax;
        }

        private static int _idGenerator;

        public PintaFunctionBuilder Function { get; private set; }
        public int Id { get; private set; }
        
        public bool IsEmitted { get; set; }

        public PintaLabel(PintaFunctionBuilder function)
        {
            Function = function;
            Id = Interlocked.Increment(ref _idGenerator); 
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
