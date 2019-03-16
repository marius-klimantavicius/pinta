using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public partial class PintaFunctionCompiler
    {
        private PintaModule _module;
        private PintaCodeGenerator _code;
        private PintaScope _scope;
        private PintaLabelScope _labelScope;

        public PintaFunctionCompiler(PintaModule module, PintaCodeGenerator code, PintaScope scope)
        {
            _module = module;
            _code = code;
            _scope = scope;
            _labelScope = new PintaLabelScope();
        }
    }
}
