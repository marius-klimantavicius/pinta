using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaRegisterStringsNodeVisitor : PintaNodeVisitor
    {
        private PintaProgramBuilder _program;

        public override void Visit(PintaCallCodeLine line)
        {
        }

        public override void Visit(PintaCallInternalCodeLine line)
        {
        }

        public override void Visit(PintaFunctionVariableCodeLine line)
        {
        }

        public override void Visit(PintaIntegerCodeLine line)
        {
        }

        public override void Visit(PintaLabelCodeLine line)
        {
        }

        public override void Visit(PintaParameterCodeLine line)
        {
        }

        public override void Visit(PintaProgramVariableCodeLine line)
        {
        }

        public override void Visit(PintaSimpleCodeLine line)
        {
        }

        public override void Visit(PintaStringCodeLine line)
        {
        }

        public override void Visit(PintaBlobCodeLine line)
        {
        }

        public override void Visit(PintaProgramBuilder program)
        {
            _program = program;

            foreach (var item in program.Globals)
                item.Value.Accept(this);

            foreach (var item in program.Functions)
                item.Accept(this);
        }

        public override void Visit(PintaFunctionBuilder function)
        {
            foreach (var item in function.Parameters)
                item.Accept(this);

            foreach (var item in function.Variables)
                item.Accept(this);

            foreach (var item in function.Body)
                item.Accept(this);
        }

        public override void Visit(PintaFunctionParameter functionParameter)
        {
        }

        public override void Visit(PintaFunctionVariable functionVariable)
        {
        }

        public override void Visit(PintaProgramVariable programVariable)
        {
            programVariable.Data.String = _program.RegisterString(programVariable.Name);
        }
    }
}
