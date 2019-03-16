%partial
%namespace Marius.Script.Parsing
%start compilation_unit
%visibility internal
%using Marius.Script.Tree

%union {

    public ScriptIdentifier Identifier;

    public List<ScriptSourceElement> SourceElementList;
    public ScriptSourceElement SourceElement;

    public List<ScriptStatement> StatementList;
    public ScriptStatement Statement;
    public List<ScriptVariable> VariableList;
    public ScriptVariable Variable;
    
    public List<ScriptSwitchCase> SwitchCaseList;
    public ScriptSwitchCase SwitchCase;

    public ScriptFunctionDeclaration FunctionDeclaration;

    public ScriptExpression Expression;

    public int Count;
    public string Value;

    public List<ScriptArrayElement> ArrayElementList;
    public List<ScriptNameValue> NameValueList;
    public List<ScriptExpression> ExpressionList;
    public List<ScriptIdentifier> IdentifierList;

    public ScriptNameValue NameValue;
}

%YYLTYPE ScriptSourceSpan
%YYSTYPE ScriptValueType

%type<SourceElementList> source_elements
%type<SourceElement> source_element
%type<StatementList> statement_list
%type<Statement> statement, empty_statement, block_statement, break_statement, continue_statement, return_statement, debugger_statement, loop_statement, expression_statement, switch_statement, if_statement, throw_statement, try_statement, var_statement, with_statement, label_statement
%type<Statement> do_while_statement, while_statement, for_statement

%type<SwitchCaseList> switch_body
%type<SwitchCase> switch_case

%type<VariableList> var_fragment, var_fragment_noin, var_identifier_list, var_identifier_list_noin
%type<Variable> var_identifier, var_identifier_noin

%type<Expression> expression, expression_st, expression_noin, opt_expression
%type<Expression> lhs_expression, lhs_expression_st, assignment_expression, assignment_expression_st, assignment_expression_noin
%type<Expression> primary_expression, primary_expression_st, array_literal, object_literal, property_name
%type<Expression> member_expression, member_expression_st, function_expression, call_expression, call_expression_st
%type<Expression> new_expression, new_expression_st

%type<Expression> literal

%type<FunctionDeclaration> function_declaration

%type<Count> comma_list, opt_comma_list
%type<ArrayElementList> element_list
%type<NameValueList> name_value_list
%type<ExpressionList> arguments, argument_list
%type<IdentifierList> formal_parameter_list

%type<NameValue> property_assignment

%type<Identifier> identifier
%token<Identifier> Identifier

%token KeyBreak "break"
%token KeyFor "for"
%token KeyThrow "throw"
%token KeyCase "case"
%token KeyFunction "function"
%token KeyTry "try"
%token KeyCatch "catch"
%token KeyIf "if"
%token KeyTypeof "typeof"
%token KeyContinue "continue"
%token KeyIn "in"
%token KeyVar "var"
%token KeyDefault "default"
%token KeyInstanceof "instanceof"
%token KeyVoid "void"
%token KeyDelete "delete"
%token KeyNew "new"
%token KeyDo "do"
%token KeyReturn "return"
%token KeyWhile "while"
%token KeyElse "else"
%token KeySwitch "switch"
%token KeyWith "with"
%token KeyFinally "finally"
%token KeyThis "this"
%token KeyDebugger "debugger"

%token KeyProtected "protected"
%token KeyPublic "public"
%token KeyImplements "implements"
%token KeyStatic "static"
%token KeyClass "class"
%token KeyImport "import"
%token KeySuper "super"
%token KeyConst "const"
%token KeyLet "let"
%token KeyInterface "interface"
%token KeyEnum "enum"
%token KeyExport "export"
%token KeyPackage "package"
%token KeyExtends "extends"
%token KeyPrivate "private"
%token KeyYield "yield"

%token KeySet "set"
%token KeyGet "get"

%token KeyTrue "true"
%token KeyFalse "false"
%token KeyNull "null"

%token Increment					"++"
%token Decrement					"--"
%token LogicalAnd					"&&"
%token LogicalOr					"||"
%token SlimArrow					"->"
%token LogicalEqual					"=="
%token LogicalNotEqual				"!="
%token LessEqual					"<="
%token MoreEqual					">="
%token AddAssign					"+="
%token SubtractAssign				"-="
%token MultiplyAssign				"*="
%token DivideAssign					"/="
%token RemainderAssign				"%="
%token AndAssign					"&="
%token OrAssign						"|="
%token ExclusiveOrAssign			"^="
%token ShiftLeft					"<<"
%token ShiftLeftAssign				"<<="
%token Arrow						"=>"
%token ShiftRight					">>"
%token ShiftArithmeticRight			">>>"
%token ShiftRightAssign				">>="
%token ShiftArithmeticRightAssign   ">>>="
%token ExactEqual					"==="
%token ExactNotEqual				"!=="

