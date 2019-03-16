using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaLengthVisitor : PintaNodeVisitor
    {
        public override void Visit(PintaCallCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaCallInternalCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaFunctionVariableCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaIntegerCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaLabelCodeLine line)
        {
            if (line.Code == PintaCode.Label)
            {
                line.Data.Length = 0;
                return;
            }

            var offset = 0;
            if (line.Label.Data.OffsetMin <= line.Data.OffsetMin)
                offset = line.Label.Data.OffsetMin - line.Data.OffsetMax;
            else
                offset = line.Label.Data.OffsetMax - line.Data.OffsetMin;
            line.Data.Length = 1 + PintaBinaryWriter.GetLengthSleb128(offset);
        }

        public override void Visit(PintaParameterCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaProgramVariableCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaSimpleCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaStringCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaBlobCodeLine line)
        {
            line.Data.Length = line.Data.LengthMin;
        }

        public override void Visit(PintaFunctionBuilder function)
        {
            foreach (var item in function.Body)
                item.Accept(this);
        }
    }
}
