%{
#include "scanner.hpp"
#include "yyparse.hpp"
#include "debug.hpp"

using namespace ast;
%}

%union { ast::Node* node; Mutability mutability; }
%token <node> IDENTIFIER
%token <node> LITERAL

%type <node> Action
%type <node> ActivateStatement
%type <node> AddExpression
%type <node> AndExpression
%type <node> ArrayDimension
%type <node> ArrayType
%type <node> AssignmentStatement
%type <node> Bind
%type <node> BindStatement
%type <node> Block
%type <node> ChangeStatement
%type <node> CompareExpression
%type <node> ComponentType
%type <node> Const
%type <node> Definition
%type <node> DefinitionList
%type <node> ElementType
%type <node> EmptyStatement
%type <node> Expression
%type <node> ExpressionList
%type <node> ExpressionStatement
%type <node> FieldList
%type <node> ForIotaStatement
%type <node> Func
%type <node> Getter
%type <node> HeapType
%type <node> IdentifierList
%type <node> IfStatement
%type <node> IncrementStatement
%type <node> IndexExpression
%type <node> Init
%type <node> Instance
%type <node> KeyType
%type <node> LiteralValue
%type <node> MapType
%type <node> Method
%type <node> MultiplyExpression
%type <node> OptionalExpression
%type <node> OptionalExpressionList
%type <node> OptionalPushPortCallList
%type <node> OptionalTypeOrExpressionList
%type <node> OrExpression
%type <node> Parameter
%type <node> ParameterList
%type <node> PointerType
%type <node> PrimaryExpression
%type <node> PullPortType
%type <node> PushPortCall
%type <node> PushPortCallList
%type <node> PushPortType
%type <node> Reaction
%type <node> Receiver
%type <node> ReturnStatement
%type <node> Signature
%type <node> SimpleStatement
%type <node> SliceType
%type <node> Statement
%type <node> StatementList
%type <node> StructType
%type <node> Type
%type <node> TypeDecl
%type <node> TypeLit
%type <node> TypeName
%type <node> TypeLitExpression
%type <node> TypeOrExpressionList
%type <node> UnaryExpression
%type <node> Value
%type <node> VarStatement
%type <node> WhileStatement

%destructor { /* TODO:  Free the node. node_free ($$); */ } <node>

%type <mutability> Mutability
%type <mutability> DereferenceMutability

%token ACTION ACTIVATE BIND BREAK CASE CHANGE COMPONENT CONST CONTINUE DEFAULT ELSE ENUM FALLTHROUGH FOR FOREIGN_KW FUNC GETTER GOTO HEAP IF INIT INSTANCE INTERFACE MAP PULL PUSH RANGE REACTION RETURN_KW STRUCT SWITCH TYPE VAR

%token LEFT_SHIFT RIGHT_SHIFT AND_NOT ADD_ASSIGN SUBTRACT_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN MODULUS_ASSIGN AND_ASSIGN OR_ASSIGN XOR_ASSIGN LEFT_SHIFT_ASSIGN RIGHT_SHIFT_ASSIGN AND_NOT_ASSIGN LOGIC_AND LOGIC_OR LEFT_ARROW RIGHT_ARROW INCREMENT DECREMENT EQUAL NOT_EQUAL LESS_EQUAL MORE_EQUAL SHORT_ASSIGN DOTDOTDOT

%precedence '{'
%precedence IDENTIFIER

%%

top: DefinitionList { root = $1; }

DefinitionList: /* empty */ { $$ = new SourceFile (); }
| DefinitionList Definition { $$ = $1->Append ($2); }

Definition: TypeDecl { $$ = $1; }
| Init { $$ = $1; }
| Getter { $$ = $1; }
| Action { $$ = $1; }
| Reaction { $$ = $1; }
| Bind { $$ = $1; }
| Instance { $$ = $1; }
| Method { $$ = $1; }
| Func { $$ = $1; }
| Const { $$ = $1; }

Const:
  CONST IdentifierList Type '=' ExpressionList ';'
  { $$ = new ast_const_t (@1, $2, $3, $5); }
| CONST IdentifierList          '=' ExpressionList ';'
  { $$ = new ast_const_t (@1, $2, new ast_empty_type_spec_t (@1), $4); }

Instance: INSTANCE IDENTIFIER TypeName IDENTIFIER '(' OptionalExpressionList ')' ';' { $$ = new ast_instance_t (@1, $2, $3, $4, $6); }

