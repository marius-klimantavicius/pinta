namespace Marius.Pinta.Script.Parser.Ast
{
    public class VariableDeclarator : Expression
    {
        public Identifier Id;
        public Expression Init;
    }
}