%token<Value> NumericLiteral 
%token<Value> StringLiteral
%token<Value> RegexLiteral
%token AutoSemicolon "auto-;"

%left ','
%right '=', MultiplyAssign, DivideAssign, RemainderAssign, AddAssign, SubtractAssign, ShiftLeftAssign, ShiftRightAssign, ShiftArithmeticRightAssign, AndAssign, ExclusiveOrAssign, OrAssign
%left '?', ':'
%left LogicalOr
%left LogicalAnd
%left '|'
%left '^'
%left '&'
%left LogicalEqual, LogicalNotEqual, ExactEqual, ExactNotEqual
%left '<', '>', LessEqual, MoreEqual, KeyIn, KeyInstanceof
%left ShiftLeft, ShiftRight, ShiftArithmeticRight
%left '+', '-'
%left '*', '/', '%'
%right '!', '~', KeyTypeof, KeyDelete, KeyVoid
%left Increment, Decrement

%nonassoc IF_Prec
%nonassoc KeyElse

%%

compilation_unit
	:   source_elements                         { Program = new ScriptProgram($1, @1); }
	;

source_elements
    :   /* empty */                             { $$ = new List<ScriptSourceElement>(); }
    |   source_elements source_element          { $$ = $1; $$.Add($2); }
    ;

source_element
    :   statement                               { $$ = $1; }
    |   function_declaration                    { $$ = $1; }
    ;
    
function_declaration
    :   KeyFunction identifier '(' ')' '{' source_elements '}'                          { $$ = new ScriptFunctionDeclaration($2, null, $6, @$); }
    |   KeyFunction identifier '(' formal_parameter_list ')' '{' source_elements '}'    { $$ = new ScriptFunctionDeclaration($2, $4, $7, @$); }
    ;

formal_parameter_list
    :   identifier                              { $$ = new List<ScriptIdentifier>(); $$.Add($1); }
    |   formal_parameter_list ',' identifier    { $$ = $1; $$.Add($3); }
    ;

statement
    :   empty_statement                         { $$ = $1; }
    |   block_statement                         { $$ = $1; }
    |   break_statement                         { $$ = $1; }
    |   continue_statement                      { $$ = $1; }
    |   return_statement                        { $$ = $1; }
    |   debugger_statement                      { $$ = $1; }
    |   loop_statement                          { $$ = $1; }
    |   expression_statement                    { $$ = $1; }
    |   switch_statement                        { $$ = $1; }
    |   if_statement                            { $$ = $1; }
    |   throw_statement                         { $$ = $1; }
    |   try_statement                           { $$ = $1; }
    |   var_statement                           { $$ = $1; }
    |   with_statement                          { $$ = $1; }
    |   label_statement                         { $$ = $1; }
    ;

empty_statement
    :   ';'                                     { $$ = new ScriptEmptyStatement(@1); }
    ;

block_statement
    :   '{' statement_list '}'                  { $$ = new ScriptBlockStatement($2, @$); }
    ;

statement_list
    :   /* empty */                             { $$ = new List<ScriptStatement>(); }
    |   statement_list statement                { $$ = $1; $$.Add($2); }
    ;

break_statement
    :   KeyBreak ';'                            { $$ = new ScriptBreakStatement(@$); }
    |   KeyBreak identifier ';'                 { $$ = new ScriptBreakStatement($2, @$); }
    ;

continue_statement
    :   KeyContinue ';'                         { $$ = new ScriptContinueStatement(@$); }
    |   KeyContinue identifier ';'              { $$ = new ScriptContinueStatement($2, @$); }
    ;

return_statement                                
    :   KeyReturn ';'                           { $$ = new ScriptReturnStatement(@$); }
    |   KeyReturn expression ';'                { $$ = new ScriptReturnStatement($2, @$); }
    ;

debugger_statement
    :   KeyDebugger ';'                         { $$ = new ScriptDebuggerStatement(@$); }
    ;

loop_statement
    :   do_while_statement                      { $$ = $1; }
    |   while_statement                         { $$ = $1; }
    |   for_statement                           { $$ = $1; }
    ;

