using System;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public abstract class PintaNode<T>
        where T: new()
    {
        public T Data { get; private set; }

        public PintaNode()
        {
            Data = new T();
        }

        public abstract void Accept(IPintaNodeVisitor visitor);
    }
}
