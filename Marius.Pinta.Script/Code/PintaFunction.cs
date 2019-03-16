using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser.Ast;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaFunction
    {
        public PintaCodeFunction Function { get; set; }
        public PintaScope Scope { get; set; }
        public IEnumerable<Statement> Body { get; private set; }

        public bool IsSimple { get; set; }
        public IEnumerable<PintaFunction> CallerFunctions { get; set; }
        public IEnumerable<PintaFunctionInner> InnerFunctions { get; set; }

        public PintaFunction(PintaCodeFunction function, PintaScope scope, IEnumerable<Statement> body)
        {
            Function = function;
            Scope = scope;
            Body = body;
        }
    }
}