do_while_statement
    :   KeyDo statement KeyWhile '(' expression ')' ';'             { $$ = new ScriptDoStatement($2, $5, @$); }
    ;

while_statement
    :   KeyWhile '(' expression ')' statement                       { $$ = new ScriptWhileStatement($3, $5, @$); }
    ;

for_statement
    :   KeyFor '(' lhs_expression KeyIn expression ')' statement                            { $$ = new ScriptForinStatement(new ScriptForExpressionTarget($3), $5, $7, @$); }
    |   KeyFor '(' var_fragment_noin KeyIn expression ')' statement                         { $$ = new ScriptForinStatement(new ScriptForVarTarget($3), $5, $7, @$); }
    |   KeyFor '(' var_fragment_noin ';' opt_expression ';' opt_expression ')' statement    { $$ = new ScriptForStatement(new ScriptForVarTarget($3), $5, $7, $9, @$); }
    |   KeyFor '(' expression_noin ';' opt_expression ';' opt_expression ')' statement      { $$ = new ScriptForStatement(new ScriptForExpressionTarget($3), $5, $7, $9, @$); }
    |   KeyFor '(' ';' opt_expression ';' opt_expression ')' statement                      { $$ = new ScriptForStatement(null, $4, $6, $8, @$); }
    ;

expression_statement
    :   expression_st ';'                       { $$ = new ScriptExpressionStatement($1, @$); }
    ;

switch_statement
    :   KeySwitch '(' expression ')' '{' switch_body '}'            { $$ = new ScriptSwitchStatement($3, $6, @$); }
    ;

switch_body
    :   /* empty */                             { $$ = new List<ScriptSwitchCase>(); }
    |   switch_body switch_case                 { $$ = $1; $$.Add($2); }
    ;

switch_case
    :   KeyCase expression ':' statement_list                       { $$ = new ScriptSwitchCase($2, $4, @$); }
    |   KeyDefault ':' statement_list                               { $$ = new ScriptSwitchCase(null, $3, @$); }
    ;

if_statement
    :   KeyIf '(' expression ')' statement  %prec IF_Prec           { $$ = new ScriptIfStatement($3, $5, null, @$); }
    |   KeyIf '(' expression ')' statement KeyElse statement        { $$ = new ScriptIfStatement($3, $5, $7, @$); }
    ;

throw_statement
    :   KeyThrow expression ';'                                     { $$ = new ScriptThrowStatement($2, @$); }
    ;

try_statement
    :   KeyTry block_statement KeyCase '(' identifier ')' block_statement                               { $$ = new ScriptTryStatement($2, $5, $7, null, @$); }
    |   KeyTry block_statement KeyFinally block_statement                                               { $$ = new ScriptTryStatement($2, null, null, $4, @$); }
    |   KeyTry block_statement KeyCase '(' identifier ')' block_statement KeyFinally block_statement    { $$ = new ScriptTryStatement($2, $5, $7, $9, @$); }
    ;

var_statement
    :   var_fragment ';'                        { $$ = new ScriptVarStatement($1, @$); }
    ;

var_fragment
    :   KeyVar var_identifier_list              { $$ = $2; }
    ;

var_fragment_noin
    :   KeyVar var_identifier_list_noin         { $$ = $2; }
    ;

var_identifier_list
    :   var_identifier                          { $$ = new List<ScriptVariable>(); $$.Add($1); }
    |   var_identifier_list ',' var_identifier  { $$ = $1; $$.Add($3); }
    ;

var_identifier_list_noin
    :   var_identifier_noin                                 { $$ = new List<ScriptVariable>(); $$.Add($1); }
    |   var_identifier_list_noin ',' var_identifier_noin    { $$ = $1; $$.Add($3); }
    ;

var_identifier
    :   identifier                              { $$ = new ScriptLocalVariable($1, null, @$); }
    |   identifier '=' assignment_expression    { $$ = new ScriptLocalVariable($1, $3, @$); }
    ;

var_identifier_noin
    :   identifier                                          { $$ = new ScriptLocalVariable($1, null, @$); }
    |   identifier '=' assignment_expression_noin           { $$ = new ScriptLocalVariable($1, $3, @$); }
    ;

with_statement
    :   KeyWith '(' expression ')' statement    { $$ = new ScriptWithStatement($3, $5, @$); }
    ;

label_statement
    :   identifier ':' statement                { $$ = new ScriptLabelStatement($1, $3, @$); }
    ;

