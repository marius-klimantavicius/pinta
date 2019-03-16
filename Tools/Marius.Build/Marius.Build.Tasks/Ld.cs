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
using Microsoft.Build.Utilities;
using Microsoft.Build.Framework;

namespace Marius.Build.Tasks
{
    public class Ld: SimpleToolTask
    {
        private string MingwPath { get { return Path.Combine(ToolsPath, "Mingw\\bin"); } }

        [Required]
        public ITaskItem[] InputFiles { get; set; }

        public string Entry { get; set; }
        public ITaskItem OutputFile { get; set; }
        public ITaskItem MapFile { get; set; }

        public bool Nostdlib { get; set; }
        public bool StripDebugSections { get; set; }

        public string FileAlignment { get; set; }
        public string SectionAlignment { get; set; }
        public string ImageBase { get; set; }
        public string OutputFormat { get; set; }

        protected override string GenerateFullPathToTool()
        {
            return Path.Combine(MingwPath, ToolName);
        }

        protected override string ToolName
        {
            get { return "ld.exe"; }
        }

        protected override string GenerateCommandLineCommands()
        {
            CommandLineBuilder cb = new CommandLineBuilder();

            cb.AppendSwitchUnquotedIfNotNull("-e ", Entry);
            cb.AppendSwitchIfNotNull("-o ", OutputFile);
            cb.AppendSwitchIfNotNull("-Map ", MapFile);

            if (Nostdlib)
                cb.AppendSwitch("-nostdlib");

            if (StripDebugSections)
            {
                cb.AppendSwitch("-s");
                cb.AppendSwitch("-S");
            }

            cb.AppendSwitchUnquotedIfNotNull("--file-alignment ", FileAlignment);
            cb.AppendSwitchUnquotedIfNotNull("--section-alignment ", SectionAlignment);
            cb.AppendSwitchUnquotedIfNotNull("--image-base ", ImageBase);
            cb.AppendSwitchUnquotedIfNotNull("--oformat ", OutputFormat);

            cb.AppendFileNamesIfNotNull(InputFiles, " ");

            Log.LogMessage(MessageImportance.High, "ld.exe {0}", cb.ToString());

            return cb.ToString();
        }
    }
}
