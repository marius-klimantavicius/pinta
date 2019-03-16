using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using Marius.Script.Pinta;

namespace Marius.Pinta.Managed.Sample
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        List<ExecutionUnit> executionUnits = new List<ExecutionUnit>();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_CompileAndExecute_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var val = script.Text;
                if (string.IsNullOrEmpty(val))
                    return;

                var pc = new Marius.Pinta.Script.PintaCompiler();
                pc.SetInternalFunction("out", 0);
                pc.SetInternalFunction("outline", 0);
                var tempCompiledFile = "temp.compiled";

                if (File.Exists(tempCompiledFile))
                    File.Delete(tempCompiledFile);

                using (var output = new FileStream(tempCompiledFile, FileMode.Create))
                    pc.CompileString(val, output, false);

                Execute(tempCompiledFile);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        class ManagedEnvironment : IPintaEnvironment
        {
            public void CloseFile(object handle)
            {
                var stream = (FileStream)handle;
                stream.Dispose();
            }

            public uint GetFileSize(object handle)
            {
                var stream = (FileStream)handle;
                return (uint)stream.Length;
            }

            public object OpenFile(string name)
            {
                return new FileStream(name, FileMode.Open);
            }

            public uint ReadFile(object handle, byte[] buffer)
            {
                var stream = (FileStream)handle;
                return (uint)stream.Read(buffer, 0, buffer.Length);
            }

            public uint WriteFile(object handle, byte[] buffer)
            {
                var stream = (FileStream)handle;
                stream.Write(buffer, 0, buffer.Length);
                return (uint)buffer.Length;
            }
        }

        private void Execute(string filename)
        {
            var pe = new PintaEngine(filename);

            //var parameters = parameterDataGrid.ItemsSource as List<Parameter>;
            //if (parameters != null)
            //{
            //    foreach (var p in parameters)
            //    {
            //        if (p.Name == null)
            //            continue;

            //        if (string.IsNullOrEmpty(p.Value))
            //            pe.SetGlobal(p.Name, null);
            //        else
            //            pe.SetGlobal(p.Name, p.Value);
            //    }
            //}

            execResultTxt.Text = "Executing...";

            var data = pe.Execute();
            var result = Encoding.Unicode.GetString(data);

            execResultTxt.Text = result;

            var error = pe.GetGlobal("ERROR");
            if (error != null)
                execResultTxt.Text += "\r\n*-* ERROR = " + error + " *-*";
        }

        private string ToHex(byte[] bytes)
        {
            var sb = new StringBuilder(bytes.Length * 2);
            foreach (var b in bytes)
            {
                sb.Append(b.ToString("X2"));
            }
            return sb.ToString();
        }

        //private void jsFileComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        //{
        //    try
        //    {
        //        var val = jsFileComboBox.SelectedValue as string;
        //        if (String.IsNullOrEmpty(val))
        //            return;

        //        var pc = new Marius.Pinta.Script.PintaCompiler();
        //        pc.SetInternalFunction("out", 0);
        //        pc.SetInternalFunction("outline", 0);
        //        var tempCompiledFile = "temp.compiled";

        //        if (File.Exists(tempCompiledFile))
        //            File.Delete(tempCompiledFile);

        //        using (var output = new FileStream(tempCompiledFile, FileMode.Create))
        //            pc.Compile(val, output, isBigEndian.IsChecked ?? false);

        //        var globals = pc.Globals;
        //        parameterDataGrid.ItemsSource = globals.Select(s => new Parameter() { Name = s }).ToList();
        //    }
        //    catch (Exception ex)
        //    {
        //        MessageBox.Show(ex.ToString());
        //    }
        //}
    }

    class Parameter
    {
        public string Name { get; set; }
        public string Value { get; set; }
    }

    class ExecutionUnit
    {
        public static ExecutionUnit LoadFromFile(string filename)
        {
            var eu = new ExecutionUnit();
            eu.OutputFilename = filename + "-out.txt";

            foreach (var line in File.ReadAllLines(filename))
            {
                if (String.IsNullOrEmpty(line) || line.StartsWith("#"))
                    continue;

                var items = line.Split(new[] { '=' });
                if (items.Length != 2)
                    throw new Exception("Invalid line: " + line);

                eu.variables[items[0].Trim()] = items[1].Trim();
            }

            return eu;
        }

        Dictionary<string, string> variables = new Dictionary<string, string>();
        public string OutputFilename { get; private set; }

        async public Task Execute(string sourceFile)
        {
            await Task.Run(() =>
                {
                    var pintaFile = @"compiled.pint";
                    var compiler = new PintaCompiler();
                    compiler.SetInternalFunction("outline", 0);

                    compiler.Compile(sourceFile, pintaFile);

                    using (var engine = new PintaEngine(pintaFile))
                    {
                        foreach (var v in variables)
                        {
                            engine.SetGlobal(v.Key, v.Value);
                        }

                        engine.Execute();
                    }
                });
        }
    }
}
