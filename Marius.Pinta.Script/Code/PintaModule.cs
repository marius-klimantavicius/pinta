using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Optimization;
using Marius.Pinta.Script.Parser;
using Marius.Pinta.Script.Parser.Ast;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaModule
    {
        private class PintaModuleFunction
        {
            public PintaFunctionInner Inner;
            public FunctionDeclaration Declaration;
        }

        private readonly PintaCodeOptimizer _codeOptimizer = new PintaCodeOptimizer();

        private List<PintaFunction> _functions = new List<PintaFunction>();
        private ConditionalWeakTable<FunctionExpression, PintaFunction> _expressionFunctions = new ConditionalWeakTable<FunctionExpression, PintaFunction>();

        public PintaCodeModule Module { get; set; }

        public PintaModule()
        {
            Module = new PintaCodeModule();
        }

        public PintaFunction Compile(Program module)
        {
            var mainFunction = CreateFunction(null, "#main", Enumerable.Empty<Identifier>(), module.VariableDeclarations, module.FunctionDeclarations, module.Body, true);
            _functions.Add(mainFunction);

            CompileFunctions();

            Module.StartFunction = mainFunction.Function;
            return mainFunction;
        }

        private void CompileFunctions()
        {
            var complex = new Queue<PintaFunction>(_functions.Where(s => !s.IsSimple));
            while (complex.Count > 0)
            {
                var item = complex.Dequeue();
                if (item.CallerFunctions == null)
                    continue;

                foreach (var caller in item.CallerFunctions)
                {
                    if (caller.IsSimple)
                    {
                        complex.Enqueue(caller);
                        caller.IsSimple = false;
                    }
                }
            }

            foreach (var item in _functions)
            {
                if (item.InnerFunctions == null)
                    continue;

                foreach (var inner in item.InnerFunctions)
                {
                    if (!inner.Function.IsSimple && inner.Local == null)
                        inner.Local = item.Scope.DeclareLocal(inner.Name);
                }
            }

            foreach (var item in _functions)
            {
                var compiler = new PintaFunctionCompiler(this, item.Function.GetCodeGenerator(), item.Scope);
                compiler.Emit(item.Body);

                if (item.InnerFunctions == null)
                    continue;

                foreach (var inner in item.InnerFunctions)
                {
                    if (inner.Local != null)
                        compiler.EmitFunctionDeclaration(inner.Local, inner.Function);
                }
            }
        }

        public void Save(Stream output, bool emitBigEndian)
        {
            Module.Save(output, emitBigEndian);
        }

        public void SetInternalFunction(string name, uint id)
        {
            Module.SetInternalFunction(name, id);
        }

        public void SetInternalFunctions(Dictionary<string, uint> dictionary)
        {
            Module.SetInternalFunctions(dictionary);
        }

        public string[] GetGlobals()
        {
            return Module.GetGlobals();
        }

        public PintaFunction CreateFunction(PintaScope parent, FunctionExpression expression, bool isStartFunction = false)
        {
            var name = default(string);
            if (expression.Id != null)
                name = expression.Id.Name;

            var body = default(IEnumerable<Statement>);
            if (expression.Body.Type == SyntaxNodes.BlockStatement)
                body = expression.Body.As<BlockStatement>().Body;
            else
                body = new[] { expression.Body };

            var function = CreateFunction(parent, name, expression.Parameters, expression.VariableDeclarations, expression.FunctionDeclarations, body, isStartFunction);
            _functions.Add(function);
            _expressionFunctions.Add(expression, function);
            return function;
        }

        public PintaFunction GetFunction(FunctionExpression expression)
        {
            var function = default(PintaFunction);
            if (!_expressionFunctions.TryGetValue(expression, out function))
                throw new Exception("Internal error");
            return function;
        }

        private PintaFunction CreateFunction(PintaScope parent, FunctionDeclaration declaration, bool isStartFunction = false)
        {
            var name = default(string);
            if (declaration.Id != null)
                name = declaration.Id.Name;

            var body = default(IEnumerable<Statement>);
            if (declaration.Body.Type == SyntaxNodes.BlockStatement)
                body = declaration.Body.As<BlockStatement>().Body;
            else
                body = new[] { declaration.Body };

            var function = CreateFunction(parent, name, declaration.Parameters, declaration.VariableDeclarations, declaration.FunctionDeclarations, body, isStartFunction);
            _functions.Add(function);
            return function;
        }

        private PintaFunction CreateFunction(PintaScope parent, string name, IEnumerable<Identifier> parameters, IEnumerable<VariableDeclaration> variableDeclarations, IEnumerable<FunctionDeclaration> functionDeclarations, IEnumerable<Statement> body, bool isStartFunction = false)
        {
            body = _codeOptimizer.Optimize(body).ToList();

            var function = Module.DefineFunction(name);

            var scope = new PintaScope(function, parent);
            var result = new PintaFunction(function, scope, body);

            foreach (var item in parameters)
                scope.DeclareParameter(item.Name);

            foreach (var item in variableDeclarations)
            {
                foreach (var declaration in item.Declarations)
                    scope.DeclareLocal(declaration.Id.Name);
            }

            var innerFunctionDeclarations = new List<PintaModuleFunction>();
            foreach (var item in functionDeclarations)
            {
                var inner = scope.DeclareFunction(item.Id.Name);
                innerFunctionDeclarations.Add(new PintaModuleFunction() { Inner = inner, Declaration = item });
            }

            var innerFunctions = new List<PintaFunctionInner>();
            foreach (var item in innerFunctionDeclarations)
            {
                var inner = item.Inner;
                inner.Function = CreateFunction(scope, item.Declaration);
                innerFunctions.Add(inner);
            }
            
            var simpleFunctionResolver = new PintaFunctionResolver(this, result);
            simpleFunctionResolver.Resolve(body);

            result.InnerFunctions = innerFunctions;
            result.IsSimple = simpleFunctionResolver.IsSimple;

            return result;
        }
    }
}
