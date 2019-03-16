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
using System.IO;
using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;

namespace Marius.Build.Tasks
{
    public class Gpp: SimpleToolTask
    {
        public string MingwPath { get { return Path.Combine(ToolsPath, "Mingw\\bin"); } }

        [Required]
        public ITaskItem[] InputFiles { get; set; }

        [Required]
        public string WorkingDirectory { get; set; }

        public bool Link { get; set; }

        public bool Nostdlib { get; set; }
        public bool Nostdinc { get; set; }
        public bool NoBuiltin { get; set; }
        public bool NoRtti { get; set; }
        public bool NoExceptions { get; set; }
        public bool NoStartFiles { get; set; }

        public ITaskItem Warnings { get; set; }
        public ITaskItem DebugLevel { get; set; }

        [Output]
        public ITaskItem[] OutputFiles
        {
            get
            {
                return InputFiles.Select(s => Path.GetFileNameWithoutExtension(s.ItemSpec)).Select(s => Path.Combine(WorkingDirectory, s) + ".o").Select(s => new TaskItem(s)).ToArray();
            }
        }

        protected override string GenerateFullPathToTool()
        {
            return Path.Combine(MingwPath, ToolName);
        }

        protected override string ToolName
        {
            get { return "g++.exe"; }
        }

        protected override string GenerateCommandLineCommands()
        {
            CommandLineBuilder cb = new CommandLineBuilder();

            if (Warnings != null && string.Compare(Warnings.ItemSpec, "all", StringComparison.OrdinalIgnoreCase) == 0)
            {
                cb.AppendSwitch("-Wall");
                cb.AppendSwitch("-W");
            }

            if (!Link)
                cb.AppendSwitch("-c");

            if (Nostdlib)
                cb.AppendSwitch("-nostdlib");

            if (Nostdinc)
                cb.AppendSwitch("-nostdinc");

            if (NoBuiltin)
                cb.AppendSwitch("-fno-builtin");

            if (NoRtti)
                cb.AppendSwitch("-fno-rtti");

            if (NoExceptions)
                cb.AppendSwitch("-fno-exceptions");

            if (NoStartFiles)
                cb.AppendSwitch("-nostartfiles");

            if (DebugLevel != null && !string.IsNullOrEmpty(DebugLevel.ItemSpec))
                cb.AppendSwitch("-g" + DebugLevel.ItemSpec);

            cb.AppendFileNamesIfNotNull(InputFiles, " ");

            Log.LogMessage(MessageImportance.High, "g++.exe {0}", cb.ToString());

            return cb.ToString();
        }

        protected override bool ValidateParameters()
        {
            int level;
            if (DebugLevel != null && (!int.TryParse(DebugLevel.ItemSpec, out level) || level < 0 || level > 3))
                Log.LogError("DebugLevel must be a number between 0 and 3");

            return base.ValidateParameters();
        }

        protected override string GetWorkingDirectory()
        {
            return WorkingDirectory;
        }
    }
}
