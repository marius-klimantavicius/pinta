namespace Marius.Pinta.Script.Parser.Ast
{
    public class LabelledStatement : Statement
    {
        public Identifier Label;
        public Statement Body;
    }
}