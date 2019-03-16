using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection.Lines;

namespace Marius.Pinta.Script.Reflection.Emit
{
    public class PintaCodeLabelOffsetResolver
    {
        private class CodeInfo
        {
            public PintaCodeLine Line { get; set; }

            public int Offset { get; set; }
            public int Length { get; set; }

            public int MinimumOffset { get; set; }
            public int MaximumOffset { get; set; }
        }

        public void Resolve(IEnumerable<PintaCodeLine> lines)
        {
            var offsetMin = 0;
            var offsetMax = 0;

            var lengthMin = 0;
            var lengthMax = 0;

            var info = lines.Select(s => new CodeInfo() { Line = s }).ToList();
            foreach (var item in info)
            {
                var current = item.Line;
                GetLength(current, out lengthMin, out lengthMax);

                item.Length = lengthMin;
                Debug.Assert(current.Type == PintaCodeLineType.Label || lengthMin == lengthMax);

                item.MinimumOffset = offsetMin;
                item.MaximumOffset = offsetMax;

                offsetMin += lengthMin;
                offsetMax += lengthMax;

                if (current.Type == PintaCodeLineType.LabelMark)
                {
                    var labelCode = current.As<PintaCodeLineLabelMark>();
                    var label = labelCode.Label;

                    label.MinimumOffset = offsetMin;
                    label.MaximumOffset = offsetMax;
                }
            }

            foreach (var item in info)
            {
                var current = item.Line;
                var length = 0;
                if (current.Type == PintaCodeLineType.Label)
                {
                    GetLength(current.As<PintaCodeLineLabel>(), item.MinimumOffset, item.MaximumOffset, out length);
                    item.Length = length;
                }
            }

            var offset = 0;
            foreach (var item in info)
            {
                var current = item.Line;
                if (current.Type == PintaCodeLineType.LabelMark)
                {
                    var labelCode = current.As<PintaCodeLineLabelMark>();
                    var label = labelCode.Label;

                    label.ActualOffset = offset;
                }

                item.Offset = offset;
                offset += item.Length;
            }

            foreach (var item in info)
            {
                var current = item.Line;
                if (current.Type == PintaCodeLineType.Label)
                {
                    var labelCode = current.As<PintaCodeLineLabel>();
                    var predictedLength = item.Length;

                    offset = labelCode.Label.ActualOffset - item.Offset;
                    var actualLength = 1 + PintaCodeBinaryWriter.GetLengthSleb128(offset);

                    labelCode.Offset = offset;
                    labelCode.Padding = predictedLength - actualLength;
                }
            }
        }

        private void GetLength(PintaCodeLine item, out int minimumLength, out int maximumLength)
        {
            switch (item.Type)
            {
                case PintaCodeLineType.Binary:
                    GetLength(item.As<PintaCodeLineBinary>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Call:
                    GetLength(item.As<PintaCodeLineCall>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.CallInternal:
                    GetLength(item.As<PintaCodeLineCallInternal>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Closure:
                    GetLength(item.As<PintaCodeLineClosure>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Code:
                    GetLength(item.As<PintaCodeLineCode>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Global:
                    GetLength(item.As<PintaCodeLineGlobal>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Integer:
                    GetLength(item.As<PintaCodeLineInteger>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Invoke:
                    GetLength(item.As<PintaCodeLineInvoke>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Label:
                    GetLength(item.As<PintaCodeLineLabel>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.LabelMark:
                    GetLength(item.As<PintaCodeLineLabelMark>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Local:
                    GetLength(item.As<PintaCodeLineLocal>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.NewFunction:
                    GetLength(item.As<PintaCodeLineNewFunction>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Parameter:
                    GetLength(item.As<PintaCodeLineParameter>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.String:
                    GetLength(item.As<PintaCodeLineString>(), out minimumLength, out maximumLength);
                    break;
                case PintaCodeLineType.Match:
                    GetLength(item.As<PintaCodeLineMatch>(), out minimumLength, out maximumLength);
                    break;
                default:
                    throw new ArgumentException("item");
            }
        }

        private void GetLength(PintaCodeLineCall line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.Function.Id) + PintaCodeBinaryWriter.GetLengthUleb128(line.ArgumentsCount);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineCallInternal line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.FunctionId) + PintaCodeBinaryWriter.GetLengthUleb128(line.ArgumentsCount);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineLocal line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.Local.Id);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineInteger line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthSleb128(line.Value);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineLabel line, out int minimumLength, out int maximumLength)
        {
            minimumLength = 2;
            maximumLength = 6;
        }

        private void GetLength(PintaCodeLineLabel line, int minimumOffset, int maximumOffset, out int length)
        {
            var offset = 0;
            if (line.Label.MinimumOffset <= minimumOffset)
                offset = line.Label.MinimumOffset - maximumOffset;
            else
                offset = line.Label.MaximumOffset - minimumOffset;
            length = 1 + PintaCodeBinaryWriter.GetLengthSleb128(offset);
        }

        private void GetLength(PintaCodeLineLabelMark line, out int minimumLength, out int maximumLength)
        {
            minimumLength = 0;
            maximumLength = 0;
        }

        private void GetLength(PintaCodeLineParameter line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.Parameter.Id);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineGlobal line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.Global.Id);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineCode line, out int minimumLength, out int maximumLength)
        {
            minimumLength = 1;
            maximumLength = 1;
        }

        private void GetLength(PintaCodeLineString line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.String.Id);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineBinary line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.Binary.Id);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineInvoke line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.ArgumentsCount);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineNewFunction line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(line.Function.Id);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineClosure line, out int minimumLength, out int maximumLength)
        {
            var lengthMin = 0;

            if (line.Closure.Parameter != null)
                lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(2 * line.Closure.ClosureIndex) + PintaCodeBinaryWriter.GetLengthUleb128(line.Closure.Parameter.Id);
            else
                lengthMin = 1 + PintaCodeBinaryWriter.GetLengthUleb128(2 * line.Closure.ClosureIndex + 1) + PintaCodeBinaryWriter.GetLengthUleb128(line.Closure.Local.Id);

            minimumLength = lengthMin;
            maximumLength = lengthMin;
        }

        private void GetLength(PintaCodeLineMatch line, out int minimumLength, out int maximumLength)
        {
            minimumLength = 2;
            maximumLength = 2;
        }
    }
}
