using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaBlobCodeLine : PintaCodeLine
    {
        public PintaProgramBlob Value { get; private set; }

        public PintaBlobCodeLine(PintaCode code, PintaProgramBlob value)
            : base(code)
        {
            Value = value;
        }

        public override string ToString()
        {
            switch (Value.Type)
            {
                case PintaProgramBlobType.Hex:
                    return GetCodeString() + " hex('" + Value.Value + "')";
                case PintaProgramBlobType.Base64:
                    return GetCodeString() + " base64('" + Value.Value + "')";
                case PintaProgramBlobType.Ascii:
                    return GetCodeString() + " ascii('" + Value.Value + "')";
                case PintaProgramBlobType.Utf8:
                    return GetCodeString() + " utf8('" + Value.Value + "')";
                default:
                    return GetCodeString() + " '" + Value.Value + "'";
            }
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
