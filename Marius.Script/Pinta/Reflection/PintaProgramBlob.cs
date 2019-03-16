using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaProgramBlob : PintaNode<PintaProgramBlob.NodeData>
    {
        public class NodeData
        {
            public uint Index;
            public uint Offset;
        }

        private byte[] _blob;

        public PintaProgramBlobType Type { get; private set; }
        public string Value { get; private set; }

        public PintaProgramBlob(PintaProgramBlobType blobType, string value)
        {
            Type = blobType;
            Value = value;
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }

        public byte[] Blob
        {
            get
            {
                if (_blob == null)
                {
                    switch (Type)
                    {
                        case PintaProgramBlobType.Hex:
                            _blob = ParseHex(Value);
                            break;

                        case PintaProgramBlobType.Base64:
                            _blob = Convert.FromBase64String(Value);
                            break;

                        case PintaProgramBlobType.Ascii:
                            _blob = Encoding.ASCII.GetBytes(Value);
                            break;

                        case PintaProgramBlobType.Utf8:
                            _blob = Encoding.UTF8.GetBytes(Value);
                            break;

                        default:
                            throw new ArgumentException("Invalid blob type");
                    }
                }

                return _blob;
            }
        }

        private static byte[] ParseHex(string hex)
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