primary_expression
    :   KeyThis                                 { $$ = new ScriptLiteralExpression(ScriptLiteralKind.This, @$); }
    |   identifier                              { $$ = new ScriptNameExpression($1, @$); }
    |   literal                                 { $$ = $1; }
    |   array_literal                           { $$ = $1; }
    |   object_literal                          { $$ = $1; }
    |   '(' expression ')'                      { $$ = $2; }
    ;

primary_expression_st
    :   KeyThis                                 { $$ = new ScriptLiteralExpression(ScriptLiteralKind.This, @$); }
    |   identifier                              { $$ = new ScriptNameExpression($1, @$); }
    |   literal                                 { $$ = $1; }
    |   array_literal                           { $$ = $1; }
    |   '(' expression ')'                      { $$ = $2; }
    ;

literal
    :   StringLiteral                           { $$ = ScriptLiteralExpression.String($1, @1); }    
    |   NumericLiteral                          { $$ = ScriptLiteralExpression.Numeric($1, @1); }
    |   KeyTrue                                 { $$ = ScriptLiteralExpression.True; }
    |   KeyFalse                                { $$ = ScriptLiteralExpression.False; }
    |   KeyNull                                 { $$ = ScriptLiteralExpression.Null; }
    |   '/'                                     { $$ = RegexLiteral(); } 
    |   DivideAssign                            { $$ = RegexLiteral(); }
    ;

array_literal
    :   '[' opt_comma_list ']'                      { $$ = new ScriptArrayExpression(new ScriptArrayElement[] { new ScriptArrayGapElement($2) }, @$); }
    |   '[' element_list ']'                        { $$ = new ScriptArrayExpression($2, @$); }
    |   '[' element_list ',' opt_comma_list ']'     { $2.Add(new ScriptArrayGapElement($4 + 1)); $$ = new ScriptArrayExpression($2, @$); }
    ;

element_list
    :   opt_comma_list assignment_expression                    { $$ = new List<ScriptArrayElement>(); if ($1 > 0) $$.Add(new ScriptArrayGapElement($1)); $$.Add(new ScriptArrayExpressionElement($2)); }
    |   element_list ',' opt_comma_list assignment_expression   { $$ = $1; if ($3 > 0) $$.Add(new ScriptArrayGapElement($3)); $$.Add(new ScriptArrayExpressionElement($4)); }
    ;

comma_list
    :   ','                                     { $$ = 1; }
    |   comma_list ','                          { $$ = $1 + 1; }
    ;

opt_comma_list
    :   /* empty */                             { $$ = 0; }
    |   comma_list                              { $$ = $1; }
    ;

object_literal
    :   '{' '}'                                 { $$ = new ScriptObjectExpression(@$); }
    |   '{' name_value_list '}'                 { $$ = new ScriptObjectExpression($2, @$); }
    |   '{' name_value_list ',' '}'             { $$ = new ScriptObjectExpression($2, @$); }
    ;

name_value_list
    :   property_assignment                         { $$ = new List<ScriptNameValue>(); $$.Add($1); }
    |   name_value_list ',' property_assignment     { $$ = $1; $$.Add($3); }
    ;
   
property_assignment
    :   property_name ':' assignment_expression                             { $$ = new ScriptNameValue($1, $3, @$); }
    |   KeyGet ':' assignment_expression                                    { $$ = new ScriptNameValue(new ScriptNameExpression(new ScriptIdentifier("get", @1), @1), $3, @$); }
    |   KeySet ':' assignment_expression                                    { $$ = new ScriptNameValue(new ScriptNameExpression(new ScriptIdentifier("set", @1), @1), $3, @$); }
    |   KeyGet property_name '(' ')' '{' source_elements '}'                { $$ = ScriptNameValue.Getter($2, new ScriptFunctionExpression(null, null, $6, @$), @$); }
    |   KeySet property_name '(' identifier ')' '{' source_elements '}'     { $$ = ScriptNameValue.Setter($2, new ScriptFunctionExpression(null, new []{ $4 }, $7, @$), @$); }
    ;

property_name
    :   Identifier                              { $$ = new ScriptNameExpression($1, @1); }
    |   StringLiteral                           { $$ = ScriptLiteralExpression.String($1, @1); }
    |   NumericLiteral                          { $$ = ScriptLiteralExpression.Numeric($1, @1); }
    ;

member_expression
    :   primary_expression                      { $$ = $1; }
    |   function_expression                     { $$ = $1; }
    |   member_expression '[' expression ']'    { $$ = new ScriptArrayAccessExpression($1, $3, @$); }
    |   member_expression '.' identifier        { $$ = new ScriptMemberAccessExpression($1, $3, @$); }
    |   KeyNew member_expression arguments      { $$ = new ScriptNewExpression($2, $3, @$); }
    ;

