namespace Marius.Pinta.Script.Parser.Ast
{
    public class WhileStatement : Statement
    {
        public Expression Test;
        public Statement Body;
    }
}