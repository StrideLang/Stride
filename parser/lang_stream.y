%{
#include <iostream>

#include <cstdio>  // for fopen
#include <cstdarg>  //for var args

#include "ast.h"

using namespace std;

extern "C" int yylex();
extern "C" FILE *yyin;

extern int yylineno;
void yyerror(const char *s, ...);

AST *tree_head;

AST *parse(const char *filename);

int error = 0;

%}
%code requires { #include "ast.h" }
%code requires { #include "platformnode.h" }
%code requires { #include "objectnode.h" }
%code requires { #include "streamnode.h" }
%code requires { #include "propertynode.h" }
%code requires { #include "bundlenode.h" }
%code requires { #include "valuenode.h" }
%code requires { #include "namenode.h" }

%union {
	int 	ival;
	float 	fval;
	char *	sval;
        void *   aval;
        AST  *  ast;
        PlatformNode *platformNode;
        ObjectNode *objectNode;
        StreamNode *streamNode;
        PropertyNode *propertyNode;
        BundleNode *bundleNode;
}

/* declare types for nodes */
%type <platformNode> platformDef
%type <objectNode> blockDef
%type <ast> blockType
%type <ast> properties
%type <propertyNode> property
%type <ast> propertyType
%type <streamNode> streamDef
%type <streamNode> streamType
%type <ast> streamExp
%type <ast> streamComp
%type <ast> valueExp
%type <ast> valueComp
%type <ast> indexExp
%type <ast> indexComp
%type <bundleNode> bundleDef


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

%%

entry: 
		/*epsilon*/		{}
        | 	entry start		{ cout << endl << "Grabbing Next ..." << endl ; }
	| 	entry SEMICOLON	{ cout << "Ignoring Semicolon!" << endl ; }	
	;

start:
                platformDef		{  tree_head->addChild($1);
                                           cout << "Platform Definition Resolved!" << endl; }
        |	blockDef		{ tree_head->addChild($1);
                                          cout << "Block Resolved!" << endl; }
        |	streamDef		{ tree_head->addChild($1);
                                          cout << "Stream Definition Resolved!" << endl;}
	|	ERROR			{ yyerror("Unrecognised Character: ", $1); }
	;

// ================================= 
//	PLATFORM DEFINITION
// =================================

platformDef:
                USE UVAR VERSION FLOAT {
                cout << "Platform: " << $2 << endl << "Version: " << $4 << endl;
                $$ = new PlatformNode(string($2), $4);
                }
	;

// ================================= 
//	BLOCK DEFINITION
// =================================
	
blockDef: 	
                WORD UVAR blockType 		{
                                                  $$ = new ObjectNode($2, $1, $3);
                                                  // AST *props = $3;
                                                  // delete props;  // FIXME this leaks!
                                                  cout << "Block: " << $1 << ", Labelled: " << $2 << endl; }
	|	WORD bundleDef blockType	{ cout << "Block Bundle ..." << endl;  }
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
                valueExp STREAM streamExp  	{ $$ = new StreamNode( $1, $3);
                                                  cout << "Stream Resolved!" << endl; }
        |	valueListExp STREAM streamExp	{ cout << "Stream Resolved!" << endl; }
	;
	
// ================================= 
//	ARRAY DEFINITION
// =================================

bundleDef:
                UVAR '[' indexExp ']'	{ $$ = new BundleNode($1, $3);
                                          cout << "Bundle name: " << $1 << endl; }
	;

// ================================= 
//	FUNCTION DEFINITION
// =================================

functionDef:
                WORD '(' ')'			{ cout << "Platform function: " << $1 << endl; }
	|	WORD '(' properties ')'		{ cout << "Properties () ..." << endl << "Platform function: " << $1 << endl; }
        |	UVAR '(' ')'			{ cout << "User function: " << $1 << endl; }
	|	UVAR '(' properties ')' 	{ cout << "Properties () ..." << endl << "User function: " << $1 << endl; }
	;	
	
// ================================= 
//	PROPERTIES DEFINITION
// =================================
	
properties: 	
                properties property SEMICOLON	{ AST *temp = new AST();
                                                  AST *props = $1;
                                                  props->pushParent(temp);
                                                  temp->addChild($2);
                                                  $$ = temp;
                                                  // delete props;  // FIXME this leaks!
                                                  cout << "Ignoring semicolon!" << endl ; }
        |	properties property		{
                                                  AST *temp = new AST();
                                                  AST *props = $1;
                                                  props->pushParent(temp);
                                                  temp->addChild($2);
                                                  $$ = temp;
                                                  // delete props;  // FIXME this leaks!
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
                WORD COLON propertyType 	{ $$ = new PropertyNode($1, $3);
                                                  cout << "Property: " << $1 << endl << "New property ... " << endl; }
	;
	
propertyType: 	
		NONE			{ cout << "Keyword: none" << endl; }
	|	ON				{ cout << "Keyword: on" << endl; }
	|	OFF				{ cout << "Keyword: off" << endl; }
        |	STRING			{ $$ = new ValueNode($1); cout << "String: " << $1 << endl; }
        |	valueExp		{ $$ = $1; cout << "Value expression as property value!" << endl; }
	|	blockType		{ cout << "Block as property value!" << endl; }
	|	streamType		{ cout << "Stream as property value!" << endl; }
	|	listDef			{}
	|	valueListExp	{ cout << "List expression as property value!" << endl; }
	;

// ================================= 
//	REGULAR LIST DEFINITION
// =================================

listDef:
		'[' stringList ']'	{}
	|	'[' switchList ']'	{}
	|	'[' blockList  ']'	{}
	|	'[' streamList ']'	{}
	|	'[' listList   ']'	{}
	;

stringList:
		stringList COMMA STRING		{ cout << "String: " << $3 << endl << "New list item ... " << endl; }
	|	STRING						{ cout << "String: " << $1 << endl << "New list item ... " << endl; }
	;

switchList:
		switchList COMMA ON		{ cout << "switch: ON" << endl << "New list item ... " << endl; }
	|	switchList COMMA OFF	{ cout << "switch: OFF" << endl << "New list item ... " << endl; }
	|	ON						{ cout << "switch: ON" << endl << "New list item ... " << endl; }
	|	OFF						{ cout << "switch: OFF" << endl << "New list item ... " << endl; }
	;
	
blockList:
		blockList COMMA blockDef	{ cout << "Block definition ... " << endl << "New list item ... " << endl; }
	|	blockDef					{ cout << "Block definition ... " << endl << "New list item ... " << endl; }
	;
	
streamList:
		streamList COMMA streamType	{ cout << "Stream definition ... " << endl << "New list item ... " << endl; }
	|	streamType					{ cout << "Stream definition ... " << endl << "New list item ... " << endl; }
	;
	
listList:
		listList COMMA listDef		{ cout << "List of lists ..." << endl << "New list item ... " << endl; }
	|	listDef						{ cout << "List of lists ..." << endl << "New list item ... " << endl; }
	;
	
// ================================= 
//	VALUE LIST DEFINITION
// =================================
	
valueListDef:
		'[' valueList ']'		{}
	|	'[' valueListList ']'	{}	
	;
	
valueList:
		valueList COMMA valueExp	{ cout << "Value expression ..." << endl << "New list item ... " << endl; }
	|	valueExp					{ cout << "Value expression ..." << endl << "New list item ... " << endl; }
	;	

valueListList:
		valueListList COMMA valueListDef	{ cout << "List of lists ..." << endl << "New list item ... " << endl; }
	|	valueListDef						{ cout << "List of lists ..." << endl << "New list item ... " << endl; }
	;

// ================================= 
//	INDEX EXPRESSION
// =================================
	
indexExp:
		indexExp '+' indexExp 	{ cout << "Index/Size adding ... " << endl; }
	|	indexExp '-' indexExp 	{ cout << "Index/Size subtracting ... " << endl; }
	|	indexExp '*' indexExp 	{ cout << "Index/Size multiplying ... " << endl; }
	|	indexExp '/' indexExp 	{ cout << "Index/Size dividing ... " << endl; }
	|	'(' indexExp ')' 		{ cout << "Index/Size enclosure ..." << endl; }
        |	indexComp		{  $$ = $1; }
	;

// ================================= 
//	VALUE LIST EXPRESSION
// =================================

valueListExp:
		valueListDef '+' valueExp	{ cout << "Adding ... " << endl; }
	|	valueListDef '-' valueExp	{ cout << "Subtracting ... " << endl; }
	|	valueListDef '*' valueExp	{ cout << "Multiplying ... " << endl; }
	|	valueListDef '/' valueExp	{ cout << "Dividing ... " << endl; }
	|	valueExp '+' valueListDef	{ cout << "Adding ... " << endl; }
	|	valueExp '-' valueListDef	{ cout << "Subtracting ... " << endl; }
	|	valueExp '*' valueListDef	{ cout << "Multiplying ... " << endl; }
	|	valueExp '/' valueListDef	{ cout << "Dividing ... " << endl; }
	|	valueListDef				{}
	;

// ================================= 
//	VALUE EXPRESSION
// =================================
	
valueExp:
		valueExp '+' valueExp 		{ cout << "Adding ... " << endl; }
	|	valueExp '-' valueExp 		{ cout << "Subtracting ... " << endl; }
	|	valueExp '*' valueExp 		{ cout << "Multiplying ... " << endl; }
	|	valueExp '/' valueExp 		{ cout << "Dividing ... " << endl; }
	|	valueExp AND valueExp 		{ cout << "Logical AND ... " << endl; }
	|	valueExp OR valueExp 		{ cout << "Logical OR ... " << endl; }
        |	'(' valueExp ')' 		{ cout << "Enclosure ..." << endl; }
	| 	'-' valueExp %prec UMINUS 	{ cout << "Unary minus ... " << endl; }
	| 	NOT valueExp %prec NOT 		{ cout << "Logical NOT ... " << endl; }
        | 	valueComp			{ $$ = $1; }
	;

// ================================= 
//	STREAM EXPRESSION
// =================================
	
streamExp:
                streamComp STREAM streamExp	{ $$ = new StreamNode($1, $3); }
        |	streamComp			{ $$ = $1; }
	;

// ================================= 
//	INDEX COMPONENTS
// =================================
	
indexComp:
                INT		{ $$ = new ValueNode($1);
                                  cout << "Index/Size Integer: " << $1 << endl; }
        |	UVAR		{ cout << "Index/Size User variable: " << $1 << endl; }
        |	bundleDef	{ cout << "Resolving indexed array ..." << endl; }
	;

// ================================= 
//	STREAM COMPONENTS
// =================================
	
streamComp:
                UVAR		{ cout << "User variable: " << $1 << endl << "Streaming ... " << endl; }
        |	bundleDef	{ $$ = $1;
                                  cout << "Resolving indexed array ..." << endl << "Streaming ... " << endl; }
        |	functionDef	{ cout << "Resolving function definition ... " << endl << "Streaming ... " << endl; }
	|	valueListDef	{ cout << "Resolving list definition ... " << endl << "Streaming ... " << endl;}
	;

// ================================= 
//	VALUE COMPONENTS
// =================================

valueComp:
                INT	{ $$ = new ValueNode($1);
                          cout << "Integer: " << $1 << endl; }
        |	FLOAT	{ $$ = new ValueNode($1);
                          cout << "Real: " << $1 << endl; }
        |	UVAR	{ $$ = new NameNode($1);
                          cout << "User variable: " << $1 << endl; }
        |	bundleDef	{ $$ = $1;
                                  cout << "Resolving indexed array ..." << endl; }
        |	functionDef	{ cout << "Resolving function definition ..." << endl; }
	;
	
%%
void yyerror(const char *s, ...){
	va_list ap;
	va_start(ap, s);
	cout << "ERROR: " << s << " => " << va_arg(ap, char*) << " @ line " <<  yylineno << endl;
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
	yyparse();
	
	if (error > 0){
                cout << endl << "Number of Errors: " << error << endl;
                //TODO: free AST if error
                return ast;
        }
        ast = tree_head;
	cout << "Completed Analysing: " << fileName << endl;
        return ast;
}

