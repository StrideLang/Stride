%{
#include <iostream>
#include <stdio.h>
#include <stdarg.h>

using namespace std;

extern "C" int yylex();
extern "C" FILE *yyin;

extern int yylineno;
void yyerror(char *s, ...);

int parse(const char *filename);

int error = 0;

%}

%union {
	int 	ival;
	float 	fval;
	char *	sval;
}

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
		platformDef		{ cout << "Platform Definition Resolved!" << endl; }
	|	blockDef		{ cout << "Block Resolved!" << endl; }
	|	streamDef		{}
	|	ERROR			{ yyerror("Unrecognised Character: ", $1) }
	;

// ================================= 
//	PLATFORM DEFINITION
// =================================
	
platformDef:
		USE UVAR VERSION FLOAT { cout << "Platform: " << $2 << endl << "Version: " << $4 << endl; }
	;

// ================================= 
//	BLOCK DEFINITION
// =================================
	
blockDef: 	
		WORD UVAR blockType 		{ cout << "Block: " << $1 << ", Labelled: " << $2 << endl; }
	|	WORD bundleDef blockType		{ cout << "Block Bundle ..." << endl;  }
	;

blockType: 	
		'{' '}'				{ cout << "Default values assigned!" << endl; }
	|	'{' properties '}' 	{}
	;

// ================================= 
//	STREAM DEFINITION
// =================================
	
streamDef:
		streamType SEMICOLON			{}
	;

streamType:
		valueExp STREAM streamExp  		{ cout << "Stream Resolved!" << endl; }
	|	valueListExp STREAM streamExp	{ cout << "Stream Resolved!" << endl; }
	;
	
// ================================= 
//	ARRAY DEFINITION
// =================================

bundleDef:
		UVAR '[' indexExp ']'	{ cout << "Bundle name: " << $1 << endl; }
	;

// ================================= 
//	FUNCTION DEFINITION
// =================================

functionDef:
		WORD '(' ')'				{ cout << "Platform function: " << $1 << endl; }
	|	WORD '(' properties ')'		{ cout << "Properties () ..." << endl << "Platform function: " << $1 << endl; }
	|	UVAR '(' ')'				{ cout << "User function: " << $1 << endl; }
	|	UVAR '(' properties ')' 	{ cout << "Properties () ..." << endl << "User function: " << $1 << endl; }
	;	
	
// ================================= 
//	PROPERTIES DEFINITION
// =================================
	
properties: 	
		properties property				{}
	|	properties SEMICOLON property	{ cout << "Ignoring semicolon!" << endl ; }
	|	property						{}
	;
	
property: 	
		WORD COLON propertyType 	{ cout << "Property: " << $1 << endl << "New property ... " << endl; }
	;
	
propertyType: 	
		NONE			{ cout << "Keyword: none" << endl; }
	|	ON				{ cout << "Keyword: on" << endl; }
	|	OFF				{ cout << "Keyword: off" << endl; }
	|	STRING			{ cout << "String: " << $1 << endl; }
	|	valueExp		{ cout << "Value expression as property value!" << endl; }
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
	|	indexComp				{}
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
	|	valueListDef				{ }
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
	|	'(' valueExp ')' 			{ cout << "Enclosure ..." << endl; }
	| 	'-' valueExp %prec UMINUS 	{ cout << "Unary minus ... " << endl; }
	| 	NOT valueExp %prec NOT 		{ cout << "Logical NOT ... " << endl; }
	| 	valueComp					{}
	;

// ================================= 
//	STREAM EXPRESSION
// =================================
	
streamExp:
		streamComp STREAM streamExp	{}
	|	streamComp					{}
	;

// ================================= 
//	INDEX COMPONENTS
// =================================
	
indexComp:
		INT				{ cout << "Index/Size Integer: " << $1 << endl; }
	|	UVAR			{ cout << "Index/Size User variable: " << $1 << endl; }
	|	bundleDef		{ cout << "Resolving indexed array ..." << endl; }
	;

// ================================= 
//	STREAM COMPONENTS
// =================================
	
streamComp:
		UVAR			{ cout << "User variable: " << $1 << endl << "Streaming ... " << endl; }
	|	bundleDef		{ cout << "Resolving indexed array ..." << endl << "Streaming ... " << endl; }
	|	functionDef		{ cout << "Resolving function definition ... " << endl << "Streaming ... " << endl; }
	|	valueListDef	{ cout << "Resolving list definition ... " << endl << "Streaming ... " << endl;}
	;

// ================================= 
//	VALUE COMPONENTS
// =================================

valueComp:
		INT				{ cout << "Integer: " << $1 << endl; } 
	|	FLOAT			{ cout << "Real: " << $1 << endl; }
	|	UVAR			{ cout << "User variable: " << $1 << endl; }
	|	bundleDef		{ cout << "Resolving indexed array ..." << endl; }
	|	functionDef		{ cout << "Resolving function definition ..." << endl; }
	;
	
%%
void yyerror(char *s, ...){
	va_list ap;
	va_start(ap, s);
	cout << "ERROR: " << s << " => " << va_arg(ap, char*) << " @ line " <<  yylineno << endl;
	va_end(ap);
	error++;
}

int parse(const char *filename){
	FILE * file;
        char const * fileName = filename;

	file = fopen(fileName, "r");

	if (!file){
		cout << "Can't open " << fileName << endl;;
		return -1;
	}
	
	cout << "Analysing: " << fileName << endl;
	cout << "===========" << endl;

	yyin = file;
	yyparse();
	
	if (error > 0){
		cout << endl << "Number of Errors: " << error << endl; 
	}
	
	cout << "Completed Analysing: " << fileName << endl;
	return 0;
}