TypeDecl: TYPE IDENTIFIER Type ';' { $$ = new ast::Type (@1, $2, $3); }

Mutability:
/* Empty. */
{ $$ = MUTABLE; }
| CONST
{ $$ = IMMUTABLE; }
| FOREIGN_KW
{ $$ = FOREIGN; }

DereferenceMutability:
  /* Empty. */   { $$ = MUTABLE; }
| '$' CONST      { $$ = IMMUTABLE; }
| '$' FOREIGN_KW { $$ = FOREIGN; }

Receiver:
  '(' IDENTIFIER Mutability DereferenceMutability '*' IDENTIFIER ')'
{ $$ = new ast_receiver_t (@1, $2, $3, $4, true, $6); }
| '(' IDENTIFIER Mutability DereferenceMutability IDENTIFIER ')'
{ $$ = new ast_receiver_t (@1, $2, $3, $4, false, $5); }

Action:
  ACTION Receiver IDENTIFIER                '(' Expression ')' Block
{ $$ = new ast_action_t (@1, $2, $3, $5, $7); }
| ArrayDimension ACTION Receiver IDENTIFIER '(' Expression ')' Block
{ $$ = new ast_dimensioned_action_t (@1, $1, $3, $4, $6, $8); }

Reaction:
REACTION Receiver IDENTIFIER Signature Block
{ $$ = new ast_reaction_t (@1, $2, $3, $4, $5); }
| ArrayDimension REACTION Receiver IDENTIFIER Signature Block
{ $$ = new ast_dimensioned_reaction_t (@2, $1, $3, $4, $5, $6); }

Bind:
BIND Receiver IDENTIFIER Block
{ $$ = new ast_bind_t (@1, $2, $3, $4); }

Init:
INIT Receiver IDENTIFIER Signature DereferenceMutability Type Block
{ $$ = new ast_initializer_t (@1, $2, $3, $4, $5, $6, $7); }
| INIT Receiver IDENTIFIER Signature           Block
{ $$ = new ast_initializer_t (@1, $2, $3, $4, IMMUTABLE, new ast_empty_type_spec_t (@1), $5); }

Getter:
GETTER Receiver IDENTIFIER Signature DereferenceMutability Type Block
{ $$ = new ast_getter_t (@1, $2, $3, $4, $5, $6, $7); }

Method:
FUNC Receiver IDENTIFIER Signature DereferenceMutability Type Block
{ $$ = new ast_method_t (@1, $2, $3, $4, $5, $6, $7); }
| FUNC Receiver IDENTIFIER Signature           Block
{ $$ = new ast_method_t (@1, $2, $3, $4, IMMUTABLE, new ast_empty_type_spec_t (@1), $5); }

Func:
FUNC IDENTIFIER Signature Block
{ $$ = new ast_function_t (@1, $2, $3, IMMUTABLE, new ast_empty_type_spec_t (@1), $4); }
| FUNC IDENTIFIER Signature DereferenceMutability Type Block
{ $$ = new ast_function_t (@1, $2, $3, $4, $5, $6); }

Signature: '(' ')' { $$ = new ast_signature_type_spec_t (yyloc); }
| '(' ParameterList optional_semicolon ')' { $$ = $2; }

ParameterList: Parameter { $$ = (new ast_signature_type_spec_t (@1))->Append ($1); }
| ParameterList ';' Parameter { $$ = $1->Append ($3); }

Parameter:
  IdentifierList Mutability DereferenceMutability Type
{ $$ = new ast_identifier_list_type_spec_t (@1, $1, $2, $3, $4); }

optional_semicolon: /* Empty. */
| ';'

BindStatement: Expression RIGHT_ARROW Expression ';' { $$ = new ast_bind_push_port_statement_t (@1, $1, $3); } /* CHECK */
| Expression RIGHT_ARROW Expression DOTDOTDOT Expression';' { $$ = new ast_bind_push_port_param_statement_t (@1, $1, $3, $5); } /* CHECK */
| Expression LEFT_ARROW Expression ';' { $$ = new ast_bind_pull_port_statement_t (@1, $1, $3); } /* CHECK */

Block: '{' StatementList '}' { $$ = $2; }

StatementList: /* empty */ { $$ = new ast_list_statement_t (yyloc); }
| StatementList Statement { $$ = $1->Append ($2); }

