using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public interface IPintaNodeVisitor
    {
        void Visit(PintaCallCodeLine line);
        void Visit(PintaCallInternalCodeLine line);
        void Visit(PintaFunctionVariableCodeLine line);
        void Visit(PintaIntegerCodeLine line);
        void Visit(PintaLabelCodeLine line);
        void Visit(PintaParameterCodeLine line);
        void Visit(PintaProgramVariableCodeLine line);
        void Visit(PintaSimpleCodeLine line);
        void Visit(PintaStringCodeLine line);
        void Visit(PintaBlobCodeLine line);

        void Visit(PintaProgramBuilder program);
        void Visit(PintaFunctionBuilder function);

        void Visit(PintaFunctionParameter functionParameter);

        void Visit(PintaFunctionVariable functionVariable);

        void Visit(PintaLabel label);

        void Visit(PintaProgramString programString);
        void Visit(PintaProgramBlob programBlob);

        void Visit(PintaProgramVariable programVariable);
    }
}
