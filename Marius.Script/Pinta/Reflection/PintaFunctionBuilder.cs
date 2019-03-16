using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaFunctionBuilder : PintaNode<PintaFunctionBuilder.NodeData>
    {
        public class NodeData
        {
            public uint Index;

            public PintaBinaryWriter BodyWriter;
            public uint CodeOffset;
            public uint CodeLength;
        }

        private List<PintaFunctionVariable> _variables = new List<PintaFunctionVariable>();
        private LinkedList<PintaTemporaryFunctionVariable> _temporaryVariables = new LinkedList<PintaTemporaryFunctionVariable>();
        private Dictionary<string, PintaFunctionVariable> _namedVariables = new Dictionary<string, PintaFunctionVariable>(StringComparer.Ordinal);

        private List<PintaFunctionParameter> _parameters = new List<PintaFunctionParameter>();
        private Dictionary<string, PintaFunctionParameter> _namedParameters = new Dictionary<string, PintaFunctionParameter>(StringComparer.Ordinal);

        private List<PintaCodeLine> _body = new List<PintaCodeLine>();

        public PintaProgramBuilder Program { get; private set; }

        public string Name { get; set; }
        public List<PintaFunctionParameter> Parameters { get { return _parameters; } }
        public List<PintaFunctionVariable> Variables { get { return _variables; } }

        public List<PintaCodeLine> Body { get { return _body; } }

        public PintaFunctionBuilder(PintaProgramBuilder program)
        {
            if (program == null)
                throw new ArgumentNullException("program");

            Program = program;
        }

        public PintaFunctionVariable DefineVariable(string debugName)
        {
            var variable = new PintaFunctionVariable(this);
            _variables.Add(variable);
            variable.DebugName = debugName;
            return variable;
        }

        public PintaFunctionVariable GetVariable(string name, bool create = false)
        {
            var variable = default(PintaFunctionVariable);
            if (_namedVariables.TryGetValue(name, out variable))
                return variable;

            if (create)
            {
                variable = DefineVariable(name);
                _namedVariables[name] = variable;
            }

            return variable;
        }

        public PintaFunctionVariable CreateVariable(string name)
        {
            var variable = default(PintaFunctionVariable);
            if (_namedVariables.TryGetValue(name, out variable))
                return null;

            variable = DefineVariable(name);
            _namedVariables[name] = variable;
            return variable;
        }

        public PintaFunctionParameter DefineParameter(string debugName)
        {
            var parameter = new PintaFunctionParameter(this);
            _parameters.Add(parameter);
            parameter.DebugName = debugName;
            return parameter;
        }

        public PintaFunctionParameter GetParameter(string name, bool create = false)
        {
            var parameter = default(PintaFunctionParameter);
            if (_namedParameters.TryGetValue(name, out parameter))
                return parameter;

            if (create)
            {
                parameter = DefineParameter(name);
                _namedParameters[name] = parameter;
            }

            return parameter;
        }

        public PintaFunctionParameter CreateParameter(string name)
        {
            var parameter = default(PintaFunctionParameter);
            if (_namedParameters.TryGetValue(name, out parameter))
                return null;

            parameter = DefineParameter(name);
            _namedParameters[name] = parameter;
            return parameter;
        }

        public PintaLabel DefineLabel()
        {
            return new PintaLabel(this);
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
                    _body.Add(new PintaSimpleCodeLine(code));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, string arg)
        {
            if (arg == null)
                throw new ArgumentNullException(arg);

            var stringValue = Program.RegisterString(arg);

            switch (code)
            {
                case PintaCode.LoadGlobal:
                case PintaCode.LoadString:
                case PintaCode.StoreGlobal:
                    _body.Add(new PintaStringCodeLine(code, stringValue));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaProgramBlobType type, string arg)
        {
            if (arg == null)
                throw new ArgumentNullException(arg);

            var blobValue = Program.RegisterBlob(type, arg);
            switch (code)
            {
                case PintaCode.LoadBlob:
                    _body.Add(new PintaBlobCodeLine(code, blobValue));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, int arg)
        {
            switch (code)
            {
                case PintaCode.LoadInteger:
                    _body.Add(new PintaIntegerCodeLine(code, arg));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaFunctionVariable variable)
        {
            if (variable == null)
                throw new ArgumentNullException("variable");

            switch (code)
            {
                case PintaCode.LoadLocal:
                case PintaCode.StoreLocal:
                    _body.Add(new PintaFunctionVariableCodeLine(code, variable));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaProgramVariable variable)
        {
            if (variable == null)
                throw new ArgumentNullException("variable");

            switch (code)
            {
                case PintaCode.LoadGlobal:
                case PintaCode.StoreGlobal:
                    _body.Add(new PintaProgramVariableCodeLine(code, variable));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaFunctionParameter parameter)
        {
            if (parameter == null)
                throw new ArgumentNullException("parameter");

            switch (code)
            {
                case PintaCode.LoadArgument:
                case PintaCode.StoreArgument:
                    _body.Add(new PintaParameterCodeLine(code, parameter));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void Emit(PintaCode code, PintaLabel label)
        {
            if (label == null)
                throw new ArgumentNullException("label");

            switch (code)
            {
                case PintaCode.Jump:
                case PintaCode.JumpNotZero:
                case PintaCode.JumpZero:
                    _body.Add(new PintaLabelCodeLine(code, label));
                    break;
                default:
                    throw new ArgumentException("code");
            }
        }

        public void EmitCall(PintaFunctionBuilder function, uint argumentsLength)
        {
            if (function == null)
                throw new ArgumentNullException("function");

            _body.Add(new PintaCallCodeLine(function, argumentsLength));
        }

        public void EmitCallInternal(uint internalFunctionToken, uint argumentsLength)
        {
            _body.Add(new PintaCallInternalCodeLine(internalFunctionToken, argumentsLength));
        }

        public void Emit(PintaLabel label)
        {
            if (label == null)
                throw new ArgumentNullException("label");

            if (label.IsEmitted)
                throw new ArgumentException("label");

            label.IsEmitted = true;
            _body.Add(new PintaLabelCodeLine(PintaCode.Label, label));
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }

        public PintaTemporaryFunctionVariable GetTemporary()
        {
            var result = default(PintaTemporaryFunctionVariable);
            if (_temporaryVariables.Count == 0)
            {
                result = new PintaTemporaryFunctionVariable(this);
            }
            else
            {
                result = _temporaryVariables.First.Value;
                _temporaryVariables.RemoveFirst();
            }

            return result;
        }

        public void Free(PintaTemporaryFunctionVariable temp)
        {
            _temporaryVariables.AddLast(temp);
        }
    }
}
