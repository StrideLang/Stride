%{
#include <iostream>
#include <stdio.h>
#include "lang_stream.parser.hpp"
#define YY_DECL extern "C" int yylex()

using namespace std;
%}


DIGIT [0-9]
LETTER [a-z]
CLETTER [A-Z]

%%
"("					{ return OP; }
")"					{ return CP; }
"{"					{ return OCB; }
"}"					{ return CCB; }
"["					{ return OSB; }
"]"					{ return CSB; }
","					{ return COMMA; }		
":"					{ return COLON;}
";"					{ return SEMICOLON; }
">>"				{ return STREAM; }
"use"				{ return USE; }
"none"				{ return NONE; }
"on"				{ return ON; }
"off"				{ return OFF; }
{DIGIT}+\.{DIGIT}*	{ yylval.fval = atof(yytext); return FLOAT; }
{DIGIT}*\.{DIGIT}+	{ yylval.fval = atof(yytext); return FLOAT; }
{DIGIT}+			{ yylval.ival = atoi(yytext); return INT; }
{CLETTER}({LETTER}|{CLETTER}|{DIGIT}|_)+	{ yylval.sval = strdup(yytext); return UVAR;}
{LETTER}({LETTER}|{CLETTER}|{DIGIT})*		{ yylval.sval = strdup(yytext); return WORD;}
"'".+"'"			{ yylval.sval = strdup(yytext); return STRING; }
"\"".+"\""			{ yylval.sval = strdup(yytext); return STRING; } 
[ \t]				{ /* Skip white spaces*/ }
[\n]				{return EOL;}
"#".*"\n"			{ /* Ignore Comments followed by EOL */ } 
.              		{ return ERROR; }
%%
