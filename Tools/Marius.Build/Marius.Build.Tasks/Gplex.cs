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
    public class Gplex: Task
    {
        private class GplexTask: SimpleToolTask
        {
            private Gplex _parent;
            private ITaskItem _inputFile;
            private ITaskItem _outputFile;

            public GplexTask(Gplex parent, ITaskItem inputFile, ITaskItem outputFile)
            {
                _parent = parent;
                _inputFile = inputFile;
                _outputFile = outputFile;
            }

            protected string GplexBinPath
            {
                get { return Path.Combine(ToolsPath, "gplex\\binaries"); }
            }

            protected override string GenerateFullPathToTool()
            {
                return Path.Combine(GplexBinPath, ToolName);
            }

            protected override string ToolName
            {
                get { return "gplex.exe"; }
            }

            protected override string GenerateCommandLineCommands()
            {
                CommandLineBuilder cb = new CommandLineBuilder();

                cb.AppendBoolSwich(_parent.Babel, "/babel");
                cb.AppendBoolSwich(_parent.Classes, "/classes");
                cb.AppendSwitchIfNotNull("/codePage:", _parent.CodePage);
                cb.AppendSwitchIfNotNull("/frame:", _parent.Frame);
                cb.AppendBoolSwich(_parent.Listing, "/listing");
                cb.AppendBoolSwich(_parent.NoCompress, "/noCompress");
                cb.AppendBoolSwich(_parent.NoCompressMap, "/noCompressMap");
                cb.AppendBoolSwich(_parent.NoCompressNext, "/noCompressNext");
                cb.AppendBoolSwich(_parent.NoEmbedBuffers, "/noEmbedBuffers");
                cb.AppendBoolSwich(_parent.NoFiles, "/noFiles");
                cb.AppendBoolSwich(_parent.NoMinimize, "/noMinimize");
                cb.AppendBoolSwich(_parent.NoParser, "/noParser");
                cb.AppendBoolSwich(_parent.NoPersistBuffer, "/noPersistBuffer");
                cb.AppendBoolSwich(_parent.NoInfo, "/noInfo");
                cb.AppendBoolSwich(_parent.Squeeze, "/squeeze");
                cb.AppendBoolSwich(_parent.Stack, "/stack");
                cb.AppendBoolSwich(_parent.Summary, "/summary");
                cb.AppendBoolSwich(_parent.Unicode, "/unicode");
                cb.AppendBoolSwich(_parent.Verbose, "/verbose");

                cb.AppendSwitchIfNotNull("/out:", Path.GetFullPath(_outputFile.ItemSpec));

                if (_parent.UseFullPath)
                    cb.AppendFileNameIfNotNull(Path.GetFullPath(_inputFile.ItemSpec));
                else
                    cb.AppendFileNameIfNotNull(Extensions.MakeRelativePath(Path.GetFullPath(GetWorkingDirectory() + "\\"), Path.GetFullPath(_inputFile.ItemSpec)));

                return cb.ToString();
            }

            public override bool Execute()
            {
                return base.Execute();
            }

            protected override string GetWorkingDirectory()
            {
                return _parent.IntermediateDirectory.ItemSpec;
            }
        }

        public bool Babel { get; set; }
        public bool Classes { get; set; }
        public ITaskItem CodePage { get; set; }
        public ITaskItem Frame { get; set; }
        public bool Listing { get; set; }
        public bool NoCompress { get; set; }
        public bool NoCompressMap { get; set; }
        public bool NoCompressNext { get; set; }
        public bool NoEmbedBuffers { get; set; }
        public bool NoFiles { get; set; }
        public bool NoMinimize { get; set; }
        public bool NoParser { get; set; }
        public bool NoPersistBuffer { get; set; }
        public bool NoInfo { get; set; }
        public bool Squeeze { get; set; }
        public bool Stack { get; set; }
        public bool Summary { get; set; }
        public bool Unicode { get; set; }
        public bool Verbose { get; set; }
        public string OutputBaseDirectory { get; set; }

        public bool UseFullPath { get; set; }

        [Required]
        public string ToolsPath { get; set; }

        [Required]
        public ITaskItem IntermediateDirectory { get; set; }

        [Required]
        public ITaskItem[] InputFiles { get; set; }

        [Output]
        public ITaskItem[] OutputFiles { get; set; }

        public override bool Execute()
        {
            bool success = true;

            List<ITaskItem> output = new List<ITaskItem>();

            foreach (var item in InputFiles)
            {
                string outputPath = Path.ChangeExtension(item.ItemSpec, ".Generated.cs");
                if (OutputBaseDirectory != null)
                    outputPath = Path.Combine(OutputBaseDirectory, outputPath);
                var outputItem = new TaskItem(outputPath);
                var task = new GplexTask(this, item, outputItem)
                {
                    BuildEngine = this.BuildEngine,
                    HostObject = this.HostObject,

                    ToolsPath = this.ToolsPath,
                };

                success = success && task.Execute();

                output.Add(outputItem);
            }

            OutputFiles = output.ToArray();

            return success;
        }
    }
}
