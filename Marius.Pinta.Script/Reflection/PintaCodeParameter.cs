using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeParameter
    {
        public uint Id { get; set; }

        public string Name { get; private set; }

        public PintaCodeParameter(string name)
        {
            Name = name;
        }

        public override string ToString()
        {
            return "param." + Name;
        }
    }
}
