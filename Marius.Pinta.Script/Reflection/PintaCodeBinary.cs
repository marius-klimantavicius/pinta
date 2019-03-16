using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeBinary
    {
        public uint Id { get; set; }

        public PintaCodeBinaryType Type { get; private set; }
        public byte[] Value { get; private set; }

        public PintaCodeBinary(PintaCodeBinaryType type, byte[] value)
        {
            Type = type;
            Value = value;
        }

        public static byte[] ParseHex(string hex)
        {
            if (hex.Length % 2 == 1)
                throw new Exception("The binary key cannot have an odd number of digits");

            var arr = new byte[hex.Length >> 1];
            for (var i = 0; i < arr.Length; ++i)
                arr[i] = (byte)((GetHexVal(hex[i << 1]) << 4) + (GetHexVal(hex[(i << 1) + 1])));
            return arr;
        }

        private static int GetHexVal(char hex)
        {
            var val = (int)hex;
            return val - (val < 58 ? 48 : 55);
        }
    }
}
