using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection.Emit;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeModule
    {
        private List<PintaCodeFunction> _functions;

        private SortedDictionary<string, PintaCodeString> _strings;
        private Dictionary<string, PintaCodeBinary> _binary;
        private SortedDictionary<byte[], PintaCodeBinary> _binaryValue;
        private SortedDictionary<string, PintaCodeGlobal> _globals;
        private Dictionary<string, PintaCodeInternalFunction> _internalFunctions;

        public SortedDictionary<string, PintaCodeString> Strings { get { return _strings; } }
        public SortedDictionary<byte[], PintaCodeBinary> Binary { get { return _binaryValue; } }
        public SortedDictionary<string, PintaCodeGlobal> Globals { get { return _globals; } }

        public List<PintaCodeFunction> Functions { get { return _functions; } }

        public PintaCodeFunction StartFunction { get; set; }

        public bool UseCompression { get; set; }

        public PintaCodeModule()
        {
            _functions = new List<PintaCodeFunction>();
            _globals = new SortedDictionary<string, PintaCodeGlobal>(StringComparer.Ordinal);
            _internalFunctions = new Dictionary<string, PintaCodeInternalFunction>(StringComparer.Ordinal);

            _strings = new SortedDictionary<string, PintaCodeString>(StringComparer.Ordinal);

            _binary = new Dictionary<string, PintaCodeBinary>(StringComparer.Ordinal);
            _binaryValue = new SortedDictionary<byte[], PintaCodeBinary>(PintaCodeBinaryComparer.Instance);

            UseCompression = true;
        }

        public PintaCodeFunction DefineFunction(string name)
        {
            var function = new PintaCodeFunction(this, name);
            _functions.Add(function);
            return function;
        }

        public PintaCodeString GetString(string value)
        {
            var result = default(PintaCodeString);
            if (_strings.TryGetValue(value, out result))
                return result;

            result = new PintaCodeString(value);
            _strings.Add(value, result);
            return result;
        }

        public PintaCodeBinary GetBinary(PintaCodeBinaryType type, string value)
        {
            var result = default(PintaCodeBinary);
            if (_binary.TryGetValue(value, out result))
            {
                if (result.Type == type)
                    return result;
            }

            var data = default(byte[]);
            switch (type)
            {
                case PintaCodeBinaryType.Hex:
                    data = PintaCodeBinary.ParseHex(value);
                    break;
                case PintaCodeBinaryType.Base64:
                    data = Convert.FromBase64String(value);
                    break;
                case PintaCodeBinaryType.Ascii:
                    data = Encoding.ASCII.GetBytes(value);
                    break;
                case PintaCodeBinaryType.Utf8:
                    data = Encoding.UTF8.GetBytes(value);
                    break;
                default:
                    throw new ArgumentException("type");
            }

            if (_binaryValue.TryGetValue(data, out result))
                return result;

            result = new PintaCodeBinary(type, data);
            _binary[value] = result;
            _binaryValue.Add(data, result);
            return result;
        }

        public PintaCodeGlobal GetGlobal(string name)
        {
            var result = default(PintaCodeGlobal);
            if (_globals.TryGetValue(name, out result))
                return result;

            var nameString = GetString(name);
            result = new PintaCodeGlobal(nameString);
            _globals.Add(name, result);
            return result;
        }

        public void SetInternalFunction(string name, uint id)
        {
            if (_internalFunctions == null)
                _internalFunctions = new Dictionary<string, PintaCodeInternalFunction>(StringComparer.Ordinal);
            _internalFunctions[name] = new PintaCodeInternalFunction(name, id);
        }

        public void SetInternalFunctions(Dictionary<string, uint> dictionary)
        {
            if (_internalFunctions == null)
                _internalFunctions = new Dictionary<string, PintaCodeInternalFunction>(StringComparer.Ordinal);
            foreach (var item in dictionary)
                _internalFunctions[item.Key] = new PintaCodeInternalFunction(item.Key, item.Value);
        }

        public PintaCodeInternalFunction GetInternalFunction(string name)
        {
            var result = default(PintaCodeInternalFunction);
            if (_internalFunctions.TryGetValue(name, out result))
                return result;

            return null;
        }

        public void Save(Stream output, bool emitBigEndian)
        {
            var idResolver = new PintaCodeIdResolver();
            idResolver.Resolve(_strings);
            idResolver.Resolve(_binaryValue);
            idResolver.Resolve(_globals);
            idResolver.Resolve(_functions);

            var offsetResolver = new PintaCodeLabelOffsetResolver();
            foreach (var item in _functions)
            {
                offsetResolver.Resolve(item.Body);
            }

            var binaryWriter = new PintaCodeBinaryWriter(emitBigEndian);
            var writer = new PintaCodeModuleWriter();

            writer.Write(binaryWriter, this);
            binaryWriter.CopyTo(output);
        }

        public string[] GetGlobals()
        {
            return _globals.Select(s => s.Key).ToArray();
        }
    }
}