member_expression_st
    :   primary_expression_st                   { $$ = $1; }
    |   member_expression_st '[' expression ']' { $$ = new ScriptArrayAccessExpression($1, $3, @$); }
    |   member_expression_st '.' identifier     { $$ = new ScriptMemberAccessExpression($1, $3, @$); }
    |   KeyNew member_expression arguments      { $$ = new ScriptNewExpression($2, $3, @$); }
    ;

function_expression
    :   KeyFunction '(' ')' '{' source_elements '}'                                     { $$ = new ScriptFunctionExpression(null, null, $5, @$); }
    |   KeyFunction '(' formal_parameter_list ')' '{' source_elements '}'               { $$ = new ScriptFunctionExpression(null, $3, $6, @$); }
    |   KeyFunction identifier '(' ')' '{' source_elements '}'                          { $$ = new ScriptFunctionExpression($2, null, $6, @$); }
    |   KeyFunction identifier '(' formal_parameter_list ')' '{' source_elements '}'    { $$ = new ScriptFunctionExpression($2, $4, $7, @$); }
    ;

new_expression
    :   member_expression                       { $$ = $1; }
    |   KeyNew new_expression                   { $$ = new ScriptNewExpression($2, @$); }
    ;

new_expression_st
    :   member_expression_st                    { $$ = $1; }
    |   KeyNew new_expression                   { $$ = new ScriptNewExpression($2, @$); }
    ;

call_expression
    :   member_expression arguments             { $$ = new ScriptInvokeExpression($1, $2, @$); }
    |   call_expression arguments               { $$ = new ScriptInvokeExpression($1, $2, @$); }
    |   call_expression '[' expression ']'      { $$ = new ScriptArrayAccessExpression($1, $3, @$); }
    |   call_expression '.' identifier          { $$ = new ScriptMemberAccessExpression($1, $3, @$); }
    ;

call_expression_st
    :   member_expression_st arguments          { $$ = new ScriptInvokeExpression($1, $2, @$); }
    |   call_expression_st arguments            { $$ = new ScriptInvokeExpression($1, $2, @$); }
    |   call_expression_st '[' expression ']'   { $$ = new ScriptArrayAccessExpression($1, $3, @$); }
    |   call_expression_st '.' identifier       { $$ = new ScriptMemberAccessExpression($1, $3, @$); }
    ;

arguments
    :   '(' ')'                                 { $$ = new List<ScriptExpression>(); }
    |   '(' argument_list ')'                   { $$ = $2; }
    ;

argument_list
    :   assignment_expression                   { $$ = new List<ScriptExpression>();  $$.Add($1); }
    |   argument_list ',' assignment_expression { $$ = $1; $$.Add($3); }
    ;

lhs_expression
    :   new_expression                          { $$ = $1; }
    |   call_expression                         { $$ = $1; }
    ;

lhs_expression_st
    :   new_expression_st                       { $$ = $1; }
    |   call_expression_st                      { $$ = $1; }
    ;

