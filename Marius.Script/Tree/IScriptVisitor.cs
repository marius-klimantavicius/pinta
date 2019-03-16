using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Tree
{
    public interface IScriptVisitor
    {
        void Visit(ScriptArrayAccessExpression expression);
        void Visit(ScriptArrayExpression expression);
        void Visit(ScriptArrayGapElement element);
        void Visit(ScriptArrayExpressionElement element);
        void Visit(ScriptAssignmentExpression expression);
        void Visit(ScriptBinaryExpression expression);
        void Visit(ScriptConditionalExpression expression);
        void Visit(ScriptFunctionExpression expression);
        void Visit(ScriptInvokeExpression expression);
        void Visit(ScriptListExpression expression);
        void Visit(ScriptLiteralExpression expression);
        void Visit(ScriptMemberAccessExpression expression);
        void Visit(ScriptNameExpression expression);
        void Visit(ScriptNewExpression expression);
        void Visit(ScriptObjectExpression expression);
        void Visit(ScriptUnaryExpression expression);

        void Visit(ScriptBlockStatement statement);
        void Visit(ScriptBreakStatement statement);
        void Visit(ScriptContinueStatement statement);
        void Visit(ScriptDebuggerStatement statement);
        void Visit(ScriptDoStatement statement);
        void Visit(ScriptEmptyStatement statement);
        void Visit(ScriptExpressionStatement statement);
        void Visit(ScriptForinStatement statement);
        void Visit(ScriptForStatement statement);
        void Visit(ScriptForVarTarget target);
        void Visit(ScriptForExpressionTarget target);
        void Visit(ScriptIfStatement statement);
        void Visit(ScriptLabelStatement statement);
        void Visit(ScriptReturnStatement statement);
        void Visit(ScriptSwitchStatement statement);
        void Visit(ScriptThrowStatement statement);
        void Visit(ScriptTryStatement statement);
        void Visit(ScriptVarStatement statement);
        void Visit(ScriptWhileStatement statement);
        void Visit(ScriptWithStatement statement);

        void Visit(ScriptArgumentVariable variable);
        void Visit(ScriptLocalVariable variable);

        void Visit(ScriptIdentifier identifier);
        void Visit(ScriptNameValue nameValue);

        void Visit(ScriptFunctionDeclaration function);

        void Visit(ScriptSwitchCase switchCase);

        void Visit(ScriptProgram program);


    }
}
