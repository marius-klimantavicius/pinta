using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeFunction
    {
        private PintaCodeModule _module;
        private PintaCodeGenerator _codeGenerator;

        private List<PintaCodeLocal> _locals;
        private List<PintaCodeParameter> _parameters;

        public uint Id { get; set; }

        public string Name { get; private set; }

        public IEnumerable<PintaCodeLine> Body { get { return _codeGenerator.Body; } }

        public ICollection<PintaCodeParameter> Parameters { get { return _parameters; } }
        public ICollection<PintaCodeLocal> Locals { get { return _locals; } }

        public PintaCodeFunction(PintaCodeModule module, string name)
        {
            Name = name;

            _module = module;
            _codeGenerator = new PintaCodeGenerator(module);

            _locals = new List<PintaCodeLocal>();
            _parameters = new List<PintaCodeParameter>();
        }

        public PintaCodeGenerator GetCodeGenerator()
        {
            return _codeGenerator;
        }

        public PintaCodeLocal DeclareLocal(string name)
        {
            var result = _codeGenerator.DeclareLocal(name);
            _locals.Add(result);
            return result;
        }

        public PintaCodeParameter DeclareParameter(string name)
        {
            var result = _codeGenerator.DeclareParameter(name);
            _parameters.Add(result);
            return result;
        }

        public PintaCodeClosure DeclareClosure(uint closureIndex, PintaCodeLocal local)
        {
            return new PintaCodeClosure(closureIndex, local);
        }

        public PintaCodeClosure DeclareClosure(uint closureIndex, PintaCodeParameter parameter)
        {
            return new PintaCodeClosure(closureIndex, parameter);
        }
    }
}
