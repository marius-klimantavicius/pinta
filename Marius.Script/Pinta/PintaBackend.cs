using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Marius.Script.Pinta.Reflection;
using Marius.Script.Tree;

namespace Marius.Script.Pinta
{
    public class PintaBackend
    {
        private readonly bool _emitBigEndian;
        private Dictionary<string, uint> _internalFunctions;

        private string[] _globalVariables;

        public PintaBackend(Dictionary<string, uint> internalFunctions, bool emitBigEndian)
        {
            _internalFunctions = internalFunctions;
            _emitBigEndian = emitBigEndian;
        }

        public void Process(ScriptProgram program, Stream output)
        {
            var builder = new PintaProgramBuilder(_emitBigEndian);
            var collectFunctionsAndVariables = new PintaCollectFunctionsAndVariables(builder);
            program.Accept(collectFunctionsAndVariables);

            var generateCode = new PintaGenerateCode(builder, _internalFunctions);
            program.Accept(generateCode);

            builder.Save(output);
            _globalVariables = builder.Globals.Select(s => s.Key).ToArray();
        }

        public string[] GetGlobalVariables()
        {
            if (_globalVariables == null)
                throw new InvalidOperationException("Call Process before calling this function");
            return _globalVariables;
        }
    }
}
