using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Code;
using Marius.Pinta.Script.Parser.Ast;

namespace Marius.Pinta.Script.Code
{
    public sealed class PintaFunctionResolver : PintaCodeWalker
    {
        private readonly PintaFunction _function;
        private readonly PintaModule _module;

        public bool IsSimple { get; private set; }

        public PintaFunctionResolver(PintaModule module, PintaFunction function)
        {
            _function = function;
            _module = module;

            IsSimple = true;
        }

        public void Resolve(IEnumerable<Statement> body)
        {
            Walk(body);
        }

        public override void WalkFunctionExpression(FunctionExpression expression, bool discard = false)
        {
            if (!discard)
            {
                IsSimple = false;
                _module.CreateFunction(_function.Scope, expression);
            }
        }

        public override void WalkCallExpression(CallExpression expression, bool discard = false)
        {
            var calleeProcessed = false;
            if (expression.Callee.Type == SyntaxNodes.Identifier)
            {
                var callee = expression.Callee.As<Identifier>();
                var name = callee.Name;

                var function = _function.Scope.GetFunction(name);
                if (function != null)
                {
                    function.AddCaller(_function);
                    calleeProcessed = true;
                }
            }

            if (!calleeProcessed)
                Walk(expression.Callee);

            foreach (var item in expression.Arguments)
                Walk(item);
        }

        public override void WalkIdentifier(Identifier identifier, bool discard = false)
        {
            var local = _function.Scope.GetLocal(identifier.Name);
            if (local != null)
                return;

            var parameter = _function.Scope.GetParameter(identifier.Name);
            if (parameter != null)
                return;

            var closure = _function.Scope.GetClosure(identifier.Name);
            if (closure != null && !discard)
                IsSimple = false;
        }
    }
}
