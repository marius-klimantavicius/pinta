using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineBinary : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Binary; } }

        public PintaCodeBinary Binary { get; private set; }

        public PintaCodeLineBinary(PintaCode code, PintaCodeBinary binary)
            : base(code)
        {
            Binary = binary;
        }

        public override string ToString()
        {
            switch (Binary.Type)
            {
                case PintaCodeBinaryType.Hex:
                    return GetCodeString() + " hex('" + ToHex(Binary.Value) + "')";
                case PintaCodeBinaryType.Base64:
                    return GetCodeString() + " base64('" + ToBase64(Binary.Value) + "')";
                case PintaCodeBinaryType.Ascii:
                    return GetCodeString() + " ascii('" + ToAscii(Binary.Value) + "')";
                case PintaCodeBinaryType.Utf8:
                    return GetCodeString() + " utf8('" + ToUtf8(Binary.Value) + "')";
                default:
                    return GetCodeString() + " '" + Binary.Value + "'";
            }
        }

        private string ToUtf8(byte[] value)
        {
            return Encoding.UTF8.GetString(value);
        }

        private string ToAscii(byte[] value)
        {
            return Encoding.ASCII.GetString(value);
        }

        private string ToBase64(byte[] value)
        {
            return Convert.ToBase64String(value);
        }

        private string ToHex(byte[] value)
        {
            var sb = new StringBuilder(value.Length * 2);
            foreach (var b in value)
            {
                sb.Append(b.ToString("X2"));
            }
            return sb.ToString();
        }
    }
}
