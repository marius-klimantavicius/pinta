using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Build.Utilities;
using Microsoft.Build.Framework;
using System.IO;
using System.Diagnostics;

namespace Marius.Build.Tasks
{
    public class Jay: Task
    {
        public bool IncludeLines { get; set; }
        public bool DebugInformation { get; set; }
        public bool Verbose { get; set; }
        public string OutputBaseDirectory { get; set; }

        [Required]
        public string ToolsPath { get; set; }

        [Required]
        public ITaskItem IntermediateDirectory { get; set; }

        [Required]
        public ITaskItem[] InputFiles { get; set; }

        [Output]
        public ITaskItem[] OutputFiles { get; private set; }

        internal class JayTask
        {
            private Jay _parent;
            private ITaskItem _inputFile;
            private ITaskItem _outputFile;

            private StreamWriter _outputWriter;

            public string ToolsPath { get; set; }

            public JayTask(Jay parent, ITaskItem inputFile, ITaskItem outputFile)
            {
                _parent = parent;
                _inputFile = inputFile;
                _outputFile = outputFile;
            }

            protected string DefaultSkeletonFile
            {
                get { return Path.Combine(ToolsPath, "jay\\project\\skeleton.cs"); }
            }

            protected string JayBinPath
            {
                get { return Path.Combine(ToolsPath, "jay\\binaries"); }
            }

            protected string ToolName
            {
                get { return "jay.exe"; }
            }


            protected string GenerateFullPathToTool()
            {
                return Path.Combine(JayBinPath, ToolName);
            }

            protected string GenerateCommandLineCommands()
            {
                CommandLineBuilder cb = new CommandLineBuilder();

                cb.AppendBoolSwich(_parent.IncludeLines, "-c");
                cb.AppendBoolSwich(_parent.DebugInformation, "-t");

                if (_parent.Verbose)
                {
                    cb.AppendBoolSwich(_parent.Verbose, "-v");
                    var prefix = _inputFile.GetMetadata("Prefix");
                    cb.AppendSwitchIfNotNull("-b ", prefix);
                }

                if (_inputFile != null && !string.IsNullOrEmpty(_inputFile.ItemSpec))
                {
                    var input = Path.GetFullPath(_inputFile.ItemSpec);
                    cb.AppendFileNameIfNotNull(input);
                }
                return cb.ToString();
            }

            public bool Execute()
            {
                string toolPath = GenerateFullPathToTool();
                string arguments = GenerateCommandLineCommands();

                if (!File.Exists(toolPath))
                {
                    _parent.Log.LogError("Could not find jay.exe (Looking for '{0}')", toolPath);
                    return false;
                }

                Process process = new Process();
                process.StartInfo.RedirectStandardInput = true;
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.RedirectStandardError = true;
                process.StartInfo.Arguments = arguments;
                process.StartInfo.FileName = toolPath;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.WorkingDirectory = Path.GetDirectoryName(_parent.IntermediateDirectory.ItemSpec);

                _parent.Log.LogCommandLine(toolPath + " " + arguments);

                if (!Directory.Exists(process.StartInfo.WorkingDirectory))
                    Directory.CreateDirectory(process.StartInfo.WorkingDirectory);

                process.OutputDataReceived += new DataReceivedEventHandler(process_OutputDataReceived);
                process.ErrorDataReceived += new DataReceivedEventHandler(process_ErrorDataReceived);

                _outputWriter = new StreamWriter(_outputFile.ItemSpec, false);

                process.Start();

                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                // skeleton file
                var skeletonFile = _inputFile.GetMetadata("skeletonFile");
                if (string.IsNullOrEmpty(skeletonFile))
                    skeletonFile = DefaultSkeletonFile;

                using (FileStream skeleton = new FileStream(skeletonFile, FileMode.Open))
                    skeleton.CopyTo(process.StandardInput.BaseStream);

                process.StandardInput.Close();
                process.WaitForExit();

                _outputWriter.Close();

                return process.ExitCode == 0;
            }

            void process_ErrorDataReceived(object sender, DataReceivedEventArgs e)
            {
                if (e.Data == null)
                    return;

                var sindex = e.Data.IndexOf(':');
                if (sindex > 0)
                {
                    if ((sindex + 2) < e.Data.Length && e.Data[sindex + 2] == 'e')
                        _parent.Log.LogError("{0}", e.Data);
                    else
                        _parent.Log.LogWarning("{0}", e.Data);
                }
                else
                {
                    _parent.Log.LogMessageFromText(e.Data, MessageImportance.Normal);
                }
            }

            void process_OutputDataReceived(object sender, DataReceivedEventArgs e)
            {
                _outputWriter.WriteLine(e.Data);
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
                outputPath = Path.GetFullPath(outputPath);
                var outputItem = new TaskItem(outputPath);
                var task = new JayTask(this, item, outputItem)
                {
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
