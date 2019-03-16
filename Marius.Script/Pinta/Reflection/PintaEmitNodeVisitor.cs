using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaEmitNodeVisitor : PintaNodeVisitor
    {
        private PintaBinaryWriter _writer;

        public override void Visit(PintaCallCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteUleb128(line.Function.Data.Index);
            _writer.WriteUleb128(line.ArgumentsLength);
        }

        public override void Visit(PintaCallInternalCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteUleb128(line.InternalFunctionToken);
            _writer.WriteUleb128(line.ArgumentsLength);
        }

        public override void Visit(PintaFunctionVariableCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteUleb128(line.Variable.Data.Index);
        }

        public override void Visit(PintaIntegerCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteSleb128(line.Value);
        }

        public override void Visit(PintaLabelCodeLine line)
        {
            if (line.Code == Reflection.PintaCode.Label)
                return;

            var predictedLength = line.Data.Length;

            var offset = line.Label.Data.Offset - line.Data.Offset;
            var actualLength = 1 + PintaBinaryWriter.GetLengthSleb128(offset);

            _writer.WriteByte((byte)line.Code);
            _writer.WriteSleb128(offset);

            for (var i = 0; i < predictedLength - actualLength; i++)
                _writer.WriteByte((byte)PintaCode.Nop);
        }

        public override void Visit(PintaParameterCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteUleb128(line.Parameter.Data.Index);
        }

        public override void Visit(PintaProgramVariableCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteUleb128(line.Variable.Data.Index);
        }

        public override void Visit(PintaSimpleCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
        }

        public override void Visit(PintaStringCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteUleb128(line.Value.Data.Index);
        }

        public override void Visit(PintaBlobCodeLine line)
        {
            _writer.WriteByte((byte)line.Code);
            _writer.WriteUleb128(line.Value.Data.Index);
        }

        public override void Visit(PintaFunctionBuilder function)
        {
            _writer = function.Program.GetWriter();
            function.Data.BodyWriter = _writer;

            foreach (var item in function.Body)
                item.Accept(this);
        }
    }
}
