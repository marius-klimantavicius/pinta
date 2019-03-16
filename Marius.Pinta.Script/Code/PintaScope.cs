using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaScope
    {
        private Dictionary<string, PintaFunctionLocal> _locals;
        private Dictionary<string, PintaFunctionParameter> _parameters;
        private Dictionary<string, PintaFunctionInner> _functions;
        private Dictionary<string, PintaFunctionClosure> _closures;

        private uint _temporaryId;
        private Stack<PintaFunctionTemporaryLocal> _temporary;

        public PintaCodeFunction Function { get; private set; }
        public PintaScope Parent { get; private set; }
        public uint ClosureIndex { get; private set; }

        public PintaScope(PintaCodeFunction function, PintaScope parent)
        {
            Function = function;
            Parent = parent;

            if (parent != null)
                ClosureIndex = parent.ClosureIndex + 1;

            _locals = new Dictionary<string, PintaFunctionLocal>(StringComparer.Ordinal);
            _parameters = new Dictionary<string, PintaFunctionParameter>(StringComparer.Ordinal);
            _functions = new Dictionary<string, PintaFunctionInner>(StringComparer.Ordinal);

            _closures = new Dictionary<string, PintaFunctionClosure>(StringComparer.Ordinal);

            _temporary = new Stack<PintaFunctionTemporaryLocal>();
        }

        public PintaFunctionLocal GetLocal(string name, bool allowFunction = true)
        {
            var result = default(PintaFunctionLocal);
            if (_locals.TryGetValue(name, out result))
                return result;

            if (allowFunction)
            {
                var inner = default(PintaFunctionInner);
                if (_functions.TryGetValue(name, out inner))
                {
                    if (inner.Local == null)
                        inner.Local = DeclareLocal(name);
                    return inner.Local;
                }
            }

            return null;
        }

        public PintaFunctionParameter GetParameter(string name)
        {
            var result = default(PintaFunctionParameter);
            if (_parameters.TryGetValue(name, out result))
                return result;

            return null;
        }

        public PintaFunctionClosure GetClosure(string name)
        {
            var result = default(PintaFunctionClosure);
            if (_closures.TryGetValue(name, out result))
                return result;

            var closureIndex = ClosureIndex - 1;
            var closure = Parent;

            while (closure != null)
            {
                var local = closure.GetLocal(name);
                if (local != null)
                {
                    var item = Function.DeclareClosure(closureIndex, local.Local);
                    result = new PintaFunctionClosure(item);
                    break;
                }

                var parameter = closure.GetParameter(name);
                if (parameter != null)
                {
                    var item = Function.DeclareClosure(closureIndex, parameter.Parameter);
                    result = new PintaFunctionClosure(item);
                    break;
                }

                closure = closure.Parent;
                closureIndex--;
            }

            _closures[name] = result;
            return result;
        }

        public PintaFunctionInner GetLocalFunction(string name)
        {
            var result = default(PintaFunctionInner);
            if (_functions.TryGetValue(name, out result))
                return result;

            return null;
        }

        public PintaFunctionInner GetFunction(string name)
        {
            var result = GetLocalFunction(name);
            if(result != null)
                return result;

            var closure = Parent;
            while (closure != null)
            {
                result = closure.GetLocalFunction(name);
                if (result != null)
                    return result;

                closure = closure.Parent;
            }

            return null;
        }

        public PintaFunctionTemporaryLocal DeclareTemporaryLocal()
        {
            if (_temporary.Count > 0)
                return _temporary.Pop();

            var local = Function.DeclareLocal("temp#" + _temporaryId++);
            return new PintaFunctionTemporaryLocal(_temporary, local);
        }

        public PintaFunctionLocal DeclareLocal(string name)
        {
            var result = default(PintaFunctionLocal);
            if (_locals.TryGetValue(name, out result))
                return result;

            var local = Function.DeclareLocal(name);
            result = new PintaFunctionLocal(name, local);
            _locals.Add(name, result);
            return result;
        }

        public PintaFunctionParameter DeclareParameter(string name)
        {
            var parameter = Function.DeclareParameter(name);
            var result = new PintaFunctionParameter(name, parameter);
            _parameters[name] = result;

            return result;
        }

        public PintaFunctionInner DeclareFunction(string name)
        {
            var functionInner = new PintaFunctionInner(name);
            _functions[name] = functionInner;
            return functionInner;
        }
    }
}
