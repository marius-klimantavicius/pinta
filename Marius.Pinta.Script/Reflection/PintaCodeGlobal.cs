using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeGlobal
    {
        public uint Id { get; set; }

        public PintaCodeString Name { get; private set; }

        public PintaCodeGlobal(PintaCodeString name)
        {
            Name = name;
        }

        public override string ToString()
        {
            return "global." + Name.Value;
        }

    }
}
