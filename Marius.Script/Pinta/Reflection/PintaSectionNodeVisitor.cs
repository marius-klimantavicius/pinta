using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaSectionNodeVisitor : PintaNodeVisitor
    {
        private PintaProgramBuilder _program;

        private PintaBinaryWriter _stringsWriter;
        private PintaBinaryWriter _blobsWriter;
        private PintaBinaryWriter _globalsWriter;

        public override void Visit(PintaProgramBuilder program)
        {
            _program = program;

            _stringsWriter = program.GetWriter();
            _blobsWriter = program.GetWriter();
            _globalsWriter = program.GetWriter();

            program.Data.StringsSection = _stringsWriter;
            program.Data.BlobsSection = _blobsWriter;
            program.Data.GlobalsSection = _globalsWriter;

            foreach (var item in program.Strings)
                item.Value.Accept(this);

            foreach (var item in program.Blobs)
                item.Value.Accept(this);

            foreach (var item in program.Globals)
                item.Value.Accept(this);

            foreach (var item in program.Functions)
                item.Accept(this);
        }

        public override void Visit(PintaFunctionBuilder function)
        {
            var minMaxLength = new PintaLengthMinMaxNodeVisitor();
            var length = new PintaLengthVisitor();

            minMaxLength.Visit(function);

            var offsetMin = 0;
            var offsetMax = 0;
            for (var i = 0; i < function.Body.Count; i++)
            {
                var current = function.Body[i];
                current.Data.OffsetMin = offsetMin;
                current.Data.OffsetMax = offsetMax;

                offsetMin += current.Data.LengthMin;
                offsetMax += current.Data.LengthMax;

                var labelCode = current as PintaLabelCodeLine;
                if (labelCode != null)
                {
                    if (labelCode.Code == PintaCode.Label)
                    {
                        labelCode.Label.Data.OffsetMin = current.Data.OffsetMin;
                        labelCode.Label.Data.OffsetMax = current.Data.OffsetMax;
                    }
                }
            }

            length.Visit(function);

            var offset = 0;
            for (var i = 0; i < function.Body.Count; i++)
            {
                var current = function.Body[i];
                current.Data.Offset = offset;

                offset += current.Data.Length;

                var labelCode = function.Body[i] as PintaLabelCodeLine;
                if (labelCode != null)
                {
                    if (labelCode.Code == PintaCode.Label)
                        labelCode.Label.Data.Offset = current.Data.Offset;
                }
            }

            var emit = new PintaEmitNodeVisitor();
            emit.Visit(function);
        }

        public override void Visit(PintaProgramString stringValue)
        {
            stringValue.Data.Offset = _stringsWriter.Offset;

            _stringsWriter.WriteUleb128(unchecked((uint)stringValue.Value.Length));
            _stringsWriter.Write(stringValue.Value);
        }

        public override void Visit(PintaProgramBlob blobValue)
        {
            blobValue.Data.Offset = _blobsWriter.Offset;

            _blobsWriter.WriteUleb128(unchecked((uint)blobValue.Blob.Length));
            _blobsWriter.Write(blobValue.Blob);
        }

        public override void Visit(PintaProgramVariable programVariable)
        {
            _globalsWriter.WriteUInt(programVariable.Data.String.Data.Index);
        }
    }
}
