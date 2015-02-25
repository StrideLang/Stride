%{
#include <iostream>

#include <cstdio>  // for fopen
#include <cstdarg>  //for var args

#include "ast.h"

using namespace std;

extern "C" int yylex();
extern "C" int yylineno;
extern "C" char * yytext;
extern "C" FILE *yyin;

void yyerror(const char *s, ...);

AST *tree_head;

AST *parse(const char *filename);

int error = 0;

%}

%code requires { #include "ast.h" }
%code requires { #include "platformnode.h" }
%code requires { #include "blocknode.h" }
%code requires { #include "streamnode.h" }
%code requires { #include "propertynode.h" }
%code requires { #include "bundlenode.h" }
%code requires { #include "valuenode.h" }
%code requires { #include "namenode.h" }
%code requires { #include "functionnode.h" }
%code requires { #include "expressionnode.h" }
%code requires { #include "listnode.h" }

%union {
	int 	ival;
	float 	fval;
        char *	sval;
        AST  *  ast;
        PlatformNode *platformNode;
        BlockNode *blockNode;
        StreamNode *streamNode;
        PropertyNode *propertyNode;
        BundleNode *bundleNode;
        FunctionNode *functionNode;
        ListNode *listNode;
        ExpressionNode *expressionNode;
}

/* declare types for nodes */
%type <platformNode> platformDef
%type <blockNode> blockDef
%type <ast> blockType
%type <ast> properties
%type <propertyNode> property
%type <ast> propertyType
%type <streamNode> streamDef
%type <streamNode> streamType
%type <ast> streamExp
%type <ast> streamComp
%type <ast> valueComp
%type <ast> indexExp
%type <ast> indexComp
%type <bundleNode> bundleDef
%type <bundleNode> bundleRangeDef
%type <functionNode> functionDef
%type <listNode> listDef
%type <listNode> valueListList
%type <listNode> valueList
%type <listNode> valueListDef
%type <listNode> stringList
%type <listNode> switchList
%type <listNode> blockList
%type <listNode> streamList
%type <listNode> listList

%type <ast> valueExp
%type <ast> valueListExp


/* declare tokens */
%token 	<ival> 	INT
%token 	<fval> 	FLOAT
%token	<sval> 	UVAR
%token 	<sval> 	WORD
%token 	<sval> 	STRING
%token 	<sval> 	ERROR

%token '[' ']' '{' '}'
%token COMMA COLON SEMICOLON 
%token USE VERSION NONE ON OFF

%left STREAM
%left OR
%left AND
%left '+' '-' 
%left '*' '/'
%left '(' ')'
%nonassoc NOT
%nonassoc UMINUS

%locations

%%

entry: 
                /*epsilon*/	{}
        | 	entry start	{ cout << endl << "Grabbing Next ..." << endl ; }
	| 	entry SEMICOLON	{ cout << "Ignoring Semicolon!" << endl ; }	
	;

start:
                platformDef	{ tree_head->addChild($1);
                                           cout << "Platform Definition Resolved!" << endl; }
        |	blockDef	{ tree_head->addChild($1);
                                          cout << "Block Resolved!" << endl; }
        |	streamDef	{ tree_head->addChild($1);
                                          cout << "Stream Definition Resolved!" << endl;}
        |	ERROR		{ yyerror("Unrecognised Character: ", $1); }
	;

// ================================= 
//	PLATFORM DEFINITION
// =================================

platformDef:
                USE UVAR VERSION FLOAT {
                        cout << "Platform: " << $2 << endl << "Version: " << $4 << " line " << yylineno << endl;
                        string s;
                        s.append($2); /* string constructor leaks otherwise! */
                        $$ = new PlatformNode(s, $4, yyloc.first_line);
                        free($2);
                }
	;

// ================================= 
//	BLOCK DEFINITION
// =================================
	
blockDef: 	
                WORD UVAR blockType 		{
                                                  string word;
                                                  word.append($1); /* string constructor leaks otherwise! */
                                                  string uvar;
                                                  uvar.append($2); /* string constructor leaks otherwise! */
                                                  $$ = new BlockNode(uvar, word, $3, yyloc.first_line);
                                                  AST *props = $3;
                                                  delete props;
                                                  cout << "Block: " << $1 << ", Labelled: " << $2 << endl;
                                                  free($1);
                                                  free($2);
                                                  }
        |	WORD bundleDef blockType	{
                                                  string s;
                                                  s.append($1); /* string constructor leaks otherwise! */
                                                  $$ = new BlockNode($2, s, $3, yyloc.first_line);
                                                  AST *props = $3;
                                                  delete props;
                                                  cout << "Block Bundle ..." << endl;
                                                  free($1);  }
	;

blockType: 	
                '{' '}'			{ $$ = NULL; cout << "Default values assigned!" << endl; }
        |	'{' properties '}' 	{ $$ = $2; }
	;

// ================================= 
//	STREAM DEFINITION
// =================================
	
streamDef:
                streamType SEMICOLON			{ $$ = $1; }
	;

streamType:
                valueExp STREAM streamExp  	{ $$ = new StreamNode($1, $3, yyloc.first_line);
                                                  cout << "Stream Resolved!" << endl; }
        |	valueListExp STREAM streamExp	{ cout << "Stream Resolved!" << endl; }
	;
	
// ================================= 
//	ARRAY DEFINITION
// =================================

bundleDef:
                UVAR '[' indexExp ']'	{
                                          string s;
                                          s.append($1); /* string constructor leaks otherwise! */
                                          $$ = new BundleNode(s, $3, yyloc.first_line);
                                          cout << "Bundle name: " << $1 << endl;
                                          free($1);}
	;

// =================================
//	ARRAY RANGE ACCESS
// =================================

bundleRangeDef:
                UVAR '[' indexExp COLON indexExp']'     {
                                                          string s;
                                                          s.append($1); /* string constructor leaks otherwise! */
                                                          $$ = new BundleNode(s, $3, $5, yyloc.first_line);
                                                          cout << "Bundle Range name: " << $1 << endl;
                                                          free($1); }
        ;

// ================================= 
//	FUNCTION DEFINITION
// =================================

functionDef:
                WORD '(' ')'		{
                                          string s;
                                          s.append($1); /* string constructor leaks otherwise! */
                                          $$ = new FunctionNode(s, NULL, FunctionNode::BuiltIn, yyloc.first_line);
                                          cout << "Platform function: " << $1 << endl;
                                          free($1); }
        |	WORD '(' properties ')'	{
                                          string s;
                                          s.append($1); /* string constructor leaks otherwise! */
                                          $$ = new FunctionNode(s, $3, FunctionNode::BuiltIn, yyloc.first_line);
                                          cout << "Properties () ..." << endl << "Platform function: " << $1 << endl;
                                          free($1);}
        |	UVAR '(' ')'		{
                                          string s;
                                          s.append($1); /* string constructor leaks otherwise! */
                                          $$ = new FunctionNode(s, NULL, FunctionNode::UserDefined, yyloc.first_line);
                                          cout << "User function: " << $1 << endl;
                                          free($1); }
        |	UVAR '(' properties ')' {
                                          string s;
                                          s.append($1); /* string constructor leaks otherwise! */
                                          $$ = new FunctionNode(s, $3, FunctionNode::UserDefined, yyloc.first_line);
                                          cout << "Properties () ..." << endl << "User function: " << $1 << endl;
                                          free($1); }
	;	
	
// ================================= 
//	PROPERTIES DEFINITION
// =================================
	
properties: 	
                properties property SEMICOLON	{ AST *temp = new AST();
                                                  AST *props = $1;
                                                  props->giveChildren(temp);
                                                  temp->addChild($2);
                                                  $$ = temp;
                                                  props->deleteChildren();
                                                  delete props;
                                                  cout << "Ignoring semicolon!" << endl ; }
        |	properties property		{
                                                  AST *temp = new AST();
                                                  AST *props = $1;
                                                  props->giveChildren(temp);
                                                  temp->addChild($2);
                                                  $$ = temp;
                                                  props->deleteChildren();
                                                  delete props;
                                                  }
        |	property SEMICOLON		{
                                                  AST *temp = new AST();
                                                  temp->addChild($1);
                                                  $$ = temp;
                                                  cout << "Ignoring semicolon!" << endl ; }
        |	property			{
                                                  AST *temp = new AST();
                                                  temp->addChild($1);
                                                  $$ = temp;}
	;
	
property: 	
                WORD COLON propertyType 	{
                                                  string s;
                                                  s.append($1); /* string constructor leaks otherwise! */
                                                  $$ = new PropertyNode(s, $3);
                                                  cout << "Property: " << $1 << endl << "New property ... " << endl;
                                                  free($1);
                                                  }
	;
	
propertyType: 	
                NONE			{ $$ = new ValueNode(yyloc.first_line); cout << "Keyword: none" << endl; }
        |	ON			{ $$ = new ValueNode(true, yyloc.first_line); cout << "Keyword: on" << endl; }
        |	OFF			{ $$ = new ValueNode(false, yyloc.first_line); cout << "Keyword: off" << endl; }
        |	STRING			{
                                          string s;
                                          s.append($1); /* string constructor leaks otherwise! */
                                          $$ = new ValueNode(s, yyloc.first_line); cout << "String: " << $1 << endl; free($1); }
        |	valueExp		{ $$ = $1; cout << "Value expression as property value!" << endl; }
        |	blockType		{ $$ = new BlockNode("", "" , $1, yyloc.first_line);
                                          AST *props = $1;
                                          delete props;
                                          cout << "Block as property value!" << endl; }
        |	streamType		{ $$ = $1; cout << "Stream as property value!" << endl; }
        |	listDef			{ $$ = $1; }
        |	valueListExp	{ $$ = $1; cout << "List expression as property value!" << endl; }
	;

// ================================= 
//	REGULAR LIST DEFINITION
// =================================

listDef:
                '[' stringList ']'	{ $$ = $2; }
        |	'[' switchList ']'	{ $$ = $2; }
        |	'[' blockList  ']'	{ $$ = $2; }
        |	'[' streamList ']'	{ $$ = $2; }
        |	'[' listList   ']'	{ $$ = $2; }
	;

stringList:
                stringList COMMA STRING		{
                                                  ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                  list->stealMembers($1);
                                                  ListNode *oldList = $1;
                                                  oldList->deleteChildren();
                                                  delete oldList;
                                                  string s;
                                                  s.append($3);  /* string constructor leaks otherwise! */
                                                  list->addChild(new ValueNode(s, yyloc.first_line));
                                                  $$ = list;
                                                  cout << "String: " << $3 << endl << "New list item ... " << endl; free($3); }
        |	STRING				{
                                                  string s;
                                                  s.append($1);
                                                  $$ = new ListNode(new ValueNode(s, yyloc.first_line), yyloc.first_line);
                                                  cout << "String: " << $1 << endl << "New list item ... " << endl; free($1); }
	;

switchList:
                switchList COMMA ON	{
                                          ListNode *list = new ListNode(NULL, yyloc.first_line);
                                          list->stealMembers($1);
                                          ListNode *oldList = $1;
                                          oldList->deleteChildren();
                                          delete oldList;
                                          list->addChild(new ValueNode(true, yyloc.first_line));
                                          $$ = list;
                                          cout << "switch: ON" << endl << "New list item ... " << endl; }
        |	switchList COMMA OFF	{
                                          ListNode *list = new ListNode(NULL, yyloc.first_line);
                                          list->stealMembers($1);
                                          ListNode *oldList = $1;
                                          oldList->deleteChildren();
                                          delete oldList;
                                          list->addChild(new ValueNode(false, yyloc.first_line));
                                          $$ = list;
                                          cout << "switch: OFF" << endl << "New list item ... " << endl; }
        |	ON			{ $$ = new ListNode(new ValueNode(true, yyloc.first_line), yyloc.first_line); cout << "switch: ON" << endl << "New list item ... " << endl; }
        |	OFF			{ $$ = new ListNode(new ValueNode(false, yyloc.first_line), yyloc.first_line); cout << "switch: OFF" << endl << "New list item ... " << endl; }
	;
	
blockList:
                blockList COMMA blockDef	{
                                                  ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                  list->stealMembers($1);
                                                  ListNode *oldList = $1;
                                                  oldList->deleteChildren();
                                                  delete oldList;
                                                  list->addChild($3);
                                                  $$ = list;
                                                  cout << "Block definition ... " << endl << "New list item ... " << endl; }
        |	blockDef			{ $$ = new ListNode($1, yyloc.first_line); cout << "Block definition ... " << endl << "New list item ... " << endl; }
	;
	
streamList:
                streamList COMMA streamType	{
                                                  ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                  list->stealMembers($1);
                                                  ListNode *oldList = $1;
                                                  oldList->deleteChildren();
                                                  delete oldList;
                                                  list->addChild($3);
                                                  $$ = list;
                                                  cout << "Stream definition ... " << endl << "New list item ... " << endl; }
        |	streamType			{ $$ = new ListNode($1, yyloc.first_line); cout << "Stream definition ... " << endl << "New list item ... " << endl; }
	;
	
listList:
                listList COMMA listDef		{
                                                  ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                  list->stealMembers($1);
                                                  ListNode *oldList = $1;
                                                  oldList->deleteChildren();
                                                  delete oldList;
                                                  list->addChild($3);
                                                  $$ = list;
                                                  cout << "List of lists ..." << endl << "New list item ... " << endl; }
        |	listDef				{
                                                  $$ = new ListNode($1, yyloc.first_line); cout << "List of lists ..." << endl << "New list item ... " << endl; }
	;
	
// ================================= 
//	VALUE LIST DEFINITION
// =================================
	
valueListDef:
                '[' valueList ']'	{ $$ = $2; }
        |	'[' valueListList ']'	{ $$ = $2; }
	;
	
valueList:
                valueList COMMA valueExp	{
                                                  ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                  list->stealMembers($1);
                                                  ListNode *oldList = $1;
                                                  oldList->deleteChildren();
                                                  delete oldList;
                                                  list->addChild($3);
                                                  $$ = list;
                                                  cout << "Value expression ..." << endl << "New list item ... " << endl; }
        |	valueExp			{
                                                  $$ = new ListNode($1, yyloc.first_line);
                                                  cout << "Value expression ..." << endl << "New list item ... " << endl; }
	;	

valueListList:
                valueListList COMMA valueListDef	{
                                                          ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                          list->stealMembers($1);
                                                          ListNode *oldList = $1;
                                                          oldList->deleteChildren();
                                                          delete oldList;
                                                          list->addChild($3);
                                                          $$ = list;
                                                          cout << "List of lists ..." << endl << "New list item ... " << endl; }
        |	valueListDef				{ $$ = new ListNode($1, yyloc.first_line);  cout << "List of lists ..." << endl << "New list item ... " << endl; }
	;

// ================================= 
//	INDEX EXPRESSION
// =================================
	
indexExp:
                indexExp '+' indexExp 	{
                                          $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, yyloc.first_line);
                                          cout << "Index/Size adding ... " << endl; }
        |	indexExp '-' indexExp 	{
                                          $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                          cout << "Index/Size subtracting ... " << endl; }
        |	indexExp '*' indexExp 	{
                                          $$ = new ExpressionNode(ExpressionNode::Multiply , $1, $3, yyloc.first_line);
                                          cout << "Index/Size multiplying ... " << endl; }
        |	indexExp '/' indexExp 	{
                                          $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                          cout << "Index/Size dividing ... " << endl; }
        |	'(' indexExp ')' 	{ $$ = $2; cout << "Index/Size enclosure ..." << endl; }
        |	indexComp		{ $$ = $1; }
	;

// ================================= 
//	VALUE LIST EXPRESSION
// =================================

valueListExp:
                valueListDef '+' valueExp	{
                                                  $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, yyloc.first_line);
                                                  cout << "Adding ... " << endl; }
        |	valueListDef '-' valueExp	{
                                                  $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                                  cout << "Subtracting ... " << endl; }
        |	valueListDef '*' valueExp	{
                                                  $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, yyloc.first_line);
                                                  cout << "Multiplying ... " << endl; }
        |	valueListDef '/' valueExp	{
                                                  $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                                  cout << "Dividing ... " << endl; }
        |	valueExp '+' valueListDef	{
                                                  $$ = new ExpressionNode(ExpressionNode::Add , $1, $3, yyloc.first_line);
                                                  cout << "Adding ... " << endl; }
        |	valueExp '-' valueListDef	{
                                                  $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                                  cout << "Subtracting ... " << endl; }
        |	valueExp '*' valueListDef	{
                                                  $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, yyloc.first_line);
                                                  cout << "Multiplying ... " << endl; }
        |	valueExp '/' valueListDef	{
                                                  $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                                  cout << "Dividing ... " << endl; }
        |	valueListDef			{ $$ = $1; }
	;

// ================================= 
//	VALUE EXPRESSION
// =================================
	
valueExp:
                valueExp '+' valueExp 		{
                                                  $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, yyloc.first_line);
                                                  cout << "Adding ... " << endl; }
        |	valueExp '-' valueExp 		{
                                                  $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                                  cout << "Subtracting ... " << endl; }
        |	valueExp '*' valueExp 		{
                                                  $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, yyloc.first_line);
                                                  cout << "Multiplying ... " << endl; }
        |	valueExp '/' valueExp 		{
                                                  $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                                  cout << "Dividing ... " << endl; }
        |	valueExp AND valueExp 		{
                                                  $$ = new ExpressionNode(ExpressionNode::And, $1, $3, yyloc.first_line);
                                                  cout << "Logical AND ... " << endl; }
        |	valueExp OR valueExp 		{
                                                  $$ = new ExpressionNode(ExpressionNode::Or, $1, $3, yyloc.first_line);
                                                  cout << "Logical OR ... " << endl; }
        |	'(' valueExp ')' 		{ $$ = $2; cout << "Enclosure ..." << endl; }
        | 	'-' valueExp %prec UMINUS 	{
                                                  $$ = new ExpressionNode(ExpressionNode::UnaryMinus, $2, yyloc.first_line);
                                                  cout << "Unary minus ... " << endl; }
        | 	NOT valueExp %prec NOT 		{
                                                  $$ = new ExpressionNode(ExpressionNode::UnaryMinus, $2, yyloc.first_line);
                                                  cout << "Logical NOT ... " << endl; }
        | 	valueComp			{ $$ = $1; }
	;

// ================================= 
//	STREAM EXPRESSION
// =================================
	
streamExp:
                streamComp STREAM streamExp	{ $$ = new StreamNode($1, $3, yyloc.first_line); }
        |	streamComp			{ $$ = $1; }
	;

// ================================= 
//	INDEX COMPONENTS
// =================================
	
indexComp:
                INT		{ $$ = new ValueNode($1, yyloc.first_line);
                                  cout << "Index/Size Integer: " << $1 << endl; }
        |	UVAR		{
                                  string s;
                                  s.append($1); /* string constructor leaks otherwise! */
                                  $$ = new NameNode(s, yyloc.first_line);
                                  cout << "Index/Size User variable: " << $1 << endl; free($1); }
        |	bundleDef	{ cout << "Resolving indexed array ..." << endl; }
	;

// ================================= 
//	STREAM COMPONENTS
// =================================
	
streamComp:
                UVAR		{
                                  string s;
                                  s.append($1); /* string constructor leaks otherwise! */
                                  $$ = new NameNode(s, yyloc.first_line);
                                  cout << "User variable: " << $1 << endl << "Streaming ... " << endl;
                                  free($1); }
        |	bundleDef	{ cout << "Resolving indexed array ..." << endl << "Streaming ... " << endl; }
        |       bundleRangeDef  { cout << "Resolving indexed array range ..." << endl << "Streaming ... " << endl; }
        |	functionDef	{ cout << "Resolving function definition ... " << endl << "Streaming ... " << endl; }
	|	valueListDef	{ cout << "Resolving list definition ... " << endl << "Streaming ... " << endl;}
	;

// ================================= 
//	VALUE COMPONENTS
// =================================

valueComp:
                INT	{ $$ = new ValueNode($1, yyloc.first_line);
                          cout << "Integer: " << $1 << endl; }
        |	FLOAT	{ $$ = new ValueNode($1, yyloc.first_line);
                          cout << "Real: " << $1 << endl; }
        |	UVAR	{
                          string s;
                          s.append($1); /* string constructor leaks otherwise! */
                          $$ = new NameNode(s, yyloc.first_line);
                          cout << "User variable: " << $1 << endl;
                          free($1);
                          }
        |	bundleDef	{ $$ = $1;
                                  cout << "Resolving indexed array ..." << endl; }
        |       bundleRangeDef  { cout << "Resolving indexed array range ..." << endl; }
        |	functionDef	{ cout << "Resolving function definition ..." << endl; }
	;
	
%%

void yyerror(const char *s, ...){
	va_list ap;
	va_start(ap, s);
        cout << endl << endl << "ERROR: " << s << " => " << va_arg(ap, char*) << endl;
        cout << "Unexpected token: \"" << yytext << "\" on line: " <<  yylineno << endl;
	va_end(ap);
	error++;
}

AST *parse(const char *filename){
	FILE * file;
        AST *ast = NULL;
        char const * fileName = filename;

	file = fopen(fileName, "r");

	if (!file){
		cout << "Can't open " << fileName << endl;;
                return NULL;
	}
	
	cout << "Analysing: " << fileName << endl;
	cout << "===========" << endl;

        tree_head = new AST;
	yyin = file;
        yylineno = 1;
	yyparse();
	
	if (error > 0){
                cout << endl << "Number of Errors: " << error << endl;
                tree_head->deleteChildren();
                delete tree_head;
                return ast;
        }
        ast = tree_head;
	cout << "Completed Analysing: " << fileName << endl;
        return ast;
}