assignment_expression
    :   lhs_expression                                                                                  { $$ = $1; }
    |   lhs_expression Increment                                                                        { $$ = new ScriptUnaryExpression($1, ScriptUnaryOperator.PostIncrement, @$); }
    |   lhs_expression Decrement                                                                        { $$ = new ScriptUnaryExpression($1, ScriptUnaryOperator.PostDecrement, @$); }
    |   KeyDelete assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Delete, @$); }
    |   KeyVoid assignment_expression                                                                   { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Void, @$); }
    |   KeyTypeof assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Typeof, @$); }
    |   Increment assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.PreIncrement, @$); }
    |   Decrement assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.PreDecrement, @$); }
    |   '+' assignment_expression     %prec KeyVoid                                                     { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Plus, @$); }
    |   '-' assignment_expression     %prec KeyVoid                                                     { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Minus, @$); }
    |   '~' assignment_expression                                                                       { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.BitwiseNot, @$); }
    |   '!' assignment_expression                                                                       { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.LogicalNot, @$); }
    |   assignment_expression '*' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Multiply, @$); }
    |   assignment_expression '/' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Divide, @$); }
    |   assignment_expression '%' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Modulus, @$); }
    |   assignment_expression '+' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Add, @$); }
    |   assignment_expression '-' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Subtract, @$); }
    |   assignment_expression ShiftLeft assignment_expression                                           { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftLeft, @$); }
    |   assignment_expression ShiftRight assignment_expression                                          { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftRight, @$); }
    |   assignment_expression ShiftArithmeticRight assignment_expression                                { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftArithmeticRight, @$); }
    |   assignment_expression '<' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Less, @$); }
    |   assignment_expression '>' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.More, @$); }
    |   assignment_expression LessEqual assignment_expression                                           { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LessOrEqual, @$); }
    |   assignment_expression MoreEqual assignment_expression                                           { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.MoreOrEqual, @$); }
    |   assignment_expression KeyInstanceof assignment_expression                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.InstanceOf, @$); }
    |   assignment_expression KeyIn assignment_expression                                               { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.In, @$); }
    |   assignment_expression LogicalEqual assignment_expression                                        { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Equals, @$); }
    |   assignment_expression LogicalNotEqual assignment_expression                                     { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.NotEquals, @$); }
    |   assignment_expression ExactEqual assignment_expression                                          { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.StrictEquals, @$); }
    |   assignment_expression ExactNotEqual assignment_expression                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.StrictNotEquals, @$); }
    |   assignment_expression '&' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.BitwiseAnd, @$); }
    |   assignment_expression '^' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ExclusiveOr, @$); }
    |   assignment_expression '|' assignment_expression                                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.BitwiseOr, @$); }
    |   assignment_expression LogicalAnd assignment_expression                                          { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LogicalAnd, @$); }
    |   assignment_expression LogicalOr assignment_expression                                           { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LogicalOr, @$); }
    |   assignment_expression '?' assignment_expression ':' assignment_expression                       { $$ = new ScriptConditionalExpression($1, $3, $5, @$); }
    |   lhs_expression '=' assignment_expression                                                        { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.Assign, @$); }
    |   lhs_expression MultiplyAssign assignment_expression                                             { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.MultiplyAssign, @$); }
    |   lhs_expression DivideAssign assignment_expression                                               { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.DivideAssign, @$); }
    |   lhs_expression RemainderAssign assignment_expression                                            { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ModulusAssign, @$); }
    |   lhs_expression AddAssign assignment_expression                                                  { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.AddAssign, @$); }
    |   lhs_expression SubtractAssign assignment_expression                                             { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.SubtractAssign, @$); }
    |   lhs_expression ShiftLeftAssign assignment_expression                                            { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftLeftAssign, @$); }
    |   lhs_expression ShiftRightAssign assignment_expression                                           { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftRightAssign, @$); }
    |   lhs_expression ShiftArithmeticRightAssign assignment_expression                                 { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftArithmeticRightAssign, @$); }
    |   lhs_expression AndAssign assignment_expression                                                  { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.BitwiseAndAssign, @$); }
    |   lhs_expression ExclusiveOrAssign assignment_expression                                          { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ExclusiveOrAssign, @$); }
    |   lhs_expression OrAssign assignment_expression                                                   { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.BitwiseOrAssign, @$); }
    ;

