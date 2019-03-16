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
    public class Gppg: Task
    {
        public bool Babel { get; set; }
        public bool Conflicts { get; set; }
        public bool Defines { get; set; }
        public bool Gplex { get; set; }
        public bool Listing { get; set; }
        public bool NoLines { get; set; }
        public bool Report { get; set; }
        public bool Verbose { get; set; }
        public bool NoInfo { get; set; }
        public bool NoFilename { get; private set; }
        public string OutputBaseDirectory { get; set; }

        public bool UseFullPath { get; set; }

        [Required]
        public string ToolsPath { get; set; }

        [Required]
        public ITaskItem IntermediateDirectory { get; set; }

        [Required]
        public ITaskItem[] InputFiles { get; set; }

        [Output]
        public ITaskItem[] OutputFiles { get; private set; }

        internal class GppgTask: SimpleToolTask
        {
            private Gppg _parent;
            private ITaskItem _inputFile;
            private ITaskItem _outputFile;

            public GppgTask(Gppg parent, ITaskItem inputFile, ITaskItem outputFile)
            {
                _parent = parent;
                _inputFile = inputFile;
                _outputFile = outputFile;
            }

            protected string GppgBinPath
            {
                get { return Path.Combine(ToolsPath, "gppg\\binaries"); }
            }

            protected override string ToolName
            {
                get { return "gppg.exe"; }
            }


            protected override string GenerateFullPathToTool()
            {
                return Path.Combine(GppgBinPath, ToolName);
            }

            protected override string GenerateCommandLineCommands()
            {
                CommandLineBuilder cb = new CommandLineBuilder();

                cb.AppendSwitch("/noThrow");
                cb.AppendBoolSwich(_parent.Babel, "/babel");
                cb.AppendBoolSwich(_parent.Conflicts, "/conflicts");
                cb.AppendBoolSwich(_parent.Defines, "/defines");
                cb.AppendBoolSwich(_parent.Gplex, "/gplex");
                cb.AppendBoolSwich(_parent.Listing, "/listing");
                cb.AppendBoolSwich(_parent.NoLines, "/no-lines");
                cb.AppendBoolSwich(_parent.Report, "/report");
                cb.AppendBoolSwich(_parent.Verbose, "/verbose");
                cb.AppendBoolSwich(_parent.NoInfo, "/no-info");
                cb.AppendBoolSwich(_parent.NoFilename, "/no-filename");
                cb.AppendSwitchIfNotNull("/out:", Path.GetFullPath(_outputFile.ItemSpec));

                if (_parent.UseFullPath)
                    cb.AppendFileNameIfNotNull(Path.GetFullPath(_inputFile.ItemSpec));
                else
                    cb.AppendFileNameIfNotNull(Extensions.MakeRelativePath(Path.GetFullPath(GetWorkingDirectory() + "\\"), Path.GetFullPath(_inputFile.ItemSpec)));

                cb.AppendSwitchIfNotNull("/line-filename:", Extensions.MakeRelativePath(Path.GetFullPath(_outputFile.ItemSpec), Path.GetFullPath(_inputFile.ItemSpec)));

                Log.LogMessage("{0}", cb.ToString());

                return cb.ToString();
            }

            protected override string GetWorkingDirectory()
            {
                return _parent.IntermediateDirectory.ItemSpec;
            }

            public override bool Execute()
            {
                return base.Execute();
            }
        }

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
                var task = new GppgTask(this, item, outputItem)
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

    public static class Extensions
    {
        public static void AppendBoolSwich(this CommandLineBuilder cb, bool condition, string @switch)
        {
            if (condition)
                cb.AppendSwitch(@switch);
        }

        /// <summary>
        /// Creates a relative path from one file or folder to another.
        /// </summary>
        /// <param name="fromPath">Contains the directory that defines the start of the relative path.</param>
        /// <param name="toPath">Contains the path that defines the endpoint of the relative path.</param>
        /// <param name="dontEscape">Boolean indicating whether to add uri safe escapes to the relative path</param>
        /// <returns>The relative path from the start directory to the end path.</returns>
        /// <exception cref="ArgumentNullException"></exception>
        public static string MakeRelativePath(string fromPath, string toPath)
        {
            if (string.IsNullOrEmpty(fromPath)) throw new ArgumentNullException("fromPath");
            if (string.IsNullOrEmpty(toPath)) throw new ArgumentNullException("toPath");

            Uri fromUri = new Uri(fromPath);
            Uri toUri = new Uri(toPath);

            Uri relativeUri = fromUri.MakeRelativeUri(toUri);

            return relativeUri.ToString();

        }

        public static void CopyTo(this Stream input, Stream output)
        {
            byte[] buffer = new byte[4096];
            while (true)
            {
                int read = input.Read(buffer, 0, buffer.Length);
                if (read <= 0)
                    return;
                output.Write(buffer, 0, read);
            }
        }
    }
}
