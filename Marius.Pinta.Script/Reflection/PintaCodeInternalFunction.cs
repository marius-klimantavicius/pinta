using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeInternalFunction
    {
        public string Name { get; set; }
        public uint Id { get; set; }

        public PintaCodeInternalFunction(string name, uint id)
        {
            Name = name;
            Id = id;
        }
    }
}
