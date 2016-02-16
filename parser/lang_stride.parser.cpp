
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "..\\..\\streamstack\\parser\\lang_stride.y"

#include <iostream>
#include <string>
#include <vector>

#include <cstdio>  // for fopen
#include <cstdarg>  //for var args

#include "ast.h"

using namespace std;

extern "C" int yylex();
extern "C" int yylineno;
extern "C" char * yytext;
extern "C" FILE *yyin;

std::vector<LangError> parseErrors;

void yyerror(const char *s, ...);

AST *tree_head;

AST *parse(const char *filename);

std::vector<LangError> getErrors();

//#define DEBUG

#ifdef DEBUG
#define COUT cout
#define ENDL endl
#else
class NullStream {
public:
    void setFile() { /* no-op */ }
    template<typename TPrintable>
    NullStream& operator<<(TPrintable const&)
    { /* no-op */
        return *this;
    }
};
NullStream nstream;
#define COUT nstream
#define ENDL ""
#endif



/* Line 189 of yacc.c  */
#line 123 "..\\..\\streamstack\\parser\\lang_stride.parser.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

/* "%code requires" blocks.  */

/* Line 209 of yacc.c  */
#line 50 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "ast.h" 

/* Line 209 of yacc.c  */
#line 51 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "platformnode.h" 

/* Line 209 of yacc.c  */
#line 52 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "blocknode.h" 

/* Line 209 of yacc.c  */
#line 53 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "streamnode.h" 

/* Line 209 of yacc.c  */
#line 54 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "propertynode.h" 

/* Line 209 of yacc.c  */
#line 55 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "bundlenode.h" 

/* Line 209 of yacc.c  */
#line 56 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "valuenode.h" 

/* Line 209 of yacc.c  */
#line 57 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "namenode.h" 

/* Line 209 of yacc.c  */
#line 58 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "functionnode.h" 

/* Line 209 of yacc.c  */
#line 59 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "expressionnode.h" 

/* Line 209 of yacc.c  */
#line 60 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "listnode.h" 

/* Line 209 of yacc.c  */
#line 61 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "importnode.h" 

/* Line 209 of yacc.c  */
#line 62 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "fornode.h" 

/* Line 209 of yacc.c  */
#line 63 "..\\..\\streamstack\\parser\\lang_stride.y"
 #include "rangenode.h" 


/* Line 209 of yacc.c  */
#line 203 "..\\..\\streamstack\\parser\\lang_stride.parser.cpp"

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

/* Line 214 of yacc.c  */
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



