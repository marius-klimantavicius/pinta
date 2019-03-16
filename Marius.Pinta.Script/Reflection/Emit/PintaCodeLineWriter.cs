using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection.Lines;

namespace Marius.Pinta.Script.Reflection.Emit
{
    public class PintaCodeLineWriter
    {
        public void Write(PintaCodeBinaryWriter writer, IEnumerable<PintaCodeLine> lines)
        {
            foreach (var item in lines)
            {
                Write(writer, item);
            }
        }

        public void Write(PintaCodeBinaryWriter writer, PintaCodeLine item)
        {
            switch (item.Type)
            {
                case PintaCodeLineType.Binary:
                    Write(writer, item.As<PintaCodeLineBinary>());
                    break;
                case PintaCodeLineType.Call:
                    Write(writer, item.As<PintaCodeLineCall>());
                    break;
                case PintaCodeLineType.CallInternal:
                    Write(writer, item.As<PintaCodeLineCallInternal>());
                    break;
                case PintaCodeLineType.Closure:
                    Write(writer, item.As<PintaCodeLineClosure>());
                    break;
                case PintaCodeLineType.Code:
                    Write(writer, item.As<PintaCodeLineCode>());
                    break;
                case PintaCodeLineType.Global:
                    Write(writer, item.As<PintaCodeLineGlobal>());
                    break;
                case PintaCodeLineType.Integer:
                    Write(writer, item.As<PintaCodeLineInteger>());
                    break;
                case PintaCodeLineType.Invoke:
                    Write(writer, item.As<PintaCodeLineInvoke>());
                    break;
                case PintaCodeLineType.Label:
                    Write(writer, item.As<PintaCodeLineLabel>());
                    break;
                case PintaCodeLineType.LabelMark:
                    break;
                case PintaCodeLineType.Local:
                    Write(writer, item.As<PintaCodeLineLocal>());
                    break;
                case PintaCodeLineType.NewFunction:
                    Write(writer, item.As<PintaCodeLineNewFunction>());
                    break;
                case PintaCodeLineType.Parameter:
                    Write(writer, item.As<PintaCodeLineParameter>());
                    break;
                case PintaCodeLineType.String:
                    Write(writer, item.As<PintaCodeLineString>());
                    break;
                case PintaCodeLineType.Match:
                    Write(writer, item.As<PintaCodeLineMatch>());
                    break;
                default:
                    throw new ArgumentException("item");
            }
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineBinary line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.Binary.Id);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineCall line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.Function.Id);
            writer.WriteUleb128(line.ArgumentsCount);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineCallInternal line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.FunctionId);
            writer.WriteUleb128(line.ArgumentsCount);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineClosure line)
        {
            writer.WriteByte((byte)line.Code);

            if (line.Closure.Parameter != null)
            {
                writer.WriteUleb128(2 * line.Closure.ClosureIndex);
                writer.WriteUleb128(line.Closure.Parameter.Id);
            }
            else
            {
                writer.WriteUleb128(2 * line.Closure.ClosureIndex + 1);
                writer.WriteUleb128(line.Closure.Local.Id);
            }
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineCode line)
        {
            writer.WriteByte((byte)line.Code);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineGlobal line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.Global.Id);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineInteger line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteSleb128(line.Value);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineInvoke line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.ArgumentsCount);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineLabel line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteSleb128(line.Offset);

            for (var i = 0; i < line.Padding; i++)
                writer.WriteByte((byte)PintaCode.Nop);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineLocal line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.Local.Id);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineNewFunction line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.Function.Id);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineParameter line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.Parameter.Id);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineString line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteUleb128(line.String.Id);
        }

        private void Write(PintaCodeBinaryWriter writer, PintaCodeLineMatch line)
        {
            writer.WriteByte((byte)line.Code);
            writer.WriteByte((byte)line.Flags);
        }
    }
}