Statement: SimpleStatement { $$ = $1; }
| VarStatement { $$ = $1; }
| ActivateStatement { $$ = $1; }
| Block { $$ = $1; }
| ReturnStatement { $$ = $1; }
| IfStatement { $$ = $1; }
| WhileStatement { $$ = $1; }
| ChangeStatement { $$ = $1; }
| ForIotaStatement { $$ = $1; }
| BindStatement { $$ = $1; }
| Const { $$ = $1; }

SimpleStatement: EmptyStatement { $$ = $1; }
| ExpressionStatement { $$ = $1; }
| IncrementStatement { $$ = $1; }
| AssignmentStatement { $$ = $1; }

EmptyStatement: /* empty */ ';' { $$ = new ast_empty_statement_t (yyloc); }

ActivateStatement: ACTIVATE OptionalPushPortCallList Block { $$ = new ast_activate_statement_t (@1, $2, $3); }

ChangeStatement: CHANGE '(' Expression ',' IDENTIFIER ')' Block { $$ = new ast_change_statement_t (@1, $3, $5, $7); }

ForIotaStatement: FOR '(' IDENTIFIER DOTDOTDOT Expression ')' Block {
  unimplemented;
  // $$ = new ast_for_iota_statement_t (@1, $2, $4, $5);
 }

ReturnStatement: RETURN_KW Expression ';' { $$ = new ast_return_statement_t (@1, $2); }

IncrementStatement: Expression INCREMENT ';' { $$ = new ast_increment_statement_t (@1, $1); } /* CHECK */
| Expression DECREMENT ';' { $$ = new ast_decrement_statement_t (@1, $1); } /* CHECK */

OptionalPushPortCallList: /* Empty. */ { $$ = new ast_list_expr_t (yyloc); }
| PushPortCallList { $$ = $1; }

PushPortCallList: PushPortCall { $$ = (new ast_list_expr_t (@1))->Append ($1); }
| PushPortCallList ',' PushPortCall { $$ = $1->Append ($3); }

PushPortCall: IDENTIFIER IndexExpression '(' OptionalExpressionList ')' { $$ = new ast_indexed_port_call_expr_t (@1, $1, $2, $4); }
| IDENTIFIER '(' OptionalExpressionList ')' { $$ = new ast_push_port_call_expr_t (@1, $1, $3); }

IndexExpression: '[' Expression ']' { $$ = $2; }

OptionalExpressionList: /* Empty. */ { $$ = new ast_list_expr_t (yyloc); }
| ExpressionList { $$ = $1; }

OptionalTypeOrExpressionList: /* Empty. */ { $$ = new ast_list_expr_t (yyloc); }
| TypeOrExpressionList { $$ = $1; }

ExpressionList: Expression { $$ = (new ast_list_expr_t (@1))->Append ($1); }
| ExpressionList ',' Expression { $$ = $1->Append ($3); }

TypeOrExpressionList:
  Expression { $$ = (new ast_list_expr_t (@1))->Append ($1); }
| TypeLitExpression { $$ = (new ast_list_expr_t (@1))->Append (new TypeExpression (@1, $1)); }
| TypeOrExpressionList ',' Expression { $$ = $1->Append ($3); }
| TypeOrExpressionList ',' TypeLitExpression { $$ = $1->Append (new TypeExpression (@1, $3)); }

ExpressionStatement: Expression ';' {
  $$ = new ast_expression_statement_t (@1, $1);
 }

VarStatement: VAR IdentifierList Mutability DereferenceMutability Type '=' ExpressionList ';' { $$ = new ast_var_statement_t (@1, $2, $3, $4, $5, $7); }
| VAR IdentifierList Mutability DereferenceMutability Type ';' { $$ = new ast_var_statement_t (@1, $2, $3, $4, $5, new ast_list_expr_t (@1)); }
| VAR IdentifierList Mutability DereferenceMutability '=' ExpressionList ';' { $$ = new ast_var_statement_t (@1, $2, $3, $4, new ast_empty_type_spec_t (@1), $6); }
| IdentifierList Mutability DereferenceMutability SHORT_ASSIGN ExpressionList ';' { $$ = new ast_var_statement_t (@1, $1, $2, $3, new ast_empty_type_spec_t (@1), $5); }

AssignmentStatement: Expression '=' Expression ';' { $$ = new ast_assign_statement_t (@1, $1, $3); } /* CHECK */
| Expression ADD_ASSIGN Expression ';' { $$ = new ast_add_assign_statement_t (@1, $1, $3); } /* CHECK */

