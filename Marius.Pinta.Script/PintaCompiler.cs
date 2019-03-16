using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Parser;
using Marius.Pinta.Script.Parser.Ast;
using Marius.Pinta.Script.Code;

namespace Marius.Pinta.Script
{
    public class PintaCompiler
    {
        private PintaModule _module;

        public string[] Globals { get; private set; }

        public PintaCompiler()
        {
            _module = new PintaModule();
        }

        public void Compile(string file, Stream output, bool emitBigEndian = false)
        {
            var source = File.ReadAllText(file);
            CompileString(source, output, emitBigEndian);
        }

        public void CompileString(string source, Stream output, bool emitBigEndian = false)
        {
            var parser = new JavaScriptParser(false, true);
            var module = parser.Parse(source);

            _module.Compile(module);
            _module.Save(output, emitBigEndian);

            Globals = _module.GetGlobals();
        }

        public void SetInternalFunction(string name, uint id)
        {
            _module.SetInternalFunction(name, id);
        }

        public void SetInternalFunctions(Dictionary<string, uint> dictionary)
        {
            _module.SetInternalFunctions(dictionary);
        }
    }
}
