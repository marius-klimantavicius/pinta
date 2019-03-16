using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaProgramBuilder : PintaNode<PintaProgramBuilder.NodeData>
    {

        public class NodeData
        {
            public PintaBinaryWriter StringsSection;
            public PintaBinaryWriter BlobsSection;
            public PintaBinaryWriter GlobalsSection;
        }

        private readonly bool _emitBigEndian;
        private List<PintaFunctionBuilder> _functions = new List<PintaFunctionBuilder>();
        private Dictionary<string, PintaFunctionBuilder> _namedFunctions = new Dictionary<string, PintaFunctionBuilder>(StringComparer.Ordinal);

        private SortedDictionary<string, PintaProgramString> _strings = new SortedDictionary<string, PintaProgramString>(StringComparer.Ordinal);
        private SortedDictionary<string, PintaProgramBlob> _blobs = new SortedDictionary<string, PintaProgramBlob>(StringComparer.OrdinalIgnoreCase);
        private SortedDictionary<string, PintaProgramVariable> _globals = new SortedDictionary<string, PintaProgramVariable>(StringComparer.Ordinal);

        public PintaFunctionBuilder StartFunction { get; set; }

        public List<PintaFunctionBuilder> Functions { get { return _functions; } }
        public SortedDictionary<string, PintaProgramString> Strings { get { return _strings; } }
        public SortedDictionary<string, PintaProgramBlob> Blobs { get { return _blobs; } }
        public SortedDictionary<string, PintaProgramVariable> Globals { get { return _globals; } }

        public Dictionary<string, PintaFunctionBuilder> NamedFunctions { get { return _namedFunctions; } }

        public PintaProgramBuilder(bool emitBigEndian)
        {
            _emitBigEndian = emitBigEndian;
        }

        public PintaProgramString RegisterString(string value)
        {
            if (!_strings.ContainsKey(value))
                _strings.Add(value, new PintaProgramString(value));

            return _strings[value];
        }

        public PintaProgramBlob RegisterBlob(PintaProgramBlobType type, string value)
        {
            if (!_blobs.ContainsKey(value))
                _blobs.Add(value, new PintaProgramBlob(type, value));

            return _blobs[value];
        }

        public PintaProgramVariable GetGlobal(string name)
        {
            RegisterString(name);

            if (!_globals.ContainsKey(name))
                _globals.Add(name, new PintaProgramVariable(this, name));

            return _globals[name];
        }

        public PintaFunctionBuilder DefineFunction(string name)
        {
            var function = new PintaFunctionBuilder(this);
            function.Name = name;
            _functions.Add(function);
            return function;
        }

        public PintaFunctionBuilder GetFunction(string name, bool create = false)
        {
            var function = default(PintaFunctionBuilder);
            if (_namedFunctions.TryGetValue(name, out function))
                return function;

            if (create)
            {
                function = DefineFunction(name);
                _namedFunctions[name] = function;
            }

            return function;
        }

        public PintaBinaryWriter GetWriter()
        {
            return new PintaBinaryWriter(_emitBigEndian);
        }

        public void Save(Stream output)
        {
            var registerString = new PintaRegisterStringsNodeVisitor();
            registerString.Visit(this);

            var assignIndexes = new PintaIndexNodeVisitor();
            assignIndexes.Visit(this);

            var write = new PintaSectionNodeVisitor();
            write.Visit(this);

            var aggregate = new PintaAggregateNodeVisitor();
            aggregate.Visit(this);

            var data = aggregate.Writer.Data;
            output.Write(data, 0, data.Length);
        }

        public void Save(string outputFilename)
        {
            using (var output = new FileStream(outputFilename, FileMode.Create))
            {
                Save(output);
            }
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }

        public PintaFunctionBuilder CreateFunction(string name)
        {
            var existing = default(PintaFunctionBuilder);
            if (_namedFunctions.TryGetValue(name, out existing))
                return null;

            existing = DefineFunction(name);
            _namedFunctions[name] = existing;
            return existing;
        }
    }
}