IfStatement: IF Expression Block { $$ = new ast_if_statement_t (@1, $2, $3, new ast_list_statement_t (@1)); }
| IF Expression Block ELSE IfStatement { unimplemented; }
| IF Expression Block ELSE Block { $$ = new ast_if_statement_t (@1, $2, $3, $5); }
| IF SimpleStatement ';' Expression Block { unimplemented; }
| IF SimpleStatement ';' Expression Block ELSE IfStatement { unimplemented; }
| IF SimpleStatement ';' Expression Block ELSE Block { unimplemented; }

WhileStatement: FOR Expression Block { $$ = new ast_while_statement_t (@1, $2, $3); }

IdentifierList: IDENTIFIER { $$ = (new ast_identifier_list_t (@1))->Append ($1); }
| IdentifierList ',' IDENTIFIER { $$ = $1->Append ($3); }

// Type literals that can appear in expressions.
TypeLitExpression:
  ArrayType     { $$ = $1; }
| StructType    { $$ = $1; }
//| FunctionType  { $$ = $1; }
//| InterfaceType { $$ = $1; }
| SliceType     { $$ = $1; }
| MapType       { $$ = $1; }
| ComponentType { $$ = $1; }
| PushPortType  { $$ = $1; }
| PullPortType  { $$ = $1; }
| HeapType      { $$ = $1; }

ArrayType:
  ArrayDimension ElementType { $$ = new ast_array_type_spec_t (@1, $1, $2); }

ElementType:
  Type { $$ = $1; }

StructType:
  STRUCT '{' FieldList '}' { $$ = $3; }

SliceType:
  '[' ']' ElementType { $$ = new ast_slice_type_spec_t (@1, $3); }

MapType:
  MAP '[' KeyType ']' ElementType { unimplemented; }

KeyType:
  Type { $$ = $1; }

ComponentType:
  COMPONENT '{' FieldList '}' {
    $$ = $3;
    static_cast<ast_field_list_type_spec_t*> ($3)->IsComponent = true;
  }

PushPortType:
  PUSH Signature { $$ = new ast_push_port_type_spec_t (@1, $2); }

PullPortType:
  PULL Signature DereferenceMutability Type { $$ = new ast_pull_port_type_spec_t (@1, $2, $3, $4); }

HeapType:
  HEAP Type { $$ = new ast_heap_type_spec_t (@1, $2); }

PointerType:
  '*' Type { $$ = new ast_pointer_type_spec_t (@1, $2); }

TypeLit:
  PointerType       { $$ = $1; }
| TypeLitExpression { $$ = $1; }

TypeName:
  IDENTIFIER { $$ = new ast_identifier_type_spec_t (@1, $1); }

Type:
  TypeName     { $$ = $1; }
| TypeLit      { $$ = $1; }
| '(' Type ')' { $$ = $2; }

ArrayDimension: '[' Expression ']' { $$ = $2; }

FieldList: /* empty */ { $$ = new ast_field_list_type_spec_t (yyloc); }
| FieldList IdentifierList Type ';' { $$ = $1->Append (new ast_identifier_list_type_spec_t (@1, $2, MUTABLE, MUTABLE, $3)); }

OptionalExpression:
  /* empty */
{ $$ = new ast_auto_expr_t (yyloc); }
| Expression
{ $$ = $1; }

Expression: OrExpression { $$ = $1; }

OrExpression: AndExpression { $$ = $1; }
| AndExpression LOGIC_OR OrExpression { $$ = new ast_binary_arithmetic_expr_t (@1, LogicOr, $1, $3); }

AndExpression: CompareExpression { $$ = $1; }
| CompareExpression LOGIC_AND AndExpression { $$ = new ast_binary_arithmetic_expr_t (@1, LogicAnd, $1, $3); }

CompareExpression: AddExpression { $$ = $1; }
| AddExpression EQUAL CompareExpression { $$ = new ast_binary_arithmetic_expr_t (@1, Equal, $1, $3); }
| AddExpression NOT_EQUAL CompareExpression { $$ = new ast_binary_arithmetic_expr_t (@1, NotEqual, $1, $3); }
| AddExpression '<' CompareExpression { $$ = new ast_binary_arithmetic_expr_t (@1, LessThan, $1, $3); }
| AddExpression LESS_EQUAL CompareExpression { $$ = new ast_binary_arithmetic_expr_t (@1, LessEqual, $1, $3); }
| AddExpression '>' CompareExpression { $$ = new ast_binary_arithmetic_expr_t (@1, MoreThan, $1, $3); }
| AddExpression MORE_EQUAL CompareExpression { $$ = new ast_binary_arithmetic_expr_t (@1, MoreEqual, $1, $3); }

