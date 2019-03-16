using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Marius.Script.Parsing;
using Marius.Script.Tree;

namespace Marius.Script.Pinta
{
    public class PintaCompiler
    {
        private readonly bool _emitBigEndian;
        private Dictionary<string, uint> _internalFunctions = new Dictionary<string, uint>(StringComparer.Ordinal);

        public string[] GlobalVariables { get; private set; }

        public PintaCompiler()
        {
        }

        public PintaCompiler(bool emitBigEndian)
        {
            _emitBigEndian = emitBigEndian;
        }

        public void SetInternalFunction(string name, uint index)
        {
            _internalFunctions[name] = index;
        }

        public void SetInternalFunctions(Dictionary<string, uint> functions)
        {
            _internalFunctions = new Dictionary<string, uint>(functions, StringComparer.Ordinal);
        }

        public void CompileString(string source, Stream output)
        {
            var program = default(ScriptProgram);

            var scanner = new Scanner();
            scanner.SetSource(source, 0);
            var parser = new Parser(scanner);
            if (!parser.Parse())
                throw new ScriptSyntaxException("Failed to parse program");

            program = parser.Program;

            if (program == null)
                throw new ScriptSyntaxException("Failed to parse program");
            var backend = new PintaBackend(_internalFunctions, _emitBigEndian);
            backend.Process(program, output);
        
            GlobalVariables = backend.GetGlobalVariables();
        }

        public void Compile(string sourceFile, string outputFile)
        {
            var program = default(ScriptProgram);
            using (var file = File.OpenRead(sourceFile))
            {
                var scanner = new Scanner(file);
                var parser = new Parser(scanner);
                if (!parser.Parse())
                    throw new ScriptSyntaxException("Failed to parse program");

                program = parser.Program;
            }

            if (program == null)
                throw new ScriptSyntaxException("Failed to parse program");
            var backend = new PintaBackend(_internalFunctions, _emitBigEndian);
            using (var output = new FileStream(outputFile, FileMode.Create))
            {
                backend.Process(program, output);
            }

            GlobalVariables = backend.GetGlobalVariables();
        }
    }
}
