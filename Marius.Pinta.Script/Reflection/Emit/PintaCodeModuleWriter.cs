using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Emit
{
    public class PintaCodeModuleWriter
    {
        private class CodeEntry
        {
            public PintaCodeFunction Function;
            public uint Offset;
            public uint Length;
        }

        public void Write(PintaCodeBinaryWriter writer, PintaCodeModule module)
        {
            writer.WriteUInt(0x50496E74);

            var version = 1;
            var flags = 0;
            if (module.UseCompression)
                flags = 1;

            writer.WriteUInt(unchecked((uint)(version | (flags << 16))));

            for (var i = 0; i < 32; i++)
                writer.WriteByte(0);

            writer.WriteUInt(unchecked((uint)module.Strings.Count));
            writer.WriteUInt(0);
            writer.WriteUInt(unchecked((uint)module.Binary.Count));
            writer.WriteUInt(0);
            writer.WriteUInt(unchecked((uint)module.Globals.Count));
            writer.WriteUInt(0);
            writer.WriteUInt(unchecked((uint)module.Functions.Count));
            writer.WriteUInt(0);
            writer.WriteUInt(module.StartFunction.Id);
            writer.WriteUInt(0);
            writer.WriteUInt(0);

            var dataOffset = writer.Offset;

            var stringSection = WriteStrings(writer, module.Strings);
            var binarySection = 0U;

            if (module.UseCompression)
            {
                WriteCompressedBinary(writer, module.Binary, out binarySection);
            }
            else
            {
                binarySection = WriteBinary(writer, module.Binary);
            }

            var globalsSection = WriteGlobals(writer, module.Globals);
            var codeSection = WriteCode(writer, module);

            writer.Offset = 44;
            writer.WriteUInt(stringSection);

            writer.Offset = 52;
            writer.WriteUInt(binarySection);

            writer.Offset = 60;
            writer.WriteUInt(globalsSection);

            writer.Offset = 68;
            writer.WriteUInt(codeSection);

            writer.Offset = 76;
            writer.WriteUInt(unchecked((uint)writer.Length - dataOffset));
            writer.WriteUInt(dataOffset);
        }

        private void WriteCompressedBinary(PintaCodeBinaryWriter writer, SortedDictionary<byte[], PintaCodeBinary> binaryData, out uint binarySection)
        {
            Align(writer);

            var compressor = new PintaCodeCompressor();
            var binaryPoints = new List<Tuple<uint, uint>>(binaryData.Count);
            var binaryOffsets = new List<uint>();

            var start = writer.Offset;

            foreach (var item in binaryData)
            {
                var current = item.Value;
                var length = (uint)current.Value.Length;
                var offset = (uint)compressor.Compress(current.Value, 0, current.Value.Length);
                binaryPoints.Add(Tuple.Create(length, offset + start));
            }

            writer.Write(compressor.Output.GetBuffer(), 0, (int)compressor.Output.Length);

            Align(writer);

            foreach (var item in binaryPoints)
            {
                binaryOffsets.Add(writer.Offset);
                writer.WriteUleb128(item.Item1);
                writer.WriteUleb128(item.Item2);
            }
        
            Align(writer);

            binarySection = writer.Offset;
            foreach (var item in binaryOffsets)
            {
                writer.WriteUInt(item);
            }
        }

        private void Align(PintaCodeBinaryWriter writer)
        {
            while (writer.Offset % 4 != 0)
                writer.WriteByte(0);
        }

        private uint WriteStrings(PintaCodeBinaryWriter writer, SortedDictionary<string, PintaCodeString> data)
        {
            Align(writer);

            var stringOffsets = new List<uint>(data.Count);
            foreach (var item in data)
            {
                stringOffsets.Add(writer.Offset);

                var current = item.Value;
                writer.WriteUleb128(unchecked((uint)current.Value.Length));
                writer.Write(current.Value);
            }

            Align(writer);

            var sectionOffset = writer.Offset;
            foreach (var item in stringOffsets)
            {
                writer.WriteUInt(item);
            }
            return sectionOffset;
        }

        private uint WriteBinary(PintaCodeBinaryWriter writer, SortedDictionary<byte[], PintaCodeBinary> data)
        {
            Align(writer);

            var binaryOffsets = new List<uint>(data.Count);
            foreach (var item in data)
            {
                binaryOffsets.Add(writer.Offset);

                var current = item.Value;
                writer.WriteUleb128(unchecked((uint)current.Value.Length));
                writer.Write(current.Value);
            }

            Align(writer);

            var sectionOffset = writer.Offset;
            foreach (var item in binaryOffsets)
            {
                writer.WriteUInt(item);
            }
            return sectionOffset;
        }

        private uint WriteGlobals(PintaCodeBinaryWriter writer, SortedDictionary<string, PintaCodeGlobal> data)
        {
            Align(writer);

            var sectionOffset = writer.Offset;
            foreach (var item in data)
            {
                var current = item.Value;
                writer.WriteUInt(current.Name.Id);
            }

            return sectionOffset;
        }

        private uint WriteCode(PintaCodeBinaryWriter writer, PintaCodeModule module)
        {
            var list = module.Functions;
            var codeWriter = new PintaCodeLineWriter();

            var entries = new List<CodeEntry>();
            foreach (var item in list)
            {
                Align(writer);

                var entry = new CodeEntry();
                entry.Function = item;
                entry.Offset = writer.Offset;

                codeWriter.Write(writer, item.Body);

                entry.Length = writer.Offset - entry.Offset;
                entries.Add(entry);
            }

            Align(writer);

            var sectionOffset = writer.Offset;
            foreach (var item in entries)
            {
                var function = item.Function;
                var name = default(PintaCodeString);
                if (function.Name != null && module.Strings.TryGetValue(function.Name, out name))
                    writer.WriteUInt(name.Id);
                else
                    writer.WriteUInt(0xFFFFFFFF);

                writer.WriteUInt(unchecked((uint)function.Parameters.Count));
                writer.WriteUInt(unchecked((uint)function.Locals.Count));

                writer.WriteUInt(item.Length);
                writer.WriteUInt(item.Offset);
            }
            return sectionOffset;
        }
    }
}
