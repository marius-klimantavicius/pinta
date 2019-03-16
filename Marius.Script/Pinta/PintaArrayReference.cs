using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Marius.Script.Pinta.Reflection;
using Marius.Script.Tree;

namespace Marius.Script.Pinta
{
    public class PintaArrayReference : PintaTargetReference
    {
        private ScriptArrayAccessExpression _expression;

        private PintaTemporaryFunctionVariable _array;
        private PintaTemporaryFunctionVariable _index;

        public PintaArrayReference(ScriptArrayAccessExpression expression)
        {
            _expression = expression;
        }

        public override void Prepare(PintaGenerateCode generate, bool loadHint)
        {
            if (loadHint)
            {
                _array = generate.Function.GetTemporary();
                _index = generate.Function.GetTemporary();

                _expression.Array.Accept(generate);
                generate.Function.Emit(PintaCode.StoreLocal, _array.Variable);

                for (var i = 0; i < _expression.Indexes.Count; i++)
                {
                    if (i != 0)
                        generate.Function.Emit(PintaCode.Pop);

                    var current = _expression.Indexes[i];
                    current.Accept(generate);
                }

                generate.Function.Emit(PintaCode.StoreLocal, _index.Variable);
            }
        }

        public override void Load(PintaGenerateCode generate)
        {
            if (_array == null || _index == null)
                Prepare(generate, true);

            generate.Function.Emit(PintaCode.LoadLocal, _index.Variable);
            generate.Function.Emit(PintaCode.LoadLocal, _array.Variable);
            generate.Function.Emit(PintaCode.LoadItem);
        }

        public override void Store(PintaGenerateCode generate)
        {
            if (_array == null || _index == null)
            {
                for (var i = 0; i < _expression.Indexes.Count; i++)
                {
                    if (i != 0)
                        generate.Function.Emit(PintaCode.Pop);

                    var current = _expression.Indexes[i];
                    current.Accept(generate);
                }
                _expression.Array.Accept(generate);
            }
            else
            {
                generate.Function.Emit(PintaCode.LoadLocal, _index.Variable);
                generate.Function.Emit(PintaCode.LoadLocal, _array.Variable);
            }

            generate.Function.Emit(PintaCode.StoreItem);
        }

        public override void Cleanup(PintaGenerateCode generate)
        {
            if (_index != null)
            {
                generate.Function.Emit(PintaCode.LoadNull);
                generate.Function.Emit(PintaCode.StoreLocal, _index.Variable);
            }

            if (_array != null)
            {
                generate.Function.Emit(PintaCode.LoadNull);
                generate.Function.Emit(PintaCode.StoreLocal, _array.Variable);
            }
        }

        public override void Dispose()
        {
            if (_array != null)
            {
                _array.Dispose();
                _array = null;
            }

            if (_index != null)
            {
                _index.Dispose();
                _index = null;
            }
        }
    }
}
