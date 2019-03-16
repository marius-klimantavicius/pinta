using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeString
    {
        public uint Id { get; set; }

        public string Value { get; private set; }

        public PintaCodeString(string value)
        {
            Value = value;
        }
    }
}
