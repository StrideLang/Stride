%{
#include <iostream>
#include <stdio.h>
#include "lang_stream.parser.hpp"
#define YY_DECL extern "C" int yylex()

using namespace std;
%}

%option noyywrap nodefault yylineno

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
":"					{ return COLON; }
";"					{ return SEMICOLON; }
">>"				{ return STREAM; }

"&"					{ return AND; }
"|"					{ return OR; }
"!"					{ return NOT; }

"use"				{ return USE; }
"version"			{ return VERSION; }
"none"				{ return NONE; }
"on"				{ return ON; }
"off"				{ return OFF; }

"and"				{ return AND; }
"or"				{ return OR; }
"not"				{ return NOT; }

{DIGIT}+\.{DIGIT}*	{ yylval.fval = atof(yytext); return FLOAT; }
{DIGIT}*\.{DIGIT}+	{ yylval.fval = atof(yytext); return FLOAT; }
{DIGIT}+			{ yylval.ival = atoi(yytext); return INT; }

{CLETTER}({LETTER}|{CLETTER}|{DIGIT}|_)*	{ yylval.sval = strdup(yytext); return UVAR;}
{LETTER}({LETTER}|{CLETTER}|{DIGIT})*		{ yylval.sval = strdup(yytext); return WORD;}

'[^'\t\n]*'			{ yylval.sval = strdup(yytext); return STRING; }
\"[^"]*\"			{ yylval.sval = strdup(yytext); return STRING; } 

[ \t\n]				{ /* Skip white spaces */ }
"#".*				{ /* Ignore Comments */ } 
.              		{ yylval.sval = strdup(yytext); return ERROR; }
%%