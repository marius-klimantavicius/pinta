using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaLengthMinMaxNodeVisitor : PintaNodeVisitor
    {
        public override void Visit(PintaCallCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthUleb128(line.Function.Data.Index) + PintaBinaryWriter.GetLengthUleb128(line.ArgumentsLength);

            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaCallInternalCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthUleb128(line.InternalFunctionToken) + PintaBinaryWriter.GetLengthUleb128(line.ArgumentsLength);

            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaFunctionVariableCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthUleb128(line.Variable.Data.Index);

            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaIntegerCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthSleb128(line.Value);

            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaLabelCodeLine line)
        {
            if (line.Code == PintaCode.Label)
            {
                line.Data.LengthMin = 0;
                line.Data.LengthMax = 0;
                return;
            }

            line.Data.LengthMin = 2;
            line.Data.LengthMax = 6;
        }

        public override void Visit(PintaParameterCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthUleb128(line.Parameter.Data.Index);

            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaProgramVariableCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthUleb128(line.Variable.Data.Index);

            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaSimpleCodeLine line)
        {
            line.Data.LengthMin = 1;
            line.Data.LengthMax = 1;
        }

        public override void Visit(PintaStringCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthUleb128(line.Value.Data.Index);
            
            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaBlobCodeLine line)
        {
            var lengthMin = 1 + PintaBinaryWriter.GetLengthUleb128(line.Value.Data.Index);

            line.Data.LengthMin = lengthMin;
            line.Data.LengthMax = lengthMin;
        }

        public override void Visit(PintaFunctionBuilder function)
        {
            foreach (var item in function.Body)
                item.Accept(this);
        }
    }
}
