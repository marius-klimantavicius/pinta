using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection.Lines;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeGenerator
    {
        private PintaCodeModule _module;

        private uint _labelId;

        private PintaCodeGeneratorMode _mode;

        private List<PintaCodeLine> _prologue = new List<PintaCodeLine>();
        private List<PintaCodeLine> _body = new List<PintaCodeLine>();
        private List<PintaCodeLine> _lines;

        public PintaCodeGeneratorMode Mode
        {
            get
            {
                return _mode;
            }
            set
            {
                _mode = value;
                if (_mode == PintaCodeGeneratorMode.Default)
                    _lines = _body;
                else
                    _lines = _prologue;
            }
        }

        public IEnumerable<PintaCodeLine> Body { get { return _prologue.Concat(_body); } }

        public PintaCodeGenerator(PintaCodeModule module)
        {
            _module = module;

            Mode = PintaCodeGeneratorMode.Default;
        }

        public PintaCodeLocal DeclareLocal(string name)
        {
            return new PintaCodeLocal(name);
        }

        public PintaCodeParameter DeclareParameter(string name)
        {
            return new PintaCodeParameter(name);
        }

        public PintaCodeLabel DefineLabel()
        {
            return new PintaCodeLabel(_labelId++);
        }

        public void MarkLabel(PintaCodeLabel label)
        {
            if (label == null)
                throw new ArgumentNullException("label");

            if (label.IsMarked)
                throw new InvalidOperationException("Label is already marked");

            label.IsMarked = true;
            _lines.Add(new PintaCodeLineLabelMark(label));
        }

        public void Emit(PintaCode code)
        {
            switch (code)
            {
                case PintaCode.Nop:
                case PintaCode.Add:
                case PintaCode.Subtract:
                case PintaCode.Multiply:
                case PintaCode.Divide:
                case PintaCode.Remainder:
                case PintaCode.BitwiseAnd:
                case PintaCode.BitwiseOr:
                case PintaCode.ExclusiveOr:
                case PintaCode.BitwiseExclusiveOr:
                case PintaCode.Not:
                case PintaCode.BitwiseNot:
                case PintaCode.Negate:
                case PintaCode.CompareEqual:
                case PintaCode.CompareLessThan:
                case PintaCode.CompareMoreThan:
                case PintaCode.CompareNull:
                case PintaCode.ConvertInteger:
                case PintaCode.ConvertDecimal:
                case PintaCode.ConvertString:
                case PintaCode.NewArray:
                case PintaCode.Concat:
                case PintaCode.Substring:
                case PintaCode.Return:
                case PintaCode.LoadNull:
                case PintaCode.LoadIntegerZero:
                case PintaCode.LoadDecimalZero:
                case PintaCode.LoadIntegerOne:
                case PintaCode.LoadDecimalOne:
                case PintaCode.StoreItem:
                case PintaCode.LoadItem:
                case PintaCode.Duplicate:
                case PintaCode.Pop:
                case PintaCode.Exit:
                case PintaCode.GetLength:
                case PintaCode.Error:
                case PintaCode.LoadMember:
                case PintaCode.LoadThis:
                case PintaCode.StoreMember:
                case PintaCode.NewObject:
                    _lines.Add(new PintaCodeLineCode(code));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeLabel label)
        {
            if (label == null)
                throw new ArgumentNullException("label");

            switch (code)
            {
                case PintaCode.Jump:
                case PintaCode.JumpNotZero:
                case PintaCode.JumpZero:
                    _lines.Add(new PintaCodeLineLabel(code, label));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeGlobal global)
        {
            if (global == null)
                throw new ArgumentNullException("variable");

            switch (code)
            {
                case PintaCode.LoadGlobal:
                case PintaCode.StoreGlobal:
                    _lines.Add(new PintaCodeLineGlobal(code, global));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeClosure closure)
        {
            if (closure == null)
                throw new ArgumentNullException("closure");

            switch (code)
            {
                case PintaCode.LoadClosure:
                case PintaCode.StoreClosure:
                    _lines.Add(new PintaCodeLineClosure(code, closure));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeParameter parameter)
        {
            if (parameter == null)
                throw new ArgumentNullException("parameter");

            switch (code)
            {
                case PintaCode.LoadArgument:
                case PintaCode.StoreArgument:
                    _lines.Add(new PintaCodeLineParameter(code, parameter));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeLocal local)
        {
            if (local == null)
                throw new ArgumentNullException("local");

            switch (code)
            {
                case PintaCode.LoadLocal:
                case PintaCode.StoreLocal:
                    _lines.Add(new PintaCodeLineLocal(code, local));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, int value)
        {
            if (code == PintaCode.LoadInteger)
            {
                if (value == 0)
                {
                    Emit(PintaCode.LoadIntegerZero);
                    return;
                }

                if (value == 1)
                {
                    Emit(PintaCode.LoadIntegerOne);
                    return;
                }
            }

            switch (code)
            {
                case PintaCode.LoadInteger:
                    _lines.Add(new PintaCodeLineInteger(code, value));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, uint count)
        {
            switch (code)
            {
                case PintaCode.New:
                case PintaCode.Invoke:
                case PintaCode.InvokeMember:
                case PintaCode.DuplicateMultiple:
                    _lines.Add(new PintaCodeLineInvoke(code, count));
                    return;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, string value)
        {
            if (value == null)
                throw new ArgumentNullException("value");

            switch (code)
            {
                case PintaCode.LoadString:

                    var stringValue = _module.GetString(value);
                    _lines.Add(new PintaCodeLineString(code, stringValue));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeFunction function)
        {
            if (function == null)
                throw new ArgumentNullException("function");

            switch (code)
            {
                case PintaCode.NewFunction:
                    _lines.Add(new PintaCodeLineNewFunction(function));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeBinaryType type, string value)
        {
            if (value == null)
                throw new ArgumentNullException("value");

            switch (code)
            {
                case PintaCode.LoadBinary:
                    var binary = _module.GetBinary(type, value);
                    _lines.Add(new PintaCodeLineBinary(code, binary));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, uint id, uint count)
        {
            switch (code)
            {
                case PintaCode.CallInternal:
                    _lines.Add(new PintaCodeLineCallInternal(id, count));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeFunction function, uint count)
        {
            switch (code)
            {
                case PintaCode.Call:
                    _lines.Add(new PintaCodeLineCall(function, count));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaCodeMatchFlags flags)
        {
            switch (code)
            {
                case PintaCode.Match:
                    _lines.Add(new PintaCodeLineMatch(flags));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }
    }
}
