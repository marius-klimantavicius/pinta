using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Marius.Script.Pinta.Reflection;
using Marius.Script.Tree;

namespace Marius.Script.Pinta
{
    public class PintaCollectFunctionsAndVariables : ScriptVisitor
    {
        private PintaProgramBuilder _builder;
        private PintaFunctionBuilder _function;
        private PintaFunctionBuilder _startFunction;

        public PintaCollectFunctionsAndVariables(PintaProgramBuilder builder)
        {
            _builder = builder;
        }

        public override void Visit(ScriptProgram program)
        {
            _startFunction = _builder.DefineFunction("$$main");
            _builder.StartFunction = _startFunction;

            Accept(program.SourceElements);

            _startFunction = null;
        }

        public override void Visit(ScriptFunctionDeclaration functionDeclaration)
        {
            var function = functionDeclaration.Function;

            if (function.Name == null)
                throw new ScriptSyntaxException("Function must have a name");

            _function = _builder.CreateFunction(function.Name.Name);
            if (_function == null)
                throw new ScriptSyntaxException(string.Format("Function with the same name '{0}' already exists", function.Name.Name));

            if (function.ParameterList != null)
            {
                for (var i = 0; i < function.ParameterList.Count; i++)
                {
                    var current = function.ParameterList[i];
                    var name = current.Name;

                    if (_function.CreateParameter(name) == null)
                        throw new ScriptSyntaxException(string.Format("Function parameter with the same name '{0}' already exists", name));
                }
            }

            Accept(function.Body);

            _function = null;
        }

        public override void Visit(ScriptLocalVariable variable)
        {
            var function = _function ?? _startFunction;

            if (function.GetParameter(variable.Name.Name) != null)
                throw new ScriptSyntaxException(string.Format("Function parameter with the same name '{0}' already exists", variable.Name.Name));

            function.GetVariable(variable.Name.Name, true);
        }

        public override void Visit(ScriptFunctionExpression expression)
        {
            throw new ScriptSyntaxException("Function expressions are not supported");
        }
    }
}
