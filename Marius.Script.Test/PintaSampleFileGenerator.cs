using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script;

namespace Marius.Script.Test
{
    public static class PintaSampleFileGenerator
    {
        public static void Sample()
        {
            var filename = "sample.pint";

            using (var stream = typeof(PintaSampleFileGenerator).Assembly.GetManifestResourceStream("Marius.Script.Test.TestFiles.sample.js"))
            using (var reader = new StreamReader(stream))
            {
                SaveScriptTestFile(reader.ReadToEnd(), filename);
            }
        }

        private static void SaveScriptTestFile(string source, string filename)
        {
            var compiler = new PintaCompiler();
            var path = Path.Combine(@"..\..\..\Marius.Pinta.Test.Files\", filename);
            path = Path.GetFullPath(path);
            using (var output = new FileStream(path, FileMode.Create))
            {
                compiler.SetInternalFunctions(new Dictionary<string, uint>()
                {
                    ["outline"] = 0,
                    ["out"] = 0,
                });

                compiler.CompileString(source, output, false);

                output.Flush();
            }
        }
    }
}
