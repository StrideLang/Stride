%{
#include <iostream>
#include <stdio.h>
#include <stdarg.h>

using namespace std;

extern "C" int yylex();
extern "C" FILE *yyin;

extern int yylineno;
void yyerror(char *s, ...);

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
%token USE VERSION NONE ON OFF 

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
		platformDef		{ cout << "Platform Definition Resolved!" << endl; }
	|	blockDef		{ cout << "Block Resolved!" << endl; }
	|	streamDef		{ cout << "Stream Resolved!" << endl; }
	|	signalDef		{ cout << "Signal Chain Resolved!" << endl; }
	|	ERROR			{ yyerror("Unrecognised Character: ", $1) }
	;

// ================================= 
//	PLATFORM DEFINITION
// =================================
	
platformDef:
		USE UVAR VERSION FLOAT { cout << "Platform: " << $2 << endl << "Version: " << $4 << endl; }
	;

// ================================= 
//	BLOCK & BLOCK BUNDLE DEFINITION
// =================================
	
blockDef: 	
		WORD UVAR blockBody 			{ cout << "Block: " << $1 << ", Called: " << $2 << endl; }
	|	WORD UVAR arrayDef blockBody	{ cout << "Block Bundle: " << $1 << ", Called: " << $2 << endl; }
	;

blockBody: 	
		'{' '}'					{ cout << "z values assigned!" << endl; }
	|	'{' propertyList '}' 	{}
	;

// ================================= 
//	STREAM DEFINITION
// =================================
	
signalDef:
		UVAR STREAM UVAR					{ cout << $1 << " -> " << $3 << endl; }			
	|	UVAR STREAM streamDef				{ cout << $1 << " -> Stream" << endl; }	
	|	UVAR STREAM streamDef STREAM UVAR	{ cout << $1 << " -> Stream -> " << $5 << endl; }	
	|	streamDef STREAM UVAR				{ cout << "Stream -> " << $3 << endl; }
	;
	
streamDef:
		functionExp						{}
	|	streamDef STREAM functionExp	{ cout << "STREAMing" << endl; }
	;
	
// ================================= 
//	FUNCTIONS
// =================================


// uVarExp
		// functionExp '+' functionExp		{ cout << "ADDing" << endl; }
	// |	functionExp '-' functionExp		{ cout << "SUBing" << endl; }
	// |	functionExp '*' functionExp		{ cout << "MULing" << endl; }
	// |	functionExp '/' functionExp		{ cout << "DIVing" << endl; }
	// |	'(' functionExp ')'				{}
	// |	functionDef						{}
	// ;

functionExp: 	
		functionExp '+' functionExp		{ cout << "ADDing" << endl; }
	|	functionExp '-' functionExp		{ cout << "SUBing" << endl; }
	|	functionExp '*' functionExp		{ cout << "MULing" << endl; }
	|	functionExp '/' functionExp		{ cout << "DIVing" << endl; }
	|	'(' functionExp ')'				{}
	|	functionDef						{}
	;
	
functionDef:
		WORD '(' ')'					{ cout << "Blank Platform Function: " << $1 << endl; }
	|	WORD '(' propertyList ')'		{ cout << "Properties () ..." << endl << "Platform Function: " << $1 << endl; }
	|	UVAR '(' ')'					{ cout << "Blank User Function: " << $1 << endl; }
	|	UVAR '(' propertyList ')' 		{ cout << "Properties () ..." << endl << "User Function: " << $1 << endl; }
	;

// ================================= 
//	PROPERTIES
// =================================
	
propertyList: 	
		propertyList property	{ cout << "Property Resolved!" << endl; }
	|	property				{ cout << "Property Resolved!" << endl; }
	;
	
property: 	
		WORD COLON propertyType 	{ cout << "Property: " << $1 << endl; }
	;
	
propertyType: 	
		NONE termination			{ cout << "Keyword: none" << endl; }
	|	ON termination				{ cout << "Keyword: on" << endl; }
	|	OFF termination				{ cout << "Keyword: off" << endl; }
	|	STRING termination			{ cout << $1 << endl; }
	|	valueExp termination		{ cout << "Value Expression as Property Values!" << endl; }
	|	blockBody termination		{ cout << "Default Values for Block!" << endl; }
	|	streamDef termination		{ cout << "Stream as Property Value!" << endl; }
	|	listDef termination			{ cout << "List as Property Value!" << endl; }
	;
	
// ================================= 
//	ARRAYS 
// =================================

arrayDef:
		'[' index ']'		{ cout << "Block Bundle Size: " << endl; }
	;
	
arrayElement:
		UVAR '[' index ']'		{ cout << "Array: " << $1 << endl; }
	;
	
index:
		index '+' index 	{ cout << "Index/Size Adding... " << endl; }
	|	index '-' index 	{ cout << "Index/Size Subtracting... " << endl; }
	|	index '*' index 	{ cout << "Index/Size Multiplying... " << endl; }
	|	index '/' index 	{ cout << "Index/Size Dividing... " << endl; }
	|	'(' index ')' 		{ cout << "Index/Size Enclosure..." << endl; }
	| 	INT					{ cout << "Index/Size: " << $1 << endl; }
	|	UVAR				{ cout << "Index/Size: " << $1 << endl; }
	|	arrayElement		{ cout << "Index/Size is an array element: " << endl; }
	;

// ================================= 
//	LISTS
// =================================
	
listDef:
		'[' listType ']'	{}
	;
	
listType:
		integerList		{}	
	|	floatList 		{}
	|	stringList		{}
	|	variableList	{}
	|	arrayList		{}
	|	blockList		{}
//	| 	streamList		{}
	|	listList		{}
	;
	
integerList:
		INT							{ cout << "Integer: " << $1 << endl; }
	|	'-' INT %prec UMINUS 		{ cout << "Integer: -" << $2 << endl; }
	|	integerList COMMA INT		{ cout << "Integer: " << $3 << endl; }
	|	integerList COMMA '-' INT %prec UMINUS	{ cout << "Integer: -" << $4 << endl; }	
	;

floatList:
		FLOAT						{ cout << "Float: " << $1 << endl; }
	|	'-' FLOAT %prec UMINUS 		{ cout << "Float: -" << $2 << endl; }		
	|	floatList COMMA FLOAT		{ cout << "Float: " << $3 << endl; }
	|	floatList COMMA '-' FLOAT %prec UMINUS	{ cout << "Float: -" << $4 << endl; }
	;

stringList:
		STRING						{ cout << "String: " << $1 << endl; }	
	|	stringList COMMA STRING		{ cout << "String: " << $3 << endl; }
	;

variableList:
		UVAR						{ cout << "Variable: " << $1 << endl; }	
	|	variableList COMMA UVAR		{ cout << "Variable: " << $3 << endl; }
	;
	
arrayList:
		arrayElement					{ cout << "Array Element... " << endl; }
	|	arrayList COMMA arrayElement	{ cout << "Array Element... " << endl; }
	;
	
blockList:
		blockDef					{ cout << "Block Definition... " << endl; }	
	|	blockList COMMA blockDef	{ cout << "Block Definition... " << endl; }
	;
	
// streamList:
		// streamDef					{ cout << "Stream Definition... " << endl; }	
	// |	streamList COMMA streamDef	{ cout << "stream Definition... " << endl; }
	// ;
	
listList:
		listDef					{ cout << "SubList...:" << endl; }	
	|	listList COMMA listDef	{ cout << "SubList...:" << endl;}
	;

// ================================= 
//	VALUE EXPRESSION
// =================================
	
valueExp:
		valueExp '+' valueExp 		{ cout << "Adding... " << endl; }
	|	valueExp '-' valueExp 		{ cout << "Subtracting... " << endl; }
	|	valueExp '*' valueExp 		{ cout << "Multiplying... " << endl; }
	|	valueExp '/' valueExp 		{ cout << "Dividing... " << endl; }
	|	'(' valueExp ')' 			{ cout << "Enclosure..." << endl; }
	| 	'-' valueExp %prec UMINUS 	{ cout << "UMINUS" << endl; }
	| 	INT							{ cout << $1 << endl; }
	|	FLOAT						{ cout << $1 << endl; }
	|	UVAR						{ cout << $1 << endl; }
	|	arrayElement				{ cout << "Array Element: " << endl; }
	;

// ================================= 
//	TERMINATION
// =================================

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

int main(int argc, char *argv[]){
	int count;
	FILE * file;
	char const * fileName;

	cout << "Called with \"" << argv[0] << "\"" << endl;

	if (argc > 1) {
		for (count = 1; count < argc; count++) {
			cout << "Argument " << count << ": " << argv[count] << endl;
		}
	}
	else
    {
      cout << "The requires at least file name argument!" << endl;
    }

	fileName = argv[1];
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