/* Line 214 of yacc.c  */
#line 268 "..\\..\\streamstack\\parser\\lang_stride.parser.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

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


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 293 "..\\..\\streamstack\\parser\\lang_stride.parser.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   308

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  34
/* YYNRULES -- Number of rules.  */
#define YYNRULES  122
/* YYNRULES -- Number of states.  */
#define YYNSTATES  212

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   282

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      35,    36,    31,    29,     2,    30,     2,    32,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     9,     2,    10,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,     2,    12,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      33,    34,    37
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    12,    14,    16,    18,
      20,    22,    25,    30,    33,    38,    41,    46,    48,    52,
      55,    60,    65,    68,    70,    74,    78,    85,    88,    92,
      97,   102,   107,   114,   118,   123,   129,   136,   140,   143,
     146,   148,   152,   154,   156,   158,   160,   162,   166,   170,
     174,   176,   180,   182,   186,   190,   194,   198,   200,   204,
     206,   210,   212,   216,   220,   222,   224,   228,   232,   236,
     240,   244,   248,   250,   254,   258,   262,   266,   270,   274,
     278,   282,   286,   290,   294,   298,   302,   306,   310,   314,
     318,   322,   324,   328,   332,   336,   340,   344,   348,   352,
     355,   358,   360,   364,   366,   368,   370,   374,   376,   378,
     382,   384,   386,   388,   390,   392,   394,   396,   398,   400,
     402,   406,   408
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      39,     0,    -1,    -1,    39,    40,    -1,    39,    15,    -1,
      41,    -1,    45,    -1,    46,    -1,    48,    -1,    50,    -1,
       8,    -1,    42,    43,    -1,    42,    43,    18,    44,    -1,
      16,     5,    -1,    16,     5,    17,     4,    -1,    23,     5,
      -1,    23,     5,    17,     4,    -1,     5,    -1,    44,    28,
       5,    -1,    19,     6,    -1,    19,     6,    20,     6,    -1,
      19,     5,    20,     6,    -1,    21,    47,    -1,     5,    -1,
      47,    28,     5,    -1,     6,     5,    49,    -1,     6,     5,
       9,    65,    10,    49,    -1,    11,    12,    -1,    11,    53,
      12,    -1,    67,    26,    68,    15,    -1,    66,    26,    68,
      15,    -1,     5,     9,    63,    10,    -1,     6,    37,     5,
       9,    63,    10,    -1,     5,    35,    36,    -1,     5,    35,
      53,    36,    -1,     6,    37,     5,    35,    36,    -1,     6,
      37,     5,    35,    53,    36,    -1,    53,    54,    15,    -1,
      53,    54,    -1,    54,    15,    -1,    54,    -1,     6,    14,
      55,    -1,    22,    -1,    67,    -1,    49,    -1,    59,    -1,
      66,    -1,     9,    57,    10,    -1,     9,    58,    10,    -1,
      57,    13,    67,    -1,    67,    -1,    58,    13,    56,    -1,
      56,    -1,     9,    60,    10,    -1,     9,    61,    10,    -1,
       9,    62,    10,    -1,    60,    13,    48,    -1,    48,    -1,
      61,    13,    50,    -1,    50,    -1,    62,    13,    59,    -1,
      59,    -1,    63,    13,    65,    -1,    63,    13,    64,    -1,
      65,    -1,    64,    -1,    65,    14,    65,    -1,    65,    29,
      65,    -1,    65,    30,    65,    -1,    65,    31,    65,    -1,
      65,    32,    65,    -1,    35,    65,    36,    -1,    69,    -1,
      56,    29,    67,    -1,    56,    30,    67,    -1,    56,    31,
      67,    -1,    56,    32,    67,    -1,    56,    28,    67,    -1,
      56,    27,    67,    -1,    67,    29,    56,    -1,    67,    30,
      56,    -1,    67,    31,    56,    -1,    67,    32,    56,    -1,
      67,    28,    56,    -1,    67,    27,    56,    -1,    56,    29,
      56,    -1,    56,    30,    56,    -1,    56,    31,    56,    -1,
      56,    32,    56,    -1,    56,    28,    56,    -1,    56,    27,
      56,    -1,    56,    -1,    67,    29,    67,    -1,    67,    30,
      67,    -1,    67,    31,    67,    -1,    67,    32,    67,    -1,
      67,    28,    67,    -1,    67,    27,    67,    -1,    35,    67,
      36,    -1,    30,    67,    -1,    33,    67,    -1,    71,    -1,
      70,    26,    68,    -1,    70,    -1,     3,    -1,     5,    -1,
       6,    37,     5,    -1,    51,    -1,     5,    -1,     6,    37,
       5,    -1,    51,    -1,    52,    -1,    56,    -1,     3,    -1,
       4,    -1,    23,    -1,    24,    -1,     7,    -1,    25,    -1,
       5,    -1,     6,    37,     5,    -1,    51,    -1,    52,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   146,   146,   147,   148,   152,   156,   160,   164,   169,
     173,   183,   191,   206,   213,   223,   231,   242,   251,   271,
     278,   288,   305,   316,   325,   345,   357,   373,   377,   387,
     391,   403,   410,   458,   465,   475,   485,   506,   516,   525,
     531,   539,   549,   553,   557,   563,   566,   577,   581,   588,
     599,   607,   616,   626,   627,   628,   632,   643,   651,   662,
     670,   681,   693,   696,   699,   705,   714,   725,   729,   733,
     737,   741,   745,   755,   759,   763,   767,   771,   774,   777,
     781,   785,   789,   793,   796,   799,   802,   805,   808,   811,
     814,   817,   827,   831,   835,   839,   843,   847,   851,   855,
     859,   863,   873,   876,   886,   890,   897,   907,   918,   926,
     937,   941,   945,   956,   960,   963,   967,   971,   978,   981,
     988,   998,  1002
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "UVAR", "WORD", "STRING",
  "ERROR", "'['", "']'", "'{'", "'}'", "COMMA", "COLON", "SEMICOLON",
  "USE", "VERSION", "WITH", "IMPORT", "AS", "FOR", "NONE", "ON", "OFF",
  "STREAMRATE", "STREAM", "OR", "AND", "'+'", "'-'", "'*'", "'/'", "NOT",
  "UMINUS", "'('", "')'", "DOT", "$accept", "entry", "start",
  "platformDef", "languagePlatform", "targetPlatform", "auxPlatformDef",
  "importDef", "forDef", "forPlatformDef", "blockDef", "blockType",
  "streamDef", "bundleDef", "functionDef", "properties", "property",
  "propertyType", "valueListDef", "valueList", "valueListList", "listDef",
  "blockList", "streamList", "listList", "indexList", "indexRange",
  "indexExp", "valueListExp", "valueExp", "streamExp", "indexComp",
  "streamComp", "valueComp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    91,
      93,   123,   125,   264,   265,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   278,   279,    43,
      45,    42,    47,   280,   281,    40,    41,   282
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    38,    39,    39,    39,    40,    40,    40,    40,    40,
      40,    41,    41,    42,    42,    43,    43,    44,    44,    45,
      45,    45,    46,    47,    47,    48,    48,    49,    49,    50,
      50,    51,    51,    52,    52,    52,    52,    53,    53,    53,
      53,    54,    55,    55,    55,    55,    55,    56,    56,    57,
      57,    58,    58,    59,    59,    59,    60,    60,    61,    61,
      62,    62,    63,    63,    63,    63,    64,    65,    65,    65,
      65,    65,    65,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    66,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    68,    68,    69,    69,    69,    69,    70,    70,
      70,    70,    70,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     1,     1,     1,     1,     1,
       1,     2,     4,     2,     4,     2,     4,     1,     3,     2,
       4,     4,     2,     1,     3,     3,     6,     2,     3,     4,
       4,     4,     6,     3,     4,     5,     6,     3,     2,     2,
       1,     3,     1,     1,     1,     1,     1,     3,     3,     3,
       1,     3,     1,     3,     3,     3,     3,     1,     3,     1,
       3,     1,     3,     3,     1,     1,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     1,     3,     1,     1,     1,     3,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,   113,   114,   119,     0,   117,    10,     0,
       4,     0,     0,     0,   115,   116,   118,     0,     0,     0,
       3,     5,     0,     6,     7,     8,     9,   121,   122,    91,
       0,     0,   101,     0,     0,     0,     0,     0,    52,     0,
       0,    50,    13,     0,    19,    23,    22,    99,   100,     0,
       0,    11,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   104,   105,     0,     0,
     107,     0,    65,    64,    72,     0,    33,     0,    40,     0,
       0,    25,   120,    47,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,    15,     0,
      90,    78,    89,    77,    85,    73,    86,    74,    87,    75,
      88,    76,   108,     0,   110,   111,   112,     0,   103,     0,
      84,    97,    83,    96,    79,    92,    80,    93,    81,    94,
      82,    95,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,    34,    38,    39,     0,    27,     0,     0,     0,
      49,    51,    14,    21,    20,    24,     0,    17,    12,     0,
      30,     0,    29,   106,    71,    63,    62,    66,    67,    68,
      69,    70,     0,    42,    44,    41,    45,    46,    43,    37,
       0,    28,     0,    35,     0,    16,     0,   109,   102,    57,
      59,    52,    61,     0,     0,     0,    50,    26,    32,    36,
      18,    53,     0,    54,     0,    55,     0,     0,    56,    58,
       0,    60
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    20,    21,    22,    51,   158,    23,    24,    46,
     189,    81,   190,    27,    28,    77,    78,   175,    29,    39,
      40,   192,   193,   194,   195,    71,    72,    73,    30,    31,
     117,    74,   118,    32
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -134
static const yytype_int16 yypact[] =
{
    -134,   121,  -134,  -134,  -134,   113,     2,  -134,  -134,   201,
    -134,    24,    13,    32,  -134,  -134,  -134,   247,   247,   247,
    -134,  -134,    91,  -134,  -134,  -134,  -134,  -134,  -134,   184,
     105,   257,  -134,    17,     8,   130,   138,   116,  -134,   106,
     122,   270,   149,   147,   154,  -134,   148,  -134,  -134,   228,
     173,   167,   201,   201,   201,   201,   201,   201,    27,    27,
     201,   201,   201,   201,   201,   201,  -134,   175,   156,    17,
    -134,   137,  -134,    -5,  -134,   172,  -134,    69,   179,    17,
       9,  -134,   114,  -134,   247,  -134,   189,   247,   247,   247,
     247,   247,   247,   205,   216,   222,   224,  -134,   215,   230,
    -134,   270,  -134,   270,  -134,   270,  -134,   270,  -134,   270,
    -134,   270,   113,   196,  -134,  -134,  -134,   225,   219,   226,
    -134,    72,  -134,    60,  -134,    11,  -134,    11,  -134,  -134,
    -134,  -134,   237,   151,  -134,    17,    17,    17,    17,    17,
      17,   166,  -134,   231,  -134,   128,  -134,    28,    17,    81,
     270,  -134,  -134,  -134,  -134,  -134,   239,  -134,   220,   256,
    -134,    27,  -134,   253,  -134,  -134,    -5,   132,    47,    47,
    -134,  -134,   214,  -134,  -134,  -134,  -134,  -134,   276,  -134,
     252,  -134,   142,  -134,    82,  -134,   260,   114,  -134,  -134,
    -134,   264,  -134,   182,   187,   217,   257,  -134,  -134,  -134,
    -134,  -134,   261,  -134,   201,  -134,   259,   268,  -134,  -134,
     214,  -134
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -134,  -134,  -134,  -134,  -134,  -134,  -134,  -134,  -134,  -134,
       0,  -106,    -1,   -28,   -55,   -63,   -71,  -134,     7,  -134,
    -134,  -133,  -134,  -134,  -134,   118,   134,   -41,   133,    -7,
     -46,  -134,  -134,  -134
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -92
static const yytype_int16 yytable[] =
{
      26,    25,    41,   115,   115,    70,   143,    35,   176,   136,
      47,    48,    49,   119,    75,    75,    38,   147,    43,    44,
      66,   146,    67,    68,   137,   138,   139,   140,   133,    42,
     114,   114,   112,   113,    75,   174,     9,    45,   145,    36,
     181,    70,    91,    92,    76,   101,   103,   105,   107,   109,
     111,    70,    69,   121,   123,   125,   127,   129,   131,   100,
     102,   104,   106,   108,   110,   116,   116,   120,   122,   124,
     126,   128,   130,   211,   197,    75,   143,   150,   139,   140,
     121,   123,   125,   127,   129,   131,   184,    75,    75,    89,
      90,    91,    92,   151,   166,   167,   168,   169,   170,   171,
      88,    89,    90,    91,    92,   142,   115,    70,    70,    70,
      70,    70,    70,   143,    50,   188,    83,   183,   199,    84,
      70,     2,    33,   148,     3,     4,     5,     6,     7,     8,
       9,    58,    85,   114,   178,    86,    10,    11,   180,    79,
      12,    80,    13,    82,    14,    15,    16,   134,    34,   149,
     135,    17,   198,    36,    18,   135,    19,   137,   138,   139,
     140,   137,   138,   139,   140,   196,    93,    94,   116,     3,
       4,     5,    37,     7,    95,   172,    96,    80,    98,   191,
     137,   138,   139,   140,    33,    99,   141,   164,   173,    14,
      15,    16,   201,   132,   144,   202,    17,   203,     9,    18,
     204,    19,   208,   209,     3,     4,     5,    37,     7,   152,
       9,    52,    53,    54,    55,    56,    57,     3,     4,     5,
       6,     7,   153,   172,    14,    15,    16,   205,   154,   155,
     206,    17,   156,   159,    18,   157,    19,    14,    15,    16,
     160,   162,   163,   185,    17,   161,   179,    18,   186,    19,
       3,     4,     5,    37,     7,    87,    88,    89,    90,    91,
      92,   187,   148,    80,    97,   200,   182,   207,   210,   165,
      14,    15,    16,    35,   177,     0,     0,    17,     0,     0,
      18,     0,    19,    59,    60,    61,    62,    63,    64,    65,
     -91,    52,    53,    54,    55,    56,    57,    87,    88,    89,
      90,    91,    92,    60,    61,    62,    63,    64,    65
};

static const yytype_int16 yycheck[] =
{
       1,     1,     9,    58,    59,    33,    77,     5,   141,    14,
      17,    18,    19,    59,     6,     6,     9,    80,     5,     6,
       3,    12,     5,     6,    29,    30,    31,    32,    69,     5,
      58,    59,     5,     6,     6,   141,     9,     5,    79,    37,
      12,    69,    31,    32,    36,    52,    53,    54,    55,    56,
      57,    79,    35,    60,    61,    62,    63,    64,    65,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,   206,   180,     6,   147,    84,    31,    32,
      87,    88,    89,    90,    91,    92,   149,     6,     6,    29,
      30,    31,    32,    86,   135,   136,   137,   138,   139,   140,
      28,    29,    30,    31,    32,    36,   161,   135,   136,   137,
     138,   139,   140,   184,    23,   161,    10,    36,    36,    13,
     148,     0,     9,     9,     3,     4,     5,     6,     7,     8,
       9,    26,    10,   161,   141,    13,    15,    16,    10,     9,
      19,    11,    21,     5,    23,    24,    25,    10,    35,    35,
      13,    30,    10,    37,    33,    13,    35,    29,    30,    31,
      32,    29,    30,    31,    32,   172,    17,    20,   161,     3,
       4,     5,     6,     7,    20,     9,    28,    11,     5,   172,
      29,    30,    31,    32,     9,    18,    14,    36,    22,    23,
      24,    25,    10,    37,    15,    13,    30,    10,     9,    33,
      13,    35,   202,   204,     3,     4,     5,     6,     7,     4,
       9,    27,    28,    29,    30,    31,    32,     3,     4,     5,
       6,     7,     6,     9,    23,    24,    25,    10,     6,     5,
      13,    30,    17,    37,    33,     5,    35,    23,    24,    25,
      15,    15,     5,     4,    30,    26,    15,    33,    28,    35,
       3,     4,     5,     6,     7,    27,    28,    29,    30,    31,
      32,     5,     9,    11,    36,     5,   148,     6,     9,   135,
      23,    24,    25,     5,   141,    -1,    -1,    30,    -1,    -1,
      33,    -1,    35,    26,    27,    28,    29,    30,    31,    32,
      26,    27,    28,    29,    30,    31,    32,    27,    28,    29,
      30,    31,    32,    27,    28,    29,    30,    31,    32
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    39,     0,     3,     4,     5,     6,     7,     8,     9,
      15,    16,    19,    21,    23,    24,    25,    30,    33,    35,
      40,    41,    42,    45,    46,    48,    50,    51,    52,    56,
      66,    67,    71,     9,    35,     5,    37,     6,    56,    57,
      58,    67,     5,     5,     6,     5,    47,    67,    67,    67,
      23,    43,    27,    28,    29,    30,    31,    32,    26,    26,
      27,    28,    29,    30,    31,    32,     3,     5,     6,    35,
      51,    63,    64,    65,    69,     6,    36,    53,    54,     9,
      11,    49,     5,    10,    13,    10,    13,    27,    28,    29,
      30,    31,    32,    17,    20,    20,    28,    36,     5,    18,
      56,    67,    56,    67,    56,    67,    56,    67,    56,    67,
      56,    67,     5,     6,    51,    52,    56,    68,    70,    68,
      56,    67,    56,    67,    56,    67,    56,    67,    56,    67,
      56,    67,    37,    65,    10,    13,    14,    29,    30,    31,
      32,    14,    36,    54,    15,    65,    12,    53,     9,    35,
      67,    56,     4,     6,     6,     5,    17,     5,    44,    37,
      15,    26,    15,     5,    36,    64,    65,    65,    65,    65,
      65,    65,     9,    22,    49,    55,    59,    66,    67,    15,
      10,    12,    63,    36,    53,     4,    28,     5,    68,    48,
      50,    56,    59,    60,    61,    62,    67,    49,    10,    36,
       5,    10,    13,    10,    13,    10,    13,     6,    48,    50,
       9,    59
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 146 "..\\..\\streamstack\\parser\\lang_stride.y"
    {;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 147 "..\\..\\streamstack\\parser\\lang_stride.y"
    { COUT << ENDL << "Grabbing Next ..." << ENDL ; ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 148 "..\\..\\streamstack\\parser\\lang_stride.y"
    { COUT << "Ignoring Semicolon!" << ENDL ; ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 152 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                        tree_head->addChild((yyvsp[(1) - (1)].platformNode));
                        COUT << "Platform Definition Resolved!" << ENDL;
                    ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 156 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                        tree_head->addChild((yyvsp[(1) - (1)].importNode));
                        COUT << "Import Definition Resolved!" << ENDL;
                    ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 160 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                        tree_head->addChild((yyvsp[(1) - (1)].forNode));
                        COUT << "For Definition Resolved!" << ENDL;
                    ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 164 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                        tree_head->addChild((yyvsp[(1) - (1)].blockNode));
                        COUT << "Block Resolved!" << ENDL;
                    ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 169 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                        tree_head->addChild((yyvsp[(1) - (1)].streamNode));
                        COUT << "Stream Definition Resolved!" << ENDL;
                    ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 173 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                        yyerror("Unrecognized character", (yyvsp[(1) - (1)].sval), yyloc.first_line);
                    ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 183 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          PlatformNode *platformNode = (yyvsp[(1) - (2)].platformNode);
          HwPlatform *hwPlatform = (yyvsp[(2) - (2)].hwPlatform);
          platformNode->setHwPlatform(hwPlatform->name);
          platformNode->setHwVersion(hwPlatform->version);
          delete (yyvsp[(2) - (2)].hwPlatform);
          (yyval.platformNode) = platformNode;
        ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 191 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          PlatformNode *platformNode = (yyvsp[(1) - (4)].platformNode);
          HwPlatform *hwPlatform = (yyvsp[(2) - (4)].hwPlatform);
          AST *aux = (yyvsp[(4) - (4)].ast);
          platformNode->setHwPlatform(hwPlatform->name);
          platformNode->setHwVersion(hwPlatform->version);
          vector<AST *> children = aux->getChildren();
          platformNode->setChildren(children);
          delete (yyvsp[(2) - (4)].hwPlatform);
          delete (yyvsp[(4) - (4)].ast);
          (yyval.platformNode) = platformNode;
        ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 206 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    string s;
                                    s.append((yyvsp[(2) - (2)].sval)); /* string constructor leaks otherwise! */
                                    (yyval.platformNode) = new PlatformNode(s, -1, yyloc.first_line);
                                    COUT << "Platform: " << (yyvsp[(2) - (2)].sval) << ENDL << " Using latest version!" << ENDL;
                                    free((yyvsp[(2) - (2)].sval));
                                ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 213 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    string s;
                                    s.append((yyvsp[(2) - (4)].sval)); /* string constructor leaks otherwise! */
                                    (yyval.platformNode) = new PlatformNode(s, (yyvsp[(4) - (4)].fval), yyloc.first_line);
                                    COUT << "Platform: " << (yyvsp[(2) - (4)].sval) << ENDL << "Version: " << (yyvsp[(4) - (4)].fval) << " line " << yylineno << ENDL;
                                    free((yyvsp[(2) - (4)].sval));
                                ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 223 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                     HwPlatform *hwPlatform = new HwPlatform;
                                     hwPlatform->name = (yyvsp[(2) - (2)].sval);
                                     hwPlatform->version = -1;
                                     (yyval.hwPlatform) = hwPlatform;
                                     COUT << "Target platform: " << (yyvsp[(2) - (2)].sval) << ENDL << "Target Version: Current!" << ENDL;
                                     free((yyvsp[(2) - (2)].sval));
                                ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 231 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    HwPlatform *hwPlatform = new HwPlatform;
                                    hwPlatform->name = (yyvsp[(2) - (4)].sval);
                                    hwPlatform->version = (yyvsp[(4) - (4)].fval);
                                    (yyval.hwPlatform) = hwPlatform;
                                    COUT << "Target platform: " << (yyvsp[(2) - (4)].sval) << ENDL << "Target Version: " << (yyvsp[(4) - (4)].fval) << ENDL;
                                    free((yyvsp[(2) - (4)].sval));
                                 ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 242 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          string word;
          word.append((yyvsp[(1) - (1)].sval)); /* string constructor leaks otherwise! */
          AST *temp = new AST();
          temp->addChild(new NameNode(word, yyloc.first_line));
          (yyval.ast) = temp;
          COUT << "With additional platform: " << (yyvsp[(1) - (1)].sval) << ENDL;
          free((yyvsp[(1) - (1)].sval));
        ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 251 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          AST *temp = new AST();
          AST *aux = (yyvsp[(1) - (3)].ast);
          aux->giveChildren(temp);
          string word;
          word.append((yyvsp[(3) - (3)].sval)); /* string constructor leaks otherwise! */
          temp->addChild(new NameNode(word, yyloc.first_line));
          (yyval.ast) = temp;
          aux->deleteChildren();
          delete aux;
          COUT << "With additional platform: " << (yyvsp[(3) - (3)].sval) << ENDL;
          free((yyvsp[(3) - (3)].sval));
        ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 271 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          string word;
          word.append((yyvsp[(2) - (2)].sval)); /* string constructor leaks otherwise! */
          (yyval.importNode) = new ImportNode(word, yyloc.first_line);
          COUT << "Importing: " << (yyvsp[(2) - (2)].sval) << ENDL;
          free((yyvsp[(2) - (2)].sval));
        ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 278 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          string word;
          word.append((yyvsp[(2) - (4)].sval)); /* string constructor leaks otherwise! */
          string alias;
          alias.append((yyvsp[(4) - (4)].sval)); /* string constructor leaks otherwise! */
          (yyval.importNode) = new ImportNode(word, yyloc.first_line, alias);
          COUT << "Importing: " << (yyvsp[(2) - (4)].sval) << " as " << (yyvsp[(4) - (4)].sval) << ENDL;
          free((yyvsp[(2) - (4)].sval));
          free((yyvsp[(4) - (4)].sval));
        ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 288 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          string word;
          word.append((yyvsp[(2) - (4)].sval)); /* string constructor leaks otherwise! */
          string alias;
          alias.append((yyvsp[(4) - (4)].sval)); /* string constructor leaks otherwise! */
          (yyval.importNode) = new ImportNode(word, yyloc.first_line, alias);
          COUT << "Importing: " << (yyvsp[(2) - (4)].sval) << " as " << (yyvsp[(4) - (4)].sval) << ENDL;
          free((yyvsp[(2) - (4)].sval));
          free((yyvsp[(4) - (4)].sval));
        ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 305 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          AST* aux = (yyvsp[(2) - (2)].ast);
          ForNode *fornode = new ForNode(yyloc.first_line);
          vector<AST *> children = aux->getChildren();
          fornode->setChildren(children);
          (yyval.forNode) = fornode;
          delete (yyvsp[(2) - (2)].ast);
        ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 316 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          string word;
          word.append((yyvsp[(1) - (1)].sval)); /* string constructor leaks otherwise! */
          AST *temp = new AST();
          temp->addChild(new NameNode(word, yyloc.first_line));
          (yyval.ast) = temp;
          COUT << "For platform: " << (yyvsp[(1) - (1)].sval) << ENDL;
          free((yyvsp[(1) - (1)].sval));
        ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 325 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
          AST *temp = new AST();
          AST *aux = (yyvsp[(1) - (3)].ast);
          aux->giveChildren(temp);
          string word;
          word.append((yyvsp[(3) - (3)].sval)); /* string constructor leaks otherwise! */
          temp->addChild(new NameNode(word, yyloc.first_line));
          (yyval.ast) = temp;
          aux->deleteChildren();
          delete aux;
          COUT << "For platform: " << (yyvsp[(3) - (3)].sval) << ENDL;
          free((yyvsp[(3) - (3)].sval));
          ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 345 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        string word;
                                        word.append((yyvsp[(1) - (3)].sval)); /* string constructor leaks otherwise! */
                                        string uvar;
                                        uvar.append((yyvsp[(2) - (3)].sval)); /* string constructor leaks otherwise! */
                                        (yyval.blockNode) = new BlockNode(uvar, word, (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                        AST *props = (yyvsp[(3) - (3)].ast);
                                        delete props;
                                        COUT << "Block: " << (yyvsp[(1) - (3)].sval) << ", Labelled: " << (yyvsp[(2) - (3)].sval) << ENDL;
                                        free((yyvsp[(1) - (3)].sval));
                                        free((yyvsp[(2) - (3)].sval));
                                    ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 357 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
             string name;
             name.append((yyvsp[(2) - (6)].sval)); /* string constructor leaks otherwise! */
             ListNode *list = new ListNode((yyvsp[(4) - (6)].ast), yyloc.first_line);
             BundleNode *bundle = new BundleNode(name, list, yyloc.first_line);
             COUT << "Bundle name: " << name << ENDL;
             string type;
             type.append((yyvsp[(1) - (6)].sval)); /* string constructor leaks otherwise! */
             (yyval.blockNode) = new BlockNode(bundle, type, (yyvsp[(6) - (6)].ast), yyloc.first_line);
             COUT << "Block Bundle: " << (yyvsp[(1) - (6)].sval) << ", Labelled: " << (yyvsp[(2) - (6)].sval) << ENDL;
             free((yyvsp[(2) - (6)].sval));
             free((yyvsp[(1) - (6)].sval));
         ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 373 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                (yyval.ast) = NULL;
                                COUT << "Default values assigned!" << ENDL;
                            ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 377 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                (yyval.ast) = (yyvsp[(2) - (3)].ast);
                            ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 387 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                    (yyval.streamNode) = new StreamNode((yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), yyloc.first_line);
                                                    COUT << "Stream Resolved!" << ENDL;
                                                ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 391 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                    (yyval.streamNode) = new StreamNode((yyvsp[(1) - (4)].ast), (yyvsp[(3) - (4)].ast), yyloc.first_line);
                                                    COUT << "Stream Resolved!" << ENDL;
                                                ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 403 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
            string s;
            s.append((yyvsp[(1) - (4)].sval)); /* string constructor leaks otherwise! */
            (yyval.bundleNode) = new BundleNode(s, (yyvsp[(3) - (4)].listNode), yyloc.first_line);
            COUT << "Bundle name: " << (yyvsp[(1) - (4)].sval) << ENDL;
            free((yyvsp[(1) - (4)].sval));
        ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 410 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
            string ns;
            ns.append((yyvsp[(1) - (6)].sval)); /* string constructor leaks otherwise! */
            string s;
            s.append((yyvsp[(3) - (6)].sval)); /* string constructor leaks otherwise! */
            (yyval.bundleNode) = new BundleNode(s, ns, (yyvsp[(5) - (6)].listNode), yyloc.first_line);
            COUT << "Bundle name: " << (yyvsp[(3) - (6)].sval)  << " in NameSpace: " << (yyvsp[(1) - (6)].sval) << ENDL;
            free((yyvsp[(1) - (6)].sval));
            free((yyvsp[(3) - (6)].sval));

                                        ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 458 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                string s;
                                                s.append((yyvsp[(1) - (3)].sval)); /* string constructor leaks otherwise! */
                                                (yyval.functionNode) = new FunctionNode(s, NULL, FunctionNode::UserDefined, yyloc.first_line);
                                                COUT << "User function: " << (yyvsp[(1) - (3)].sval) << ENDL;
                                                free((yyvsp[(1) - (3)].sval));
                                            ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 465 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                string s;
                                                s.append((yyvsp[(1) - (4)].sval)); /* string constructor leaks otherwise! */
                                                (yyval.functionNode) = new FunctionNode(s, (yyvsp[(3) - (4)].ast), FunctionNode::UserDefined, yyloc.first_line);
                                                AST *props = (yyvsp[(3) - (4)].ast);
                                                delete props;
                                                COUT << "Properties () ..." << ENDL;
                                                COUT << "User function: " << (yyvsp[(1) - (4)].sval) << ENDL;
                                                free((yyvsp[(1) - (4)].sval));
                                            ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 475 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                string ns;
                                                ns.append((yyvsp[(1) - (5)].sval)); /* string constructor leaks otherwise! */
                                                string s;
                                                s.append((yyvsp[(3) - (5)].sval)); /* string constructor leaks otherwise! */
                                                (yyval.functionNode) = new FunctionNode(s, ns, NULL, FunctionNode::UserDefined, yyloc.first_line);
                                                COUT << "Function: " << (yyvsp[(3) - (5)].sval) << " in NameSpace: " << (yyvsp[(1) - (5)].sval) << ENDL;
                                                free((yyvsp[(1) - (5)].sval));
                                                free((yyvsp[(3) - (5)].sval));
                                            ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 485 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                COUT << "Properties () ..." << ENDL;
                                                string ns;
                                                ns.append((yyvsp[(1) - (6)].sval)); /* string constructor leaks otherwise! */
                                                string s;
                                                s.append((yyvsp[(3) - (6)].sval)); /* string constructor leaks otherwise! */
                                                (yyval.functionNode) = new FunctionNode(s, ns, (yyvsp[(5) - (6)].ast), FunctionNode::UserDefined, yyloc.first_line);
                                                AST *props = (yyvsp[(5) - (6)].ast);
                                                delete props;

                                                COUT << "Function: " << (yyvsp[(3) - (6)].sval) << " in NameSpace: " << (yyvsp[(1) - (6)].sval) << ENDL;
                                                free((yyvsp[(1) - (6)].sval));
                                                free((yyvsp[(3) - (6)].sval));
                                            ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 506 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            AST *temp = new AST();
                                            AST *props = (yyvsp[(1) - (3)].ast);
                                            props->giveChildren(temp);
                                            temp->addChild((yyvsp[(2) - (3)].propertyNode));
                                            (yyval.ast) = temp;
                                            props->deleteChildren();
                                            delete props;
                                            COUT << "Ignoring semicolon!" << ENDL;
                                        ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 516 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            AST *temp = new AST();
                                            AST *props = (yyvsp[(1) - (2)].ast);
                                            props->giveChildren(temp);
                                            temp->addChild((yyvsp[(2) - (2)].propertyNode));
                                            (yyval.ast) = temp;
                                            props->deleteChildren();
                                            delete props;
                                        ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 525 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            AST *temp = new AST();
                                            temp->addChild((yyvsp[(1) - (2)].propertyNode));
                                            (yyval.ast) = temp;
                                            COUT << "Ignoring semicolon!" << ENDL;
                                        ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 531 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            AST *temp = new AST();
                                            temp->addChild((yyvsp[(1) - (1)].propertyNode));
                                            (yyval.ast) = temp;
                                        ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 539 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    string s;
                                    s.append((yyvsp[(1) - (3)].sval)); /* string constructor leaks otherwise! */
                                    (yyval.propertyNode) = new PropertyNode(s, (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                    COUT << "Property: " << (yyvsp[(1) - (3)].sval) << ENDL << "New property ... " << ENDL;
                                    free((yyvsp[(1) - (3)].sval));
                                ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 549 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = new ValueNode(yyloc.first_line);
                            COUT << "Keyword: none" << ENDL;
                        ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 553 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = (yyvsp[(1) - (1)].ast);
                            COUT << "Value expression as property value!" << ENDL;
                        ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 557 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = new BlockNode("", "" , (yyvsp[(1) - (1)].ast), yyloc.first_line);
                            AST *props = (yyvsp[(1) - (1)].ast);
                            delete props;
                            COUT << "Block as property value!" << ENDL;
                        ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 563 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = (yyvsp[(1) - (1)].listNode);
                        ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 566 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = (yyvsp[(1) - (1)].ast);
                            COUT << "List expression as property value!" << ENDL;
                        ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 577 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.listNode) = (yyvsp[(2) - (3)].listNode);
                                    COUT << "New list ... " << ENDL;
                                ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 581 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.listNode) = (yyvsp[(2) - (3)].listNode);
                                    COUT << "New list of lists ... " << ENDL;
                                ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 588 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        ListNode *list = new ListNode(NULL, yyloc.first_line);
                                        list->stealMembers((yyvsp[(1) - (3)].listNode));
                                        ListNode *oldList = (yyvsp[(1) - (3)].listNode);
                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild((yyvsp[(3) - (3)].ast));
                                        (yyval.listNode) = list;
                                        COUT << "Value expression ..." << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 599 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.listNode) = new ListNode((yyvsp[(1) - (1)].ast), yyloc.first_line);
                                        COUT << "Value expression ..." << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 607 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                list->stealMembers((yyvsp[(1) - (3)].listNode));
                                                ListNode *oldList = (yyvsp[(1) - (3)].listNode);
                                                oldList->deleteChildren();
                                                delete oldList;
                                                list->addChild((yyvsp[(3) - (3)].listNode));
                                                (yyval.listNode) = list;
                                            ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 616 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                                (yyval.listNode) = new ListNode((yyvsp[(1) - (1)].listNode), yyloc.first_line);
                                            ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 626 "..\\..\\streamstack\\parser\\lang_stride.y"
    { (yyval.listNode) = (yyvsp[(2) - (3)].listNode); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 627 "..\\..\\streamstack\\parser\\lang_stride.y"
    { (yyval.listNode) = (yyvsp[(2) - (3)].listNode); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 628 "..\\..\\streamstack\\parser\\lang_stride.y"
    { (yyval.listNode) = (yyvsp[(2) - (3)].listNode); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 632 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        ListNode *list = new ListNode(NULL, yyloc.first_line);
                                        list->stealMembers((yyvsp[(1) - (3)].listNode));
                                        ListNode *oldList = (yyvsp[(1) - (3)].listNode);
                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild((yyvsp[(3) - (3)].blockNode));
                                        (yyval.listNode) = list;
                                        COUT << "Block definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 643 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.listNode) = new ListNode((yyvsp[(1) - (1)].blockNode), yyloc.first_line);
                                        COUT << "Block definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 651 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        ListNode *list = new ListNode(NULL, yyloc.first_line);
                                        list->stealMembers((yyvsp[(1) - (3)].listNode));
                                        ListNode *oldList = (yyvsp[(1) - (3)].listNode);
                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild((yyvsp[(3) - (3)].streamNode));
                                        (yyval.listNode) = list;
                                        COUT << "Stream definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 662 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.listNode) = new ListNode((yyvsp[(1) - (1)].streamNode), yyloc.first_line);
                                        COUT << "Stream definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 670 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    ListNode *list = new ListNode(NULL, yyloc.first_line);
                                    list->stealMembers((yyvsp[(1) - (3)].listNode));
                                    ListNode *oldList = (yyvsp[(1) - (3)].listNode);
                                    oldList->deleteChildren();
                                    delete oldList;
                                    list->addChild((yyvsp[(3) - (3)].listNode));
                                    (yyval.listNode) = list;
                                    COUT << "List of lists ..." << ENDL;
                                    COUT << "New list item ... " << ENDL;
                                ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 681 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.listNode) = new ListNode((yyvsp[(1) - (1)].listNode), yyloc.first_line);
                                    COUT << "List of lists ..." << ENDL;
                                    COUT << "New list item ... " << ENDL;
                                ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 693 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Resolving Index List Element ..." << ENDL;
                                        ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 696 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Resolving Index List Element ..." << ENDL;
                                        ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 699 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
            ListNode *list = new ListNode(NULL, yyloc.first_line);
            list->addChild((yyvsp[(1) - (1)].ast));
            (yyval.listNode) = list;
            COUT << "Resolving Index List Element ..." << ENDL;
        ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 705 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
            ListNode *list = new ListNode(NULL, yyloc.first_line);
            list->addChild((yyvsp[(1) - (1)].rangeNode));
            (yyval.listNode) = list;
            COUT << "Resolving Index List Range ..." << ENDL;
        ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 714 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
        (yyval.rangeNode) = new RangeNode((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
        COUT << "Resolving Index Range ..." << ENDL;
    ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 725 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.ast) = new ExpressionNode(ExpressionNode::Add, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                    COUT << "Index/Size adding ... " << ENDL;
                                ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 729 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.ast) = new ExpressionNode(ExpressionNode::Subtract, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                    COUT << "Index/Size subtracting ... " << ENDL;
                                ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 733 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.ast) = new ExpressionNode(ExpressionNode::Multiply , (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                    COUT << "Index/Size multiplying ... " << ENDL;
                                ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 737 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.ast) = new ExpressionNode(ExpressionNode::Divide, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                    COUT << "Index/Size dividing ... " << ENDL;
                                ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 741 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.ast) = (yyvsp[(2) - (3)].ast);
                                    COUT << "Index/Size enclosure ..." << ENDL;
                                ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 745 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                    (yyval.ast) = (yyvsp[(1) - (1)].ast);
                                ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 755 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Add, (yyvsp[(1) - (3)].listNode), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                            COUT << "Adding ... " << ENDL;
                                        ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 759 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Subtract, (yyvsp[(1) - (3)].listNode), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                            COUT << "Subtracting ... " << ENDL;
                                        ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 763 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Multiply, (yyvsp[(1) - (3)].listNode), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                            COUT << "Multiplying ... " << ENDL;
                                        ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 767 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Divide, (yyvsp[(1) - (3)].listNode), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                            COUT << "Dividing ... " << ENDL;
                                        ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 771 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Logical AND ..." << ENDL;
                                        ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 774 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Logical OR ... " << ENDL;
                                        ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 777 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Add , (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].listNode), yyloc.first_line);
                                            COUT << "Adding ... " << ENDL;
                                        ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 781 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Subtract, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].listNode), yyloc.first_line);
                                            COUT << "Subtracting ... " << ENDL;
                                        ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 785 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Multiply, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].listNode), yyloc.first_line);
                                            COUT << "Multiplying ... " << ENDL;
                                        ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 789 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = new ExpressionNode(ExpressionNode::Divide, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].listNode), yyloc.first_line);
                                            COUT << "Dividing ... " << ENDL;
                                        ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 793 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Logical AND ..." << ENDL;
                                        ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 796 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Logical OR ... " << ENDL;
                                        ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 799 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Adding Lists ... " << ENDL;
                                        ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 802 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Subtracting Lists ... " << ENDL;
                                        ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 805 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Multiplying Lists ... " << ENDL;
                                        ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 808 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Dividing Lists ... " << ENDL;
                                        ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 811 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Logical AND Lists ... " << ENDL;
                                        ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 814 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            COUT << "Logical OR Lists ... " << ENDL;
                                        ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 817 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                            (yyval.ast) = (yyvsp[(1) - (1)].listNode);
                                        ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 827 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::Add, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                        COUT << "Adding ... " << ENDL;
                                    ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 831 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::Subtract, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                        COUT << "Subtracting ... " << ENDL;
                                    ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 835 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::Multiply, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                        COUT << "Multiplying ... " << ENDL;
                                    ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 839 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::Divide, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                        COUT << "Dividing ... " << ENDL;
                                    ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 843 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::And, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                        COUT << "Logical AND ... " << ENDL;
                                    ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 847 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::Or, (yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                        COUT << "Logical OR ... " << ENDL;
                                    ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 851 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = (yyvsp[(2) - (3)].ast);
                                        COUT << "Enclosure ..." << ENDL;
                                    ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 855 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::UnaryMinus, (yyvsp[(2) - (2)].ast), yyloc.first_line);
                                        COUT << "Unary minus ... " << ENDL;
                                    ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 859 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new ExpressionNode(ExpressionNode::LogicalNot, (yyvsp[(2) - (2)].ast), yyloc.first_line);
                                        COUT << "Logical NOT ... " << ENDL;
                                    ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 863 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = (yyvsp[(1) - (1)].ast);
                                    ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 873 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = new StreamNode((yyvsp[(1) - (3)].ast), (yyvsp[(3) - (3)].ast), yyloc.first_line);
                                    ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 876 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                                        (yyval.ast) = (yyvsp[(1) - (1)].ast);
                                    ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 886 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = new ValueNode((yyvsp[(1) - (1)].ival), yyloc.first_line);
                            COUT << "Index/Size Integer: " << (yyvsp[(1) - (1)].ival) << ENDL;
                        ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 890 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            string s;
                            s.append((yyvsp[(1) - (1)].sval)); /* string constructor leaks otherwise! */
                            (yyval.ast) = new NameNode(s, yyloc.first_line);
                            COUT << "Index/Size User variable: " << (yyvsp[(1) - (1)].sval) << ENDL;
                            free((yyvsp[(1) - (1)].sval));
                        ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 897 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
            string ns;
            ns.append((yyvsp[(1) - (3)].sval)); /* string constructor leaks otherwise! */
            string s;
            s.append((yyvsp[(3) - (3)].sval)); /* string constructor leaks otherwise! */
            (yyval.ast) = new NameNode(s, ns, yyloc.first_line);
            COUT << "Index/Size User variable: " << (yyvsp[(3) - (3)].sval) << " in NameSpace: " << (yyvsp[(1) - (3)].sval) << ENDL;
            free((yyvsp[(1) - (3)].sval));
            free((yyvsp[(3) - (3)].sval));
                        ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 907 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
            BundleNode *bundle = (yyvsp[(1) - (1)].bundleNode);
            COUT << "Resolving indexed bundle ..." << bundle->getName() << ENDL;
        ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 918 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            string s;
                            s.append((yyvsp[(1) - (1)].sval)); /* string constructor leaks otherwise! */
                            (yyval.ast) = new NameNode(s, yyloc.first_line);
                            COUT << "User variable: " << (yyvsp[(1) - (1)].sval) << ENDL;
                            COUT << "Streaming ... " << ENDL;
                            free((yyvsp[(1) - (1)].sval));
                        ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 926 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            string ns;
                            ns.append((yyvsp[(1) - (3)].sval)); /* string constructor leaks otherwise! */
                            string s;
                            s.append((yyvsp[(3) - (3)].sval)); /* string constructor leaks otherwise! */
                            (yyval.ast) = new NameNode(s, ns, yyloc.first_line);
                            COUT << "User variable: " << (yyvsp[(3) - (3)].sval) << " in NameSpace: " << (yyvsp[(1) - (3)].sval) << ENDL;
                            COUT << "Streaming ... " << ENDL;
                            free((yyvsp[(1) - (3)].sval));
                            free((yyvsp[(3) - (3)].sval));
                        ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 937 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            COUT << "Resolving indexed bundle ..." << ENDL;
                            COUT << "Streaming ... " << ENDL;
                        ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 941 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            COUT << "Resolving function definition ... " << ENDL;
                            COUT << "Streaming ... " << ENDL;
                        ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 945 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            COUT << "Resolving list definition ... " << ENDL;
                            COUT << "Streaming ... " << ENDL;
                        ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 956 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = new ValueNode((yyvsp[(1) - (1)].ival), yyloc.first_line);
                            COUT << "Integer: " << (yyvsp[(1) - (1)].ival) << ENDL;
                        ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 960 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = new ValueNode((yyvsp[(1) - (1)].fval), yyloc.first_line);
                            COUT << "Real: " << (yyvsp[(1) - (1)].fval) << ENDL; ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 963 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = new ValueNode(true, yyloc.first_line);
                            COUT << "Keyword: on" << ENDL;
                        ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 967 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = new ValueNode(false, yyloc.first_line);
                            COUT << "Keyword: off" << ENDL;
                        ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 971 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            string s;
                            s.append((yyvsp[(1) - (1)].sval)); /* string constructor leaks otherwise! */
                            (yyval.ast) = new ValueNode(s, yyloc.first_line);
                            COUT << "String: " << (yyvsp[(1) - (1)].sval) << ENDL;
                            free((yyvsp[(1) - (1)].sval));
                        ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 978 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            COUT << "Rate: streamRate" << ENDL;
                        ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 981 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            string s;
                            s.append((yyvsp[(1) - (1)].sval)); /* string constructor leaks otherwise! */
                            (yyval.ast) = new NameNode(s,yyloc.first_line);
                            COUT << "User variable: " << (yyvsp[(1) - (1)].sval) << ENDL;
                            free((yyvsp[(1) - (1)].sval));
                        ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 988 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            string ns;
                            ns.append((yyvsp[(1) - (3)].sval)); /* string constructor leaks otherwise! */
                            string s;
                            s.append((yyvsp[(3) - (3)].sval)); /* string constructor leaks otherwise! */
                            (yyval.ast) = new NameNode(s, ns, yyloc.first_line);
                            COUT << "User variable: " << (yyvsp[(3) - (3)].sval) << " in NameSpace: " << (yyvsp[(1) - (3)].sval) << ENDL;
                            free((yyvsp[(1) - (3)].sval));
                            free((yyvsp[(3) - (3)].sval));
                        ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 998 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            (yyval.ast) = (yyvsp[(1) - (1)].bundleNode);
                            COUT << "Resolving indexed bundle ..." << ENDL;
                        ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1002 "..\\..\\streamstack\\parser\\lang_stride.y"
    {
                            COUT << "Resolving function definition ..." << ENDL;
                        ;}
    break;



