%{
#include <iostream>
#include <stdio.h>

using namespace std;

extern "C" int yylex();
extern "C" FILE *yyin;

int parse(const char* fileName);
int yyerror(const char *s);
%}

%union {
	int 	ival;
	float 	fval;
	char *	sval;
}

/* declare tokens */
%token <ival> INT
%token <fval> FLOAT
%token <sval> UVAR
%token <sval> WORD
%token <sval> STRING

%token OP CP OCB CCB OSB CSB
%token COMMA COLON SEMICOLON STREAM
%token USE NONE ON OFF 
%token EOL
%token ERROR


%%

loop: loop INT 		{ printf("Found an integer number: %d\n", $2); }
	| loop FLOAT 	{ printf("Found a floating-point number: %f\n", $2); }
	| loop UVAR		{ printf("Found a user declared variable: %s\n", $2); }
	| loop WORD		{ printf("Found a word: %s\n", $2); }
	| loop STRING	{ printf("Found a string: %s\n", $2); }
	| loop OP		{ printf("Found an OP!\n"); }
	| loop CP		{ printf("Found a CP!\n" ); }	
	| loop OCB		{ printf("Found an OCB!\n"); }	
	| loop CCB		{ printf("Found a CCB!\n"); }	
	| loop OSB		{ printf("Found an OSB!\n"); }
	| loop CSB		{ printf("Found an CSB!\n"); }
	| loop COMMA	{ printf("Found an COMMA!\n"); }
	| loop COLON	{ printf("Found an COLON!\n"); }
	| loop SEMICOLON{ printf("Found an SEMICOLON!\n"); }
	| loop STREAM	{ printf("Found an STREAM!\n"); }
	| loop USE		{ printf("Found an KEYWORD use!\n"); }	
	| loop NONE		{ printf("Found an KEYWORD none!\n"); }	
	| loop ON		{ printf("Found an KEYWORD on!\n"); }	
	| loop OFF		{ printf("Found an KEYWORD off!\n"); }	
	| loop EOL		{ printf("EOL\n"); }
	| loop ERROR 	{ yyerror("Unknown Character!"); }
	| INT 			{ printf("Found an integer number: %d\n", $1); }
	| FLOAT 		{ printf("Found a floating-point number: %f\n", $1); }
	| UVAR			{ printf("Found a user declared variable: %s\n", $1); }
	| WORD			{ printf("Found a word: %s\n", $1); }
	| STRING		{ printf("Found a string: %s\n", $1); }
	| OP			{ printf("Found an OP!\n"); }
	| CP			{ printf("Found a CP!\n" ); }	
	| OCB			{ printf("Found an OCB!\n"); }	
	| CCB			{ printf("Found a CCB!\n"); }	
	| OSB			{ printf("Found an OSB!\n"); }
	| CSB			{ printf("Found an CSB!\n"); }
	| COMMA			{ printf("Found an COMMA!\n"); }
	| COLON			{ printf("Found an COLON!\n"); }
	| SEMICOLON		{ printf("Found an SEMICOLON!\n"); }
	| STREAM		{ printf("Found an STREAM!\n"); }
	| USE			{ printf("Found an KEYWORD use!\n"); }	
	| NONE			{ printf("Found an KEYWORD none!\n"); }	
	| ON			{ printf("Found an KEYWORD on!\n"); }	
	| OFF			{ printf("Found an KEYWORD off!\n"); }	
	| EOL			{ printf("EOL\n"); }
	| ERROR			{ yyerror("Unknown Character!"); }
 ;

%%
int yyerror(const char *s) {
  cout << "yyerror : " << s << endl;
  return 0;
}

int parse(const char* fileName)
{
	FILE *myfile = fopen(fileName, "r");
	// make sure it's valid:
	if (!myfile) {
		printf("I can't open %s!\n", fileName);
		return -1;
	}
        // set lex to read from it instead of defaulting to STDIN:
        yyin = myfile;
	yyparse();
	
	printf("Completed Analysing: %s!\n", fileName);
	return 0;
}

