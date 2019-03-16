using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Code
{
    public class PintaFunctionInner
    {
        private HashSet<PintaFunction> _callers;
        private PintaFunction _function;

        public string Name { get; private set; }

        public PintaFunction Function
        {
            get
            {
                return _function;
            }
            set
            {
                _function = value;
                if (_function != null)
                    _function.CallerFunctions = _callers;
            }
        }

        public PintaFunctionLocal Local { get; set; }

        public PintaFunctionInner(string name)
        {
            Name = name;

            _callers = new HashSet<PintaFunction>();
        }

        public void AddCaller(PintaFunction function)
        {
            _callers.Add(function);
        }
    }
}