/* Line 1455 of yacc.c  */
#line 3125 "..\\..\\streamstack\\parser\\lang_stride.parser.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1007 "..\\..\\streamstack\\parser\\lang_stride.y"


void yyerror(const char *s, ...){
    va_list ap;
    va_start(ap, s);
//    const char *errorString = va_arg(ap, char*);
    int line = va_arg(ap, int);
    COUT << ENDL << ENDL << "ERROR: " << s ; //<< " => " << errorString << ENDL;
    COUT << "Unexpected token: \"" << yytext << "\" on line: " <<  line << ENDL;
    va_end(ap);
    LangError newError;
    newError.type = LangError::Syntax;
    newError.errorTokens.push_back(std::string(yytext));
    newError.lineNumber = line;
    parseErrors.push_back(newError);
}

std::vector<LangError> getErrors() {
    return parseErrors;
}

AST *parse(const char *filename){
    FILE * file;
    AST *ast = NULL;
    char const * fileName = filename;

    char *lc;
    if (!(lc =setlocale (LC_ALL, "C"))) {
        COUT << "Error C setting locale.";
    }

    parseErrors.clear();
    file = fopen(fileName, "r");

    if (!file){
        COUT << "Can't open " << fileName << ENDL;;
        return NULL;
    }
	
    COUT << "Analysing: " << fileName << ENDL;
    COUT << "===========" << ENDL;

    tree_head = new AST;
    yyin = file;
    yylineno = 1;
    yyparse();

    if (parseErrors.size() > 0){
        COUT << ENDL << "Number of Errors: " << parseErrors.size() << ENDL;
        tree_head->deleteChildren();
        delete tree_head;
        return ast;
    }
    ast = tree_head;
    COUT << "Completed Analysing: " << fileName << ENDL;
    return ast;
}


