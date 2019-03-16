using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public class ScriptVisitor : IScriptVisitor
    {
        public virtual void Visit(ScriptArrayAccessExpression expression)
        {
            Accept(expression.Array);
            Accept(expression.Indexes);
        }

        public virtual void Visit(ScriptArrayExpression expression)
        {
            Accept(expression.Elements);
        }

        public virtual void Visit(ScriptArrayGapElement element)
        {
        }

        public virtual void Visit(ScriptArrayExpressionElement element)
        {
            Accept(element.Element);
        }

        public virtual void Visit(ScriptAssignmentExpression expression)
        {
            Accept(expression.Left);
            Accept(expression.Right);
        }

        public virtual void Visit(ScriptBinaryExpression expression)
        {
            Accept(expression.Left);
            Accept(expression.Right);
        }

        public virtual void Visit(ScriptConditionalExpression expression)
        {
            Accept(expression.Condition);
            Accept(expression.TrueBody);
            Accept(expression.FalseBody);
        }

        public virtual void Visit(ScriptFunctionExpression expression)
        {
            Accept(expression.Name);
            Accept(expression.ParameterList);
            Accept(expression.Body);
        }

        public virtual void Visit(ScriptInvokeExpression expression)
        {
            Accept(expression.Function);
            Accept(expression.Arguments);
        }

        public virtual void Visit(ScriptListExpression expression)
        {
            Accept(expression.Expressions);
        }

        public virtual void Visit(ScriptLiteralExpression expression)
        {
        }

        public virtual void Visit(ScriptMemberAccessExpression expression)
        {
            Accept(expression.Object);
            Accept(expression.Member);
        }

        public virtual void Visit(ScriptNameExpression expression)
        {
            Accept(expression.Name);
        }

        public virtual void Visit(ScriptNewExpression expression)
        {
            Accept(expression.TypeExpression);
            Accept(expression.Arguments);
        }

        public virtual void Visit(ScriptObjectExpression expression)
        {
            Accept(expression.Properties);
        }

        public virtual void Visit(ScriptUnaryExpression expression)
        {
            Accept(expression.Operand);
        }

        public virtual void Visit(ScriptBlockStatement statement)
        {
            Accept(statement.Statements);
        }

        public virtual void Visit(ScriptBreakStatement statement)
        {
        }

        public virtual void Visit(ScriptContinueStatement statement)
        {
        }

        public virtual void Visit(ScriptDebuggerStatement statement)
        {
        }

        public virtual void Visit(ScriptDoStatement statement)
        {
            Accept(statement.Condition);
            Accept(statement.Body);
        }

        public virtual void Visit(ScriptEmptyStatement statement)
        {
        }

        public virtual void Visit(ScriptExpressionStatement statement)
        {
            Accept(statement.Expression);
        }

        public virtual void Visit(ScriptForinStatement statement)
        {
            Accept(statement.Target);
            Accept(statement.Object);
            Accept(statement.Body);
        }

        public virtual void Visit(ScriptForStatement statement)
        {
            Accept(statement.Init);
            Accept(statement.Condition);
            Accept(statement.Step);
            Accept(statement.Body);
        }

        public virtual void Visit(ScriptForVarTarget target)
        {
            Accept(target.Variables);
        }

        public virtual void Visit(ScriptForExpressionTarget target)
        {
            Accept(target.Expression);
        }

        public virtual void Visit(ScriptIfStatement statement)
        {
            Accept(statement.Condition);
            Accept(statement.TrueBody);
            Accept(statement.FalseBody);
        }

        public virtual void Visit(ScriptLabelStatement statement)
        {
            Accept(statement.Label);
            Accept(statement.Statement);
        }

        public virtual void Visit(ScriptReturnStatement statement)
        {
            Accept(statement.Value);
        }

        public virtual void Visit(ScriptSwitchStatement statement)
        {
            Accept(statement.Condition);
            Accept(statement.Cases);
        }

        public virtual void Visit(ScriptThrowStatement statement)
        {
            Accept(statement.Exception);
        }

        public virtual void Visit(ScriptTryStatement statement)
        {
            Accept(statement.TryBody);
            Accept(statement.CatchBody);
            Accept(statement.FinallyBody);
            Accept(statement.Exception);
        }

        public virtual void Visit(ScriptVarStatement statement)
        {
            Accept(statement.Variables);
        }

        public virtual void Visit(ScriptWhileStatement statement)
        {
            Accept(statement.Condition);
            Accept(statement.Body);
        }

        public virtual void Visit(ScriptWithStatement statement)
        {
            Accept(statement.Object);
            Accept(statement.Body);
        }

        public virtual void Visit(ScriptArgumentVariable variable)
        {
            Accept(variable.Name);
        }

        public virtual void Visit(ScriptLocalVariable variable)
        {
            Accept(variable.Name);
            Accept(variable.Initializer);
        }

        public virtual void Visit(ScriptIdentifier identifier)
        {
        }

        public virtual void Visit(ScriptNameValue nameValue)
        {
            Accept(nameValue.Name);
            Accept(nameValue.Value);
        }

        public virtual void Visit(ScriptFunctionDeclaration function)
        {
            Accept(function.Function);
        }

        public virtual void Visit(ScriptSwitchCase switchCase)
        {
            Accept(switchCase.CaseExpression);
            Accept(switchCase.Statements);
        }

        public virtual void Visit(ScriptProgram program)
        {
            Accept(program.SourceElements);
        }

        protected void Accept<T>(T node)
            where T : ScriptNode
        {
            if (node != null)
                node.Accept(this);
        }

        protected void Accept<T>(List<T> nodeList)
            where T : ScriptNode
        {
            if (nodeList != null)
            {
                for (var i = 0; i < nodeList.Count; i++)
                {
                    var current = nodeList[i];
                    if (current != null)
                        current.Accept(this);
                }
            }
        }
    }
}