AddExpression: MultiplyExpression { $$ = $1; }
| MultiplyExpression '+' AddExpression { $$ = new ast_binary_arithmetic_expr_t (@1, Add, $1, $3); }
| MultiplyExpression '-' AddExpression { $$ = new ast_binary_arithmetic_expr_t (@1, Subtract, $1, $3); }
| MultiplyExpression '|' AddExpression { $$ = new ast_binary_arithmetic_expr_t (@1, BitOr, $1, $3); }
| MultiplyExpression '^' AddExpression { $$ = new ast_binary_arithmetic_expr_t (@1, BitXor, $1, $3); }

MultiplyExpression: UnaryExpression { $$ = $1; }
| UnaryExpression '*' MultiplyExpression { $$ = new ast_binary_arithmetic_expr_t (@1, Multiply, $1, $3); }
| UnaryExpression '/' MultiplyExpression { $$ = new ast_binary_arithmetic_expr_t (@1, Divide, $1, $3); }
| UnaryExpression '%' MultiplyExpression { $$ = new ast_binary_arithmetic_expr_t (@1, Modulus, $1, $3); }
| UnaryExpression LEFT_SHIFT MultiplyExpression { $$ = new ast_binary_arithmetic_expr_t (@1, LeftShift, $1, $3); }
| UnaryExpression RIGHT_SHIFT MultiplyExpression { $$ = new ast_binary_arithmetic_expr_t (@1, RightShift, $1, $3); }
| UnaryExpression '&' MultiplyExpression { $$ = new ast_binary_arithmetic_expr_t (@1, BitAnd, $1, $3); }
| UnaryExpression AND_NOT MultiplyExpression { $$ = new ast_binary_arithmetic_expr_t (@1, BitAndNot, $1, $3); }

UnaryExpression: PrimaryExpression { $$ = $1; }
| '+' UnaryExpression { unimplemented; }
| '-' UnaryExpression { $$ = new ast_unary_arithmetic_expr_t (@1, Negate, $2); }
| '!' UnaryExpression { $$ = new ast_unary_arithmetic_expr_t (@1, LogicNot, $2); }
| '^' UnaryExpression { unimplemented; }
| '*' UnaryExpression { $$ = new ast_dereference_expr_t (@1, $2); }
| '&' UnaryExpression { $$ = new ast_address_of_expr_t (@1, $2); }

PrimaryExpression:
  LITERAL { $$ = $1; }
| TypeLitExpression LiteralValue { unimplemented; }
| IDENTIFIER LiteralValue { $$ = new ast_composite_literal_t (@1, new ast_identifier_type_spec_t (@1, $1), $2); }
| '[' DOTDOTDOT ']' ElementType LiteralValue { unimplemented; }
/* | FunctionLit */
| IDENTIFIER { $$ = new ast_identifier_expr_t (@1, $1); }
| '(' Expression ')' { $$ = $2; }
| PrimaryExpression '.' IDENTIFIER { $$ = new ast_select_expr_t (@1, $1, $3); }
| PrimaryExpression '[' Expression ']' { $$ = new ast_index_expr_t (@1, $1, $3); }
| PrimaryExpression '[' OptionalExpression ':' OptionalExpression ']' { $$ = new ast_slice_expr_t (@1, $1, $3, $5, new ast_auto_expr_t (yyloc)); }
| PrimaryExpression '[' OptionalExpression ':' Expression ':' Expression ']' { $$ = new ast_slice_expr_t (@1, $1, $3, $5, $7); }
/* | PrimaryExpression TypeAssertion { unimplemented; } */
| PrimaryExpression '(' OptionalTypeOrExpressionList ')' { $$ = new ast_call_expr_t (@1, $1, $3); }
| TypeLitExpression '(' Expression ')' { $$ = new ast_conversion_expr_t (@1, new TypeExpression (@1, $1), $3); }

LiteralValue:
'{' '}' { $$ = new ast_element_list_t (@1); }
| '{' ElementList OptionalComma '}' { unimplemented; }

ElementList:
  Element { unimplemented; }
| ElementList ',' Element { unimplemented; }

Element:
  Key ':' Value { unimplemented; }
| Value { unimplemented; }

Key:
  Expression { unimplemented; }
| LiteralValue { unimplemented; }

Value:
  Expression { unimplemented; }
| LiteralValue { unimplemented; }

OptionalComma: /* empty */ | ','

%%
