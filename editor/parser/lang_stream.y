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
%left 	<sval> 	UVAR
%token 	<sval> 	WORD
%token 	<sval> 	STRING
%token 	<sval> 	ERROR

%token '{' '}' '[' ']'
%token COMMA COLON SEMICOLON 
%token USE NONE ON OFF 

%left STREAM
%left '+' '-' 
%left '*' '/'
%left '(' ')'
%nonassoc UMINUS

%%

entry: 
		/*epsilon*/		{}
	| 	entry start		{ cout << endl << "Grabbing Next ..." << endl ; }
	| 	entry SEMICOLON	{ cout << "Ignore SEMICOLON!" << endl ; }	
	;

start:
		blockDef		{ cout << "Block Resolved!" << endl; }
	|	streamDef		{ cout << "Stream Resolved!" << endl; }
	|	signalDef		{ cout << "Signal Chain Resolved!" << endl; }
        |	ERROR			{ yyerror("Unrecognised Character: ", $1); }
	;
	
blockDef: 	
		WORD UVAR blockBody 			{ cout << "Block: " << $1 << ", Called: " << $2 << endl; }
	|	WORD UVAR arrayDef blockBody 	{ cout << "Block: " << $1 << ", Called: " << $2 << endl; }
	;

blockBody: 	
		'{' '}'					{ cout << "Default values assigned!" << endl; }
	|	'{' propertyList '}' 	{}
	;

signalDef:
		UVAR STREAM UVAR					{ cout << $1 << " -> " << $3 << endl; }			
	|	UVAR STREAM streamDef				{ cout << $1 << " -> Stream" << endl; }	
	|	UVAR STREAM streamDef STREAM UVAR	{ cout << $1 << " -> Stream -> " << $5 << endl; }	
	|	streamDef STREAM UVAR				{ cout << "Stream -> " << $3 << endl; }
	;
	
streamDef:
		functionDef						{}
	|	streamDef STREAM functionDef	{ cout << "STREAMing" << endl; }
	;
	
functionDef: 	
		WORD '(' ')'					{ cout << "Blank Platform Function: " << $1 << endl; }
	|	WORD '(' propertyList ')'		{ cout << "Properties () ..." << endl << "Platform Function: " << $1 << endl; }
	|	UVAR '(' ')'					{ cout << "Blank User Function: " << $1 << endl; }
	|	UVAR '(' propertyList ')' 		{ cout << "Properties () ..." << endl << "User Function: " << $1 << endl; }
	|	functionDef '+' functionDef		{ cout << "ADDing" << endl; }
	|	functionDef '-' functionDef		{ cout << "SUBing" << endl; }
	|	functionDef '*' functionDef		{ cout << "MULing" << endl; }
	|	functionDef '/' functionDef		{ cout << "DIVing" << endl; }
	|	'(' functionDef ')'				{}
	;

propertyList: 	
		propertyList property	{ cout << "Property Resolved!" << endl; }
	|	property				{ cout << "Property Resolved!" << endl; }
	;
	
property: 	
		WORD COLON propertyType 	{ cout << "Property: " << $1 << endl; }
	|	UVAR COLON propertyType 	{ cout << "Property: " << $1 << endl; }
	;

arrayDef:
		'[' INT ']'		{ cout << "Elements :" << $2 << endl; }
	;

listDef:
		'[' listType ']'	{}
	;
	
listType:
		integerList			{}	
	|	floatList 			{}
	|	stringList			{}
	|	blockList			{}
	|	variableList		{}
	| 	streamList			{}
	|	listList			{}
	;
	
integerList:
		INT							{ cout << "Integer: " << $1 << endl; }		
	|	integerList COMMA INT		{ cout << "Integer: " << $3 << endl; }	
	;

floatList:
		FLOAT						{ cout << "Float: " << $1 << endl; }	
	|	floatList COMMA FLOAT		{ cout << "Float: " << $3 << endl; }
	;

stringList:
		STRING						{ cout << "String: " << $1 << endl; }	
	|	stringList COMMA STRING		{ cout << "String: " << $3 << endl; }
	;

variableList:
		UVAR						{ cout << "Variable: " << $1 << endl; }	
	|	variableList COMMA UVAR		{ cout << "Variable: " << $3 << endl; }
	
blockList:
		blockDef					{ cout << "Block Definition... " << endl; }	
	|	blockList COMMA blockDef	{ cout << "Block Definition... " << endl; }
	;
	
streamList:
		streamDef					{ cout << "Stream Definition... " << endl; }	
	|	streamList COMMA streamDef	{ cout << "stream Definition... " << endl; }

listList:
		listDef					{ cout << "SubList...:" << endl; }	
	|	listList COMMA listDef	{ cout << "SubList...:" << endl;}
	;
	
valueDef:
		valueDef '+' valueDef 		{ cout << "Adding... " << endl; }
	|	valueDef '-' valueDef 		{ cout << "Subtracting... " << endl; }
	|	valueDef '*' valueDef 		{ cout << "Multiplying... " << endl; }
	|	valueDef '/' valueDef 		{ cout << "Dividing... " << endl; }
	|	'(' valueDef ')' 			{ cout << "Enclosure..." << endl; }
	| 	'-' valueDef %prec UMINUS 	{ cout << "UMINUS" << endl; }
	| 	INT							{ cout << $1 << endl; }
	|	FLOAT						{ cout << $1 << endl; }
	|	UVAR						{ cout << $1 << endl; }
	;
	
propertyType: 	
		NONE termination			{ cout << "Keyword: none" << endl; }
	|	ON termination				{ cout << "Keyword: on" << endl; }
	|	OFF termination				{ cout << "Keyword: off" << endl; }
	|	STRING termination			{ cout << $1 << endl; }
	|	valueDef termination		{ cout << "Value Expression as Property Values!" << endl; }
	|	UVAR arrayDef termination	{ cout << "Array: " << $1 << endl; }
	|	blockBody termination		{ cout << "Default Values for Block!" << endl; }
	|	streamDef termination		{ cout << "Stream as Property Value!" << endl; }
	|	listDef termination			{ cout << "List as Property Value!" << endl; }
	;

termination:
		/*epsilon*/					{}
	|	SEMICOLON					{}
	;
	
%%
void yyerror(char *s, ...){
	va_list ap;
	va_start(ap, s);
	cout << "ERROR: " << s << va_arg(ap, char*) << " @ line " <<  yylineno << endl;
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

