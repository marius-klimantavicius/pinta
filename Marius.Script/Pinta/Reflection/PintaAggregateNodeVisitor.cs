using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaAggregateNodeVisitor : PintaNodeVisitor
    {
        public PintaBinaryWriter Writer { get; private set; }

        public override void Visit(PintaProgramBuilder program)
        {
            Writer = program.GetWriter();

            Writer.WriteUInt(0x50496E74);
            Writer.WriteUInt(0);

            for (var i = 0; i < 32; i++)
                Writer.WriteByte(0);

            Writer.WriteUInt(unchecked((uint)program.Strings.Count));
            Writer.WriteUInt(0);
            Writer.WriteUInt(unchecked((uint)program.Blobs.Count));
            Writer.WriteUInt(0);
            Writer.WriteUInt(unchecked((uint)program.Globals.Count));
            Writer.WriteUInt(0);
            Writer.WriteUInt(unchecked((uint)program.Functions.Count));
            Writer.WriteUInt(0);
            Writer.WriteUInt(program.StartFunction.Data.Index);
            Writer.WriteUInt(0);
            Writer.WriteUInt(0);

            var dataOffset = Writer.Offset;

            while (Writer.Offset % 4 != 0)
                Writer.WriteByte(0);

            var stringsDataOffset = Writer.Offset;
            Writer.Write(program.Data.StringsSection);

            while (Writer.Offset % 4 != 0)
                Writer.WriteByte(0);

            var stringsOffset = Writer.Offset;
            foreach (var item in program.Strings)
            {
                var current = item.Value;
                Writer.WriteUInt(current.Data.Offset + stringsDataOffset);
            }

            while (Writer.Offset % 4 != 0)
                Writer.WriteByte(0);

            var blobsDataOffset = Writer.Offset;
            Writer.Write(program.Data.BlobsSection);

            while (Writer.Offset % 4 != 0)
                Writer.WriteByte(0);

            var blobsOffset = Writer.Offset;
            foreach (var item in program.Blobs)
            {
                var current = item.Value;
                Writer.WriteUInt(current.Data.Offset + blobsDataOffset);
            }

            while (Writer.Offset % 4 != 0)
                Writer.WriteByte(0);

            var globalsOffset = Writer.Offset;
            Writer.Write(program.Data.GlobalsSection);

            foreach (var item in program.Functions)
            {
                while (Writer.Offset % 4 != 0)
                    Writer.WriteByte(0);

                item.Data.CodeOffset = Writer.Offset;
                item.Data.CodeLength = item.Data.BodyWriter.Length;
                Writer.Write(item.Data.BodyWriter);
            }

            while (Writer.Offset % 4 != 0)
                Writer.WriteByte(0);

            var functionsOffset = Writer.Offset;
            foreach (var item in program.Functions)
            {
                var name = default(PintaProgramString);
                if (!string.IsNullOrEmpty(item.Name) && program.Strings.TryGetValue(item.Name, out name))
                    Writer.WriteUInt(name.Data.Index);
                else
                    Writer.WriteUInt(0xFFFFFFFF);

                Writer.WriteUInt(unchecked((uint)item.Parameters.Count));
                Writer.WriteUInt(unchecked((uint)item.Variables.Count));

                Writer.WriteUInt(item.Data.CodeLength);
                Writer.WriteUInt(item.Data.CodeOffset);
            }

            Writer.Offset = 44;
            Writer.WriteUInt(stringsOffset);

            Writer.Offset = 52;
            Writer.WriteUInt(blobsOffset);

            Writer.Offset = 60;
            Writer.WriteUInt(globalsOffset);

            Writer.Offset = 68;
            Writer.WriteUInt(functionsOffset);

            Writer.Offset = 76;
            Writer.WriteUInt(unchecked((uint)Writer.Length - dataOffset));
            Writer.WriteUInt(dataOffset);
        }
    }
}
