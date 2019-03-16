using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaFunctionLocal
    {
        public string Name { get; set; }
        public PintaCodeLocal Local { get; set; }

        public bool IsUsed { get; set; }

        public PintaFunctionLocal(string name, PintaCodeLocal local)
        {
            Name = name;
            Local = local;
        }
    }
}
