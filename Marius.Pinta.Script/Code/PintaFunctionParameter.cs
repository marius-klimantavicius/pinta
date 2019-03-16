using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaFunctionParameter
    {
        public string Name { get; set; }
        public PintaCodeParameter Parameter { get; set; }

        public PintaFunctionParameter(string name, PintaCodeParameter parameter)
        {
            Name = name;
            Parameter = parameter;
        }
    }
}
