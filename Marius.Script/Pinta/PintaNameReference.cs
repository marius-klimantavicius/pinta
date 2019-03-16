using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Marius.Script.Pinta.Reflection;
using Marius.Script.Tree;

namespace Marius.Script.Pinta
{
    public class PintaNameReference : PintaTargetReference
    {
        private ScriptNameExpression _expression;

        public PintaNameReference(ScriptNameExpression expression)
        {
            _expression = expression;
        }

        public override void Prepare(PintaGenerateCode generate, bool loadHint)
        {
        }

        public override void Load(PintaGenerateCode generate)
        {
            var name = _expression.Name.Name;
            var local = generate.Function.GetVariable(name);
            if (local != null)
            {
                generate.Function.Emit(PintaCode.LoadLocal, local);
                return;
            }

            var parameter = generate.Function.GetParameter(name);
            if (parameter != null)
            {
                generate.Function.Emit(PintaCode.LoadArgument, parameter);
                return;
            }

            var global = generate.Program.GetGlobal(name);
            generate.Function.Emit(PintaCode.LoadGlobal, global);
        }

        public override void Store(PintaGenerateCode generate)
        {
            var name = _expression.Name.Name;
            var local = generate.Function.GetVariable(name);
            if (local != null)
            {
                generate.Function.Emit(PintaCode.StoreLocal, local);
                return;
            }

            var parameter = generate.Function.GetParameter(name);
            if (parameter != null)
            {
                generate.Function.Emit(PintaCode.StoreArgument, parameter);
                return;
            }

            var global = generate.Program.GetGlobal(name);
            generate.Function.Emit(PintaCode.StoreGlobal, global);
        }

        public override void Dispose()
        {
        }
    }
}
