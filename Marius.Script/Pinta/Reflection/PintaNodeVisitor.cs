using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaNodeVisitor : IPintaNodeVisitor
    {
        public virtual void Visit(PintaCallCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaCallInternalCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaFunctionVariableCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaIntegerCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaLabelCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaParameterCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaProgramVariableCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaSimpleCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaStringCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaBlobCodeLine line)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaProgramBuilder program)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaFunctionBuilder function)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaFunctionParameter functionParameter)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaFunctionVariable functionVariable)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaLabel label)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaProgramString programString)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaProgramBlob programBlob)
        {
            throw new NotImplementedException();
        }

        public virtual void Visit(PintaProgramVariable programVariable)
        {
            throw new NotImplementedException();
        }
    }
}
