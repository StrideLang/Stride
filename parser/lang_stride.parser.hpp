
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* "%code requires" blocks.  */

/* Line 1676 of yacc.c  */
#line 50 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "ast.h" 

/* Line 1676 of yacc.c  */
#line 51 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "platformnode.h" 

/* Line 1676 of yacc.c  */
#line 52 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "blocknode.h" 

/* Line 1676 of yacc.c  */
#line 53 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "streamnode.h" 

/* Line 1676 of yacc.c  */
#line 54 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "propertynode.h" 

/* Line 1676 of yacc.c  */
#line 55 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "bundlenode.h" 

/* Line 1676 of yacc.c  */
#line 56 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "valuenode.h" 

/* Line 1676 of yacc.c  */
#line 57 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "namenode.h" 

/* Line 1676 of yacc.c  */
#line 58 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "functionnode.h" 

/* Line 1676 of yacc.c  */
#line 59 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "expressionnode.h" 

/* Line 1676 of yacc.c  */
#line 60 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "listnode.h" 

/* Line 1676 of yacc.c  */
#line 61 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "importnode.h" 

/* Line 1676 of yacc.c  */
#line 62 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "fornode.h" 

/* Line 1676 of yacc.c  */
#line 63 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "rangenode.h" 


/* Line 1676 of yacc.c  */
#line 96 "..\\..\\streamstack\\parser\\lang_stride.parser.hpp"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     FLOAT = 259,
     UVAR = 260,
     WORD = 261,
     STRING = 262,
     ERROR = 263,
     COMMA = 264,
     COLON = 265,
     SEMICOLON = 266,
     USE = 267,
     VERSION = 268,
     WITH = 269,
     IMPORT = 270,
     AS = 271,
     FOR = 272,
     NONE = 273,
     ON = 274,
     OFF = 275,
     STREAMRATE = 276,
     STREAM = 277,
     OR = 278,
     AND = 279,
     NOT = 280,
     UMINUS = 281,
     DOT = 282
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 65 "..\\..\\streamstack\\parser\\lang_stride.y"

	int 	ival;
    double 	fval;
    char *	sval;
    AST  *  ast;
    PlatformNode *platformNode;
    HwPlatform *hwPlatform;
    BlockNode *blockNode;
    StreamNode *streamNode;
    PropertyNode *propertyNode;
    BundleNode *bundleNode;
    FunctionNode *functionNode;
    ListNode *listNode;
    ExpressionNode *expressionNode;
    ImportNode *importNode;
    ForNode *forNode;
    RangeNode *rangeNode;



/* Line 1676 of yacc.c  */
#line 161 "..\\..\\streamstack\\parser\\lang_stride.parser.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;

