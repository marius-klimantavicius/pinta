using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeLocal
    {
        public uint Id { get; set; }

        public string Name { get; private set; }

        public PintaCodeLocal(string name)
        {
            Name = name;
        }

        public override string ToString()
        {
            return "local." + Name;
        }
    }
}