assignment_expression_noin
    :   lhs_expression                                                                                  { $$ = $1; }
    |   lhs_expression Increment                                                                        { $$ = new ScriptUnaryExpression($1, ScriptUnaryOperator.PostIncrement, @$); }
    |   lhs_expression Decrement                                                                        { $$ = new ScriptUnaryExpression($1, ScriptUnaryOperator.PostDecrement, @$); }
    |   KeyDelete assignment_expression_noin                                                            { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Delete, @$); }
    |   KeyVoid assignment_expression_noin                                                              { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Void, @$); }
    |   KeyTypeof assignment_expression_noin                                                            { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Typeof, @$); }
    |   Increment assignment_expression_noin                                                            { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.PreIncrement, @$); }
    |   Decrement assignment_expression_noin                                                            { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.PreDecrement, @$); }
    |   '+' assignment_expression_noin     %prec KeyVoid                                                { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Plus, @$); }
    |   '-' assignment_expression_noin     %prec KeyVoid                                                { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Minus, @$); }
    |   '~' assignment_expression_noin                                                                  { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.BitwiseNot, @$); }
    |   '!' assignment_expression_noin                                                                  { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.LogicalNot, @$); }
    |   assignment_expression_noin '*' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Multiply, @$); }
    |   assignment_expression_noin '/' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Divide, @$); }
    |   assignment_expression_noin '%' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Modulus, @$); }
    |   assignment_expression_noin '+' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Add, @$); }
    |   assignment_expression_noin '-' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Subtract, @$); }
    |   assignment_expression_noin ShiftLeft assignment_expression_noin                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftLeft, @$); }
    |   assignment_expression_noin ShiftRight assignment_expression_noin                                { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftRight, @$); }
    |   assignment_expression_noin ShiftArithmeticRight assignment_expression_noin                      { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftArithmeticRight, @$); }
    |   assignment_expression_noin '<' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Less, @$); }
    |   assignment_expression_noin '>' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.More, @$); }
    |   assignment_expression_noin LessEqual assignment_expression_noin                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LessOrEqual, @$); }
    |   assignment_expression_noin MoreEqual assignment_expression_noin                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.MoreOrEqual, @$); }
    |   assignment_expression_noin KeyInstanceof assignment_expression_noin                             { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.InstanceOf, @$); }
    |   assignment_expression_noin LogicalEqual assignment_expression_noin                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Equals, @$); }
    |   assignment_expression_noin LogicalNotEqual assignment_expression_noin                           { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.NotEquals, @$); }
    |   assignment_expression_noin ExactEqual assignment_expression_noin                                { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.StrictEquals, @$); }
    |   assignment_expression_noin ExactNotEqual assignment_expression_noin                             { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.StrictNotEquals, @$); }
    |   assignment_expression_noin '&' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.BitwiseAnd, @$); }
    |   assignment_expression_noin '^' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ExclusiveOr, @$); }
    |   assignment_expression_noin '|' assignment_expression_noin                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.BitwiseOr, @$); }
    |   assignment_expression_noin LogicalAnd assignment_expression_noin                                { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LogicalAnd, @$); }
    |   assignment_expression_noin LogicalOr assignment_expression_noin                                 { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LogicalOr, @$); }
    |   assignment_expression_noin '?' assignment_expression_noin ':' assignment_expression_noin        { $$ = new ScriptConditionalExpression($1, $3, $5, @$); }
    |   lhs_expression '=' assignment_expression_noin                                                   { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.Assign, @$); }
    |   lhs_expression MultiplyAssign assignment_expression_noin                                        { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.MultiplyAssign, @$); }
    |   lhs_expression DivideAssign assignment_expression_noin                                          { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.DivideAssign, @$); }
    |   lhs_expression RemainderAssign assignment_expression_noin                                       { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ModulusAssign, @$); }
    |   lhs_expression AddAssign assignment_expression_noin                                             { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.AddAssign, @$); }
    |   lhs_expression SubtractAssign assignment_expression_noin                                        { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.SubtractAssign, @$); }
    |   lhs_expression ShiftLeftAssign assignment_expression_noin                                       { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftLeftAssign, @$); }
    |   lhs_expression ShiftRightAssign assignment_expression_noin                                      { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftRightAssign, @$); }
    |   lhs_expression ShiftArithmeticRightAssign assignment_expression_noin                            { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftArithmeticRightAssign, @$); }
    |   lhs_expression AndAssign assignment_expression_noin                                             { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.BitwiseAndAssign, @$); }
    |   lhs_expression ExclusiveOrAssign assignment_expression_noin                                     { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ExclusiveOrAssign, @$); }
    |   lhs_expression OrAssign assignment_expression_noin                                              { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.BitwiseOrAssign, @$); }
    ;                                                                                                     

