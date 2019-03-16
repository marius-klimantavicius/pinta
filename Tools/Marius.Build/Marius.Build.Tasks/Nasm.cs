/* 
 Copyright (c) 2010 Marius Klimantavičius

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Build.Utilities;
using System.IO;
using Microsoft.Build.Framework;

namespace Marius.Build.Tasks
{
    public class Nasm: Task
    {
        internal class NasmToolTask : SimpleToolTask
        {
            private string NasmPath { get { return Path.Combine(ToolsPath, "Nasm"); } }

            public ITaskItem Format { get; set; }

            public ITaskItem Output { get; set; }

            public ITaskItem InputFile { get; set; }

            protected override string GenerateFullPathToTool()
            {
                return Path.Combine(NasmPath, ToolName);
            }

            protected override string GenerateCommandLineCommands()
            {
                CommandLineBuilder cb = new CommandLineBuilder();

                cb.AppendSwitchUnquotedIfNotNull("-f ", Format);
                cb.AppendSwitchIfNotNull("-o ", Output);
                cb.AppendSwitch("-Xvc");
                cb.AppendFileNameIfNotNull(InputFile);

                Log.LogMessage(MessageImportance.High, "nasm.exe {0}", cb.ToString());

                return cb.ToString();
            }

            public override bool Execute()
            {
                return base.Execute();
            }

            protected override string ToolName
            {
                get { return "nasm.exe"; }
            }
        }

        public ITaskItem Format { get; set; }
        public ITaskItem Extension { get; set; }
        [Required]
        public ITaskItem[] InputFiles { get; set; }
        [Required]
        public ITaskItem OutputDirectory { get; set; }

        [Output]
        public ITaskItem[] OutputFiles { get; private set; }

        public override bool Execute()
        {
            var output = new List<ITaskItem>();
            bool success = true;

            foreach (var item in InputFiles)
            {
                var task = new NasmToolTask()
                {
                    BuildEngine = this.BuildEngine,
                    HostObject = this.HostObject,
                    
                    Format = this.Format,
                    InputFile = item,
                    Output = CreateOutput(item),
                };

                success = success && task.Execute();

                output.Add(task.Output);
            }

            OutputFiles = output.ToArray();
            return true;
        }

        private ITaskItem CreateOutput(ITaskItem item)
        {
            string name = Path.GetFileNameWithoutExtension(item.ItemSpec);
            return new TaskItem(Path.Combine(OutputDirectory.ItemSpec, name) + "." + Extension);
        }
    }
}
