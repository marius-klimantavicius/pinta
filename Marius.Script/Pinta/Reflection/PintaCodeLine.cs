using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public abstract class PintaCodeLine : PintaNode<PintaCodeLine.NodeData>
    {
        public class NodeData
        {
            public int Offset;
            public int Length;

            public int OffsetMin;
            public int OffsetMax;
            public int LengthMin;
            public int LengthMax;
        }

        public PintaCode Code { get; set; }
        
        public PintaCodeLine(PintaCode code)
        {
            Code = code;
        }

        public override abstract string ToString();

        protected string GetCodeString()
        {
            switch (Code)
            {
                case PintaCode.Nop:
                    return "nop";
                case PintaCode.Add:
                    return "add";
                case PintaCode.Subtract:
                    return "subtract";
                case PintaCode.Multiply:
                    return "multiply";
                case PintaCode.Divide:
                    return "divide";
                case PintaCode.Remainder:
                    return "remainder";
                case PintaCode.BitwiseAnd:
                    return "bitwise.and";
                case PintaCode.BitwiseOr:
                    return "bitwise.or";
                case PintaCode.ExclusiveOr:
                    return "exclusive.or";
                case PintaCode.BitwiseExclusiveOr:
                    return "bitwise.exclusive.or";
                case PintaCode.Not:
                    return "not";
                case PintaCode.BitwiseNot:
                    return "bitwise.not";
                case PintaCode.Negate:
                    return "negate";
                case PintaCode.CompareEqual:
                    return "compare.equal";
                case PintaCode.CompareLessThan:
                    return "compare.less.than";
                case PintaCode.CompareMoreThan:
                    return "compare.more.than";
                case PintaCode.CompareNull:
                    return "compare.null";
                case PintaCode.ConvertInteger:
                    return "convert.integer";
                case PintaCode.ConvertDecimal:
                    return "convert.decimal";
                case PintaCode.ConvertString:
                    return "convert.string";
                case PintaCode.NewArray:
                    return "new.array";
                case PintaCode.Concat:
                    return "concat";
                case PintaCode.Substring:
                    return "substring";
                case PintaCode.Jump:
                    return "jump";
                case PintaCode.JumpZero:
                    return "jump.zero";
                case PintaCode.JumpNotZero:
                    return "jump.not.zero";
                case PintaCode.Call:
                    return "call";
                case PintaCode.CallInternal:
                    return "call.internal";
                case PintaCode.Return:
                    return "return";
                case PintaCode.LoadNull:
                    return "load.null";
                case PintaCode.LoadIntegerZero:
                    return "load.integer.zero";
                case PintaCode.LoadDecimalZero:
                    return "load.decimal.zero";
                case PintaCode.LoadIntegerOne:
                    return "load.integer.one";
                case PintaCode.LoadDecimalOne:
                    return "load.decimal.one";
                case PintaCode.LoadInteger:
                    return "load.integer";
                case PintaCode.LoadString:
                    return "load.string";
                case PintaCode.StoreLocal:
                    return "store.local";
                case PintaCode.StoreGlobal:
                    return "store.global";
                case PintaCode.StoreArgument:
                    return "store.argument";
                case PintaCode.StoreItem:
                    return "store.item";
                case PintaCode.LoadLocal:
                    return "load.local";
                case PintaCode.LoadGlobal:
                    return "load.global";
                case PintaCode.LoadArgument:
                    return "load.argument";
                case PintaCode.LoadItem:
                    return "load.item";
                case PintaCode.Duplicate:
                    return "duplicate";
                case PintaCode.Pop:
                    return "pop";
                case PintaCode.Exit:
                    return "exit";
                case PintaCode.GetLength:
                    return "get.length";
                case PintaCode.Error:
                    return "error";
                case PintaCode.Label:
                    return "#";
                default:
                    return Code.ToString();
            }
        }
    }
}
