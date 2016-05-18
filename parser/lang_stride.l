%{
#include <stdio.h>
#include "lang_stride.parser.hpp"
#define YY_DECL extern "C" int yylex()

using namespace std;

char * buffer;
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;

%}

%option noyywrap nodefault
%option yylineno

DIGIT	[0-9]
LETTER	[a-z]
CLETTER	[A-Z]

%%
"("	|
")"	|
"{"	|
"}"	|
"["	|
"]"	|
"+"	|
"-"	|
"*"	|
"/"					{ return yytext[0]; }

","					{ return COMMA; }
"."					{ return DOT; }
":"					{ return COLON; }
";"					{ return SEMICOLON; }
">>"				{ return STREAM; }

"&&"				{ return AND; }
"||"				{ return OR; }
"!"					{ return NOT; }

"&"					{ return BITAND; }
"|"					{ return BITOR; }
"~"					{ return BITNOT; }

"=="				{ return EQUAL; }
"!="				{ return NOTEQUAL; }
">"					{ return GREATER; }
"<"					{ return LESSER; }
">="				{ return GREATEREQUAL; }
"<="				{ return LESSEREQUAL; }

"use"				{ return USE; }
"version"			{ return VERSION; }
"with"              { return WITH; }
"import"			{ return IMPORT; }
"as"				{ return AS; }
"for"               { return FOR; }
"none"				{ return NONE; }
"on"				{ return ON; }
"off"				{ return OFF; }

"and"				{ return AND; }
"or"				{ return OR; }
"not"				{ return NOT; }

"streamRate"		{ return STREAMRATE; }
"streamDomain"		{ return STREAMDOMAIN; }

{DIGIT}+\.{DIGIT}*	{ yylval.fval = atof(yytext); return FLOAT; }
{DIGIT}*\.{DIGIT}+	{ yylval.fval = atof(yytext); return FLOAT; }
{DIGIT}+			{ yylval.ival = atoi(yytext); return INT; }

(_)*{CLETTER}({LETTER}|{CLETTER}|{DIGIT}|_)*	{ yylval.sval = strdup(yytext); return UVAR;}
(_)*{LETTER}({LETTER}|{CLETTER}|{DIGIT})*		{ yylval.sval = strdup(yytext); return WORD;}

'[^']*'			{	buffer = strdup(yytext);
                        buffer[strlen(buffer)-1] = '\0';
                        yylval.sval = strdup(buffer + 1);
                        free(buffer);
                        return STRING; }

\"[^"]*\"			{ 	buffer = strdup(yytext);
                        buffer[strlen(buffer)-1] = '\0';
                        yylval.sval = strdup(buffer + 1);
                        free(buffer);
                        return STRING; }

[ \t\n]				{ /* Skip white spaces */ }
"#".*				{ /* Ignore Comments */ } 
.              		{ yylval.sval = strdup(yytext); return ERROR; }
%%