assignment_expression_st
    :   lhs_expression_st                                                                               { $$ = $1; }
    |   lhs_expression_st Increment                                                                     { $$ = new ScriptUnaryExpression($1, ScriptUnaryOperator.PostIncrement, @$); }
    |   lhs_expression_st Decrement                                                                     { $$ = new ScriptUnaryExpression($1, ScriptUnaryOperator.PostDecrement, @$); }
    |   KeyDelete assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Delete, @$); }
    |   KeyVoid assignment_expression                                                                   { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Void, @$); }
    |   KeyTypeof assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Typeof, @$); }
    |   Increment assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.PreIncrement, @$); }
    |   Decrement assignment_expression                                                                 { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.PreDecrement, @$); }
    |   '+' assignment_expression                                                                       { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Plus, @$); }
    |   '-' assignment_expression                                                                       { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.Minus, @$); }
    |   '~' assignment_expression                                                                       { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.BitwiseNot, @$); }
    |   '!' assignment_expression                                                                       { $$ = new ScriptUnaryExpression($2, ScriptUnaryOperator.LogicalNot, @$); }
    |   assignment_expression_st '*' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Multiply, @$); }
    |   assignment_expression_st '/' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Divide, @$); }
    |   assignment_expression_st '%' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Modulus, @$); }
    |   assignment_expression_st '+' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Add, @$); }
    |   assignment_expression_st '-' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Subtract, @$); }
    |   assignment_expression_st ShiftLeft assignment_expression                                        { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftLeft, @$); }
    |   assignment_expression_st ShiftRight assignment_expression                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftRight, @$); }
    |   assignment_expression_st ShiftArithmeticRight assignment_expression                             { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ShiftArithmeticRight, @$); }
    |   assignment_expression_st '<' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Less, @$); }
    |   assignment_expression_st '>' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.More, @$); }
    |   assignment_expression_st LessEqual assignment_expression                                        { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LessOrEqual, @$); }
    |   assignment_expression_st MoreEqual assignment_expression                                        { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.MoreOrEqual, @$); }
    |   assignment_expression_st KeyInstanceof assignment_expression                                    { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.InstanceOf, @$); }
    |   assignment_expression_st KeyIn assignment_expression                                            { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.In, @$); }
    |   assignment_expression_st LogicalEqual assignment_expression                                     { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.Equals, @$); }
    |   assignment_expression_st LogicalNotEqual assignment_expression                                  { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.NotEquals, @$); }
    |   assignment_expression_st ExactEqual assignment_expression                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.StrictEquals, @$); }
    |   assignment_expression_st ExactNotEqual assignment_expression                                    { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.StrictNotEquals, @$); }
    |   assignment_expression_st '&' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.BitwiseAnd, @$); }
    |   assignment_expression_st '^' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.ExclusiveOr, @$); }
    |   assignment_expression_st '|' assignment_expression                                              { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.BitwiseOr, @$); }
    |   assignment_expression_st LogicalAnd assignment_expression                                       { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LogicalAnd, @$); }
    |   assignment_expression_st LogicalOr assignment_expression                                        { $$ = new ScriptBinaryExpression($1, $3, ScriptBinaryOperator.LogicalOr, @$); }
    |   assignment_expression_st '?' assignment_expression ':' assignment_expression                    { $$ = new ScriptConditionalExpression($1, $3, $5, @$); }
    |   lhs_expression_st '=' assignment_expression                                                     { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.Assign, @$); }
    |   lhs_expression_st MultiplyAssign assignment_expression                                          { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.MultiplyAssign, @$); }
    |   lhs_expression_st DivideAssign assignment_expression                                            { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.DivideAssign, @$); }
    |   lhs_expression_st RemainderAssign assignment_expression                                         { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ModulusAssign, @$); }
    |   lhs_expression_st AddAssign assignment_expression                                               { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.AddAssign, @$); }
    |   lhs_expression_st SubtractAssign assignment_expression                                          { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.SubtractAssign, @$); }
    |   lhs_expression_st ShiftLeftAssign assignment_expression                                         { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftLeftAssign, @$); }
    |   lhs_expression_st ShiftRightAssign assignment_expression                                        { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftRightAssign, @$); }
    |   lhs_expression_st ShiftArithmeticRightAssign assignment_expression                              { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ShiftArithmeticRightAssign, @$); }
    |   lhs_expression_st AndAssign assignment_expression                                               { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.BitwiseAndAssign, @$); }
    |   lhs_expression_st ExclusiveOrAssign assignment_expression                                       { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.ExclusiveOrAssign, @$); }
    |   lhs_expression_st OrAssign assignment_expression                                                { $$ = new ScriptAssignmentExpression($1, $3, ScriptAssignmentOperator.BitwiseOrAssign, @$); }
    ;

expression
    :   assignment_expression                   { $$ = $1; }
    |   expression ',' assignment_expression    { $$ = MakeListExpression($1, $3, @$); }
    ;

expression_noin
    :   assignment_expression_noin                      { $$ = $1; }
    |   expression_noin ',' assignment_expression_noin  { $$ = MakeListExpression($1, $3, @$); }
    ;

expression_st
    :   assignment_expression_st                { $$ = $1; }
    |   expression_st ',' assignment_expression { $$ = MakeListExpression($1, $3, @$); }
    ;

opt_expression
    :   /* empty */                             { $$ = null; }
    |   expression                              { $$ = $1; }
    ;

identifier
    :   Identifier                              { $$ = $1; }
    |   KeyGet                                  { $$ = new ScriptIdentifier("get", @1); }
    |   KeySet                                  { $$ = new ScriptIdentifier("set", @1); }
    ;
%%