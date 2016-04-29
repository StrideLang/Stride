%{
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

const char * currentFile;

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

%}

%code requires { #include "ast.h" }
%code requires { #include "platformnode.h" }
%code requires { #include "blocknode.h" }
%code requires { #include "streamnode.h" }
%code requires { #include "propertynode.h" }
%code requires { #include "bundlenode.h" }
%code requires { #include "valuenode.h" }
%code requires { #include "namenode.h" }
%code requires { #include "functionnode.h" }
%code requires { #include "expressionnode.h" }
%code requires { #include "listnode.h" }
%code requires { #include "importnode.h" }
%code requires { #include "fornode.h" }
%code requires { #include "rangenode.h" }
%code requires { #include "keywordnode.hpp" }

%union {
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
    KeywordNode *keywordNode;
}

/* declare types for nodes */
%type <platformNode> platformDef
%type <platformNode> languagePlatform
%type <ast> auxPlatformDef
%type <hwPlatform> targetPlatform
%type <importNode> importDef
%type <forNode> forDef
%type <ast> forPlatformDef
%type <blockNode> blockDef
%type <ast> blockType
%type <ast> properties
%type <propertyNode> property
%type <ast> propertyType
%type <streamNode> streamDef
%type <ast> streamExp
%type <ast> streamComp
%type <ast> valueComp
%type <ast> indexExp
%type <ast> indexComp
%type <bundleNode> bundleDef
%type <listNode> indexList
%type <rangeNode> indexRange
%type <functionNode> functionDef
%type <listNode> listDef
%type <listNode> valueListList
%type <listNode> valueList
%type <listNode> valueListDef
%type <listNode> blockList
%type <listNode> streamList
%type <listNode> listList 
%type <ast> valueExp
%type <ast> valueListExp


/* declare tokens */
%token 	<ival> 	INT
%token 	<fval> 	FLOAT
%token	<sval> 	UVAR
%token 	<sval> 	WORD
%token 	<sval> 	STRING
%token 	<sval> 	ERROR

%token '[' ']' '{' '}'
%token COMMA COLON SEMICOLON 
%token USE VERSION WITH IMPORT AS FOR NONE ON OFF
//%token STREAMRATE

%left STREAM
%left OR
%left AND
%left '+' '-' 
%left '*' '/'
%right NOT
%right UMINUS
%left '(' ')'
%token DOT

%locations

%%

entry: 
        /*epsilon*/     {}
    | 	entry start     { COUT << ENDL << "Grabbing Next ..." << ENDL ; }
    | 	entry SEMICOLON	{ COUT << "Ignoring Semicolon!" << ENDL ; }
	;

start:
        platformDef	{
                        tree_head->addChild($1);
                        COUT << "Platform Definition Resolved!" << ENDL;
                    }
    |	importDef   {
                        tree_head->addChild($1);
                        COUT << "Import Definition Resolved!" << ENDL;
                    }
    |   forDef		{
                        tree_head->addChild($1);
                        COUT << "For Definition Resolved!" << ENDL;
                    }
    |	blockDef	{
                        tree_head->addChild($1);
                        COUT << "Block Resolved!" << ENDL;
                    }

    |	streamDef	{
                        tree_head->addChild($1);
                        COUT << "Stream Definition Resolved!" << ENDL;
                    }
    |	ERROR		{
                        yyerror("Unrecognized character", $1, yyloc.first_line);
                    }
;

// ================================= 
//	PLATFORM DEFINITION
// =================================

platformDef:
        languagePlatform targetPlatform						{
          PlatformNode *platformNode = $1;
          HwPlatform *hwPlatform = $2;
          platformNode->setHwPlatform(hwPlatform->name);
          platformNode->setHwVersion(hwPlatform->version);
          delete $2;
          $$ = platformNode;
        }
    |	languagePlatform targetPlatform	WITH auxPlatformDef {
          PlatformNode *platformNode = $1;
          HwPlatform *hwPlatform = $2;
          AST *aux = $4;
          platformNode->setHwPlatform(hwPlatform->name);
          platformNode->setHwVersion(hwPlatform->version);
          vector<AST *> children = aux->getChildren();
          platformNode->setChildren(children);
          delete $2;
          delete $4;
          $$ = platformNode;
        }
    ;

languagePlatform:
        USE UVAR                {
                                    string s;
                                    s.append($2); /* string constructor leaks otherwise! */
                                    $$ = new PlatformNode(s, -1, currentFile, yyloc.first_line);
                                    COUT << "Platform: " << $2 << ENDL << " Using latest version!" << ENDL;
                                    free($2);
                                }
    |   USE UVAR VERSION FLOAT  {
                                    string s;
                                    s.append($2); /* string constructor leaks otherwise! */
                                    $$ = new PlatformNode(s, $4, currentFile, yyloc.first_line);
                                    COUT << "Platform: " << $2 << ENDL << "Version: " << $4 << " line " << yylineno << ENDL;
                                    free($2);
                                }
    ;

targetPlatform:
        ON UVAR					{
                                     HwPlatform *hwPlatform = new HwPlatform;
                                     hwPlatform->name = $2;
                                     hwPlatform->version = -1;
                                     $$ = hwPlatform;
                                     COUT << "Target platform: " << $2 << ENDL << "Target Version: Current!" << ENDL;
                                     free($2);
                                }
    |	ON UVAR VERSION FLOAT	{
                                    HwPlatform *hwPlatform = new HwPlatform;
                                    hwPlatform->name = $2;
                                    hwPlatform->version = $4;
                                    $$ = hwPlatform;
                                    COUT << "Target platform: " << $2 << ENDL << "Target Version: " << $4 << ENDL;
                                    free($2);
                                 }
    ;

auxPlatformDef:
        UVAR                    {
          string word;
          word.append($1); /* string constructor leaks otherwise! */
          AST *temp = new AST();
          temp->addChild(new NameNode(word, currentFile, yyloc.first_line));
          $$ = temp;
          COUT << "With additional platform: " << $1 << ENDL;
          free($1);
        }
    |	auxPlatformDef AND UVAR {
          AST *temp = new AST();
          AST *aux = $1;
          aux->giveChildren(temp);
          string word;
          word.append($3); /* string constructor leaks otherwise! */
          temp->addChild(new NameNode(word, currentFile, yyloc.first_line));
          $$ = temp;
          aux->deleteChildren();
          delete aux;
          COUT << "With additional platform: " << $3 << ENDL;
          free($3);
        }
    ;

// =================================
//	IMPORT DEFINITION
// =================================

importDef:
        IMPORT WORD 		{
          string word;
          word.append($2); /* string constructor leaks otherwise! */
          $$ = new ImportNode(word, currentFile, yyloc.first_line);
          COUT << "Importing: " << $2 << ENDL;
          free($2);
        }
    |	IMPORT WORD AS WORD {
          string word;
          word.append($2); /* string constructor leaks otherwise! */
          string alias;
          alias.append($4); /* string constructor leaks otherwise! */
          $$ = new ImportNode(word, currentFile, yyloc.first_line, alias);
          COUT << "Importing: " << $2 << " as " << $4 << ENDL;
          free($2);
          free($4);
        }
    |	IMPORT UVAR AS WORD {
          string word;
          word.append($2); /* string constructor leaks otherwise! */
          string alias;
          alias.append($4); /* string constructor leaks otherwise! */
          $$ = new ImportNode(word, currentFile, yyloc.first_line, alias);
          COUT << "Importing: " << $2 << " as " << $4 << ENDL;
          free($2);
          free($4);
        }
    ;

// =================================
//	FOR DEFINITION
// =================================

forDef:
        FOR forPlatformDef		{
          AST* aux = $2;
          ForNode *fornode = new ForNode(currentFile, yyloc.first_line);
          vector<AST *> children = aux->getChildren();
          fornode->setChildren(children);
          $$ = fornode;
          delete $2;
        }
    ;

forPlatformDef:
        UVAR 					{
          string word;
          word.append($1); /* string constructor leaks otherwise! */
          AST *temp = new AST();
          temp->addChild(new NameNode(word, currentFile, yyloc.first_line));
          $$ = temp;
          COUT << "For platform: " << $1 << ENDL;
          free($1);
        }
    |	forPlatformDef AND UVAR	{
          AST *temp = new AST();
          AST *aux = $1;
          aux->giveChildren(temp);
          string word;
          word.append($3); /* string constructor leaks otherwise! */
          temp->addChild(new NameNode(word, currentFile, yyloc.first_line));
          $$ = temp;
          aux->deleteChildren();
          delete aux;
          COUT << "For platform: " << $3 << ENDL;
          free($3);
          }
    ;

// ================================= 
//	BLOCK DEFINITION
// =================================
	
blockDef: 	
         WORD UVAR blockType  		{
                                        string word;
                                        word.append($1); /* string constructor leaks otherwise! */
                                        string uvar;
                                        uvar.append($2); /* string constructor leaks otherwise! */
                                        $$ = new BlockNode(uvar, word, $3, currentFile, yyloc.first_line);
                                        AST *props = $3;
//                                        props->deleteChildren();
                                        delete props;
                                        COUT << "Block: " << $1 << ", Labelled: " << $2 << ENDL;
                                        free($1);
                                        free($2);
                                    }
      | WORD UVAR '[' indexExp ']' blockType    {
             string name;
             name.append($2); /* string constructor leaks otherwise! */
             ListNode *list = new ListNode($4, currentFile, yyloc.first_line);
             BundleNode *bundle = new BundleNode(name, list, currentFile, yyloc.first_line);
             COUT << "Bundle name: " << name << ENDL;
             string type;
             type.append($1); /* string constructor leaks otherwise! */
             $$ = new BlockNode(bundle, type, $6, currentFile, yyloc.first_line);
             COUT << "Block Bundle: " << $1 << ", Labelled: " << $2 << ENDL;
             free($2);
             free($1);
             AST *propertyContainer = $6;
             delete propertyContainer;
         }
	;

blockType: 	
        '{' '}'             {
                                $$ = NULL;
                                COUT << "Default values assigned!" << ENDL;
                            }
    |	'{' properties '}' 	{
                                $$ = $2;
                            }
	;

// ================================= 
//	STREAM DEFINITION
// =================================
	
streamDef:
        valueExp STREAM streamExp SEMICOLON  	{
                                                    $$ = new StreamNode($1, $3, currentFile, yyloc.first_line);
                                                    COUT << "Stream Resolved!" << ENDL;
                                                }
    |	valueListExp STREAM streamExp SEMICOLON	{
                                                    $$ = new StreamNode($1, $3, currentFile, yyloc.first_line);
                                                    COUT << "Stream Resolved!" << ENDL;
                                                }
    ;

	
// ================================= 
//	BUNDLE DEFINITION
// =================================

bundleDef:
        UVAR '[' indexList ']'          {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new BundleNode(s, $3, currentFile, yyloc.first_line);
            COUT << "Bundle name: " << $1 << ENDL;
            free($1);
        }
    |	WORD DOT UVAR '[' indexList ']'	{
            string ns;
            ns.append($1); /* string constructor leaks otherwise! */
            string s;
            s.append($3); /* string constructor leaks otherwise! */
            $$ = new BundleNode(s, ns, $5, currentFile, yyloc.first_line);
            COUT << "Bundle name: " << $3  << " in NameSpace: " << $1 << ENDL;
            free($1);
            free($3);

                                        }
//bundleDef:
//        UVAR '[' indexExp ']'           {
//                                            string s;
//                                            s.append($1); /* string constructor leaks otherwise! */
//                                            $$ = new BundleNode(s, $3, yyloc.first_line);
//                                            COUT << "Bundle name: " << $1 << ENDL;
//                                            free($1);
//                                        }
//    |	WORD DOT UVAR '[' indexExp ']'	{
//                                            COUT << "Bundle name: " << $3  << " in NameSpace: " << $1 << ENDL;
//                                            free($3);
//                                        }
//	;

// =================================
//	ARRAY RANGE ACCESS
// =================================

//bundleRangeDef:
//        UVAR '[' indexExp COLON indexExp']'             {
//                                                            string s;
//                                                            s.append($1); /* string constructor leaks otherwise! */
//                                                            $$ = new BundleNode(s, $3, $5, yyloc.first_line);
//                                                            COUT << "Bundle Range name: " << $1 << ENDL;
//                                                            free($1);
//                                                        }
//    |	WORD DOT UVAR '[' indexExp COLON indexExp']'	{
//                                                            COUT << "Bundle name: " << $3 << " in NameSpace: " << $1 << ENDL;
//                                                            free($3);
//                                                        }
//    ;

// ================================= 
//	FUNCTION DEFINITION
// =================================

functionDef:
        UVAR '(' ')'                        {
                                                string s;
                                                s.append($1); /* string constructor leaks otherwise! */
                                                $$ = new FunctionNode(s, NULL, FunctionNode::UserDefined, currentFile, yyloc.first_line);
                                                COUT << "User function: " << $1 << ENDL;
                                                free($1);
                                            }
    |	UVAR '(' properties ')'             {
                                                string s;
                                                s.append($1); /* string constructor leaks otherwise! */
                                                $$ = new FunctionNode(s, $3, FunctionNode::UserDefined, currentFile, yyloc.first_line);
                                                AST *props = $3;
                                                delete props;
                                                COUT << "Properties () ..." << ENDL;
                                                COUT << "User function: " << $1 << ENDL;
                                                free($1);
                                            }
    |	WORD DOT UVAR '(' ')'               {
                                                string ns;
                                                ns.append($1); /* string constructor leaks otherwise! */
                                                string s;
                                                s.append($3); /* string constructor leaks otherwise! */
                                                $$ = new FunctionNode(s, NULL, FunctionNode::UserDefined, currentFile, yyloc.first_line, ns);
                                                COUT << "Function: " << $3 << " in NameSpace: " << $1 << ENDL;
                                                free($1);
                                                free($3);
                                            }
    |	WORD DOT UVAR '(' properties ')' 	{
                                                COUT << "Properties () ..." << ENDL;
                                                string ns;
                                                ns.append($1); /* string constructor leaks otherwise! */
                                                string s;
                                                s.append($3); /* string constructor leaks otherwise! */
                                                $$ = new FunctionNode(s, $5, FunctionNode::UserDefined, currentFile, yyloc.first_line, ns);
                                                AST *props = $5;
                                                delete props;

                                                COUT << "Function: " << $3 << " in NameSpace: " << $1 << ENDL;
                                                free($1);
                                                free($3);
                                            }
    ;
	
// ================================= 
//	PROPERTIES DEFINITION
// =================================
	
properties: 	
        properties property SEMICOLON   {
//                                            AST *temp = new AST();
                                            AST *props = $1;
//                                            props->giveChildren(temp);
                                            props->addChild($2);
                                            $$ = props;
//                                            props->deleteChildren();
//                                            delete props;
                                            COUT << "Ignoring semicolon!" << ENDL;
                                        }
    |	properties property             {
//                                            AST *temp = new AST();
                                            AST *props = $1;
//                                            props->giveChildren(temp);
                                            props->addChild($2);
                                            $$ = props;
//                                            props->deleteChildren();
//                                            delete props;
                                        }
    |	property SEMICOLON              {
                                            AST *temp = new AST();
                                            temp->addChild($1);
                                            $$ = temp;
                                            COUT << "Ignoring semicolon!" << ENDL;
                                        }
    |	property                        {
                                            AST *temp = new AST();
                                            temp->addChild($1);
                                            $$ = temp;
                                        }
	;
	
property: 	
        WORD COLON propertyType {
                                    string s;
                                    s.append($1); /* string constructor leaks otherwise! */
                                    $$ = new PropertyNode(s, $3, currentFile, yyloc.first_line);
                                    COUT << "Property: " << $1 << ENDL << "New property ... " << ENDL;
                                    free($1);
                                }
	;
	
propertyType:
        NONE            {
                            $$ = new ValueNode(currentFile, yyloc.first_line);
                            COUT << "Keyword: none" << ENDL;
                        }
    |   valueExp		{
                            $$ = $1;
                            COUT << "Value expression as property value!" << ENDL;
                        }
    |	blockType		{
                            $$ = new BlockNode("", "" , $1, currentFile, yyloc.first_line);
                            AST *props = $1;
                            delete props;
                            COUT << "Block as property value!" << ENDL;
                        }
    |	listDef			{
                            $$ = $1;
                        }
    |	valueListExp	{
                            $$ = $1;
                            COUT << "List expression as property value!" << ENDL;
                        }
     |	streamDef		{
                            $$ = $1;
                            COUT << "Stream as property value!" << ENDL;
                        }
	;

// =================================
//	VALUE LIST DEFINITION
// =================================

valueListDef:
        '[' valueList ']'       {
                                    $$ = $2;
                                    COUT << "New list ... " << ENDL;
                                }
    |	'[' valueListList ']'	{
                                    $$ = $2;
                                    COUT << "New list of lists ... " << ENDL;
                                }
    |	'[' ']'	{
                                    $$ = new ListNode(NULL, currentFile, yyloc.first_line);
                                    COUT << "New empty list ...  " << ENDL;
                                }
    ;

valueList:
        valueList COMMA valueExp	{
                                        ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
//                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($3);
                                        $$ = list;
                                        COUT << "Value expression ..." << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
    |	valueExp                    {
                                        $$ = new ListNode($1, currentFile, yyloc.first_line);
                                        COUT << "Value expression ..." << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
    ;

valueListList:
        valueListList COMMA valueListDef    {
                                                ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                                list->stealMembers($1);
                                                ListNode *oldList = $1;
//                                                oldList->deleteChildren();
                                                delete oldList;
                                                list->addChild($3);
                                                $$ = list;
                                            }
    |	valueListDef                        {
                                                $$ = new ListNode($1, currentFile, yyloc.first_line);
                                            }
    ;

// ================================= 
//	LIST DEFINITION
// =================================

listDef:
        '[' blockList  ']'	{ $$ = $2; }
    |	'[' streamList ']'	{ $$ = $2; }
    |	'[' listList   ']'	{ $$ = $2; }
	;
	
blockList:
        blockList COMMA blockDef	{
                                        ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
//                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($3);
                                        $$ = list;
                                        COUT << "Block definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
      |  blockList blockDef	{
                                        ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
//                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($2);
                                        $$ = list;
                                        COUT << "Block definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
    |	blockDef                    {
                                        $$ = new ListNode($1, currentFile, yyloc.first_line);
                                        COUT << "Block definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
	;
	
streamList:
        streamList COMMA streamDef	{
                                        ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
//                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($3);
                                        $$ = list;
                                        COUT << "Stream definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
    |   streamList streamDef        {
                                        ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
//                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($2);
                                        $$ = list;
                                        COUT << "Stream definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
    |	streamDef                   {
                                        $$ = new ListNode($1, currentFile, yyloc.first_line);
                                        COUT << "Stream definition ... " << ENDL;
                                        COUT << "New list item ... " << ENDL;
                                    }
	;
	
listList:
        listList COMMA listDef  {
                                    ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                    list->stealMembers($1);
                                    ListNode *oldList = $1;
//                                    oldList->deleteChildren();
                                    delete oldList;
                                    list->addChild($3);
                                    $$ = list;
                                    COUT << "List of lists ..." << ENDL;
                                    COUT << "New list item ... " << ENDL;
                                }
    |	listDef                 {
                                    $$ = new ListNode($1, currentFile, yyloc.first_line);
                                    COUT << "List of lists ..." << ENDL;
                                    COUT << "New list item ... " << ENDL;
                                }
	;
	
// =================================
//	BUNDLE INDEX DEFINITION
// =================================

indexList:
        indexList COMMA indexExp		{
                                           ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                           list->stealMembers($1);
                                           list->addChild($3);
                                           $$ = list;
                                           delete $1;
                                           COUT << "Resolving Index List Element ..." << ENDL;
        }
    |	indexList COMMA indexRange		{
                                            COUT << "Resolving Index List Element ..." << ENDL;
                                            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
                                            list->stealMembers($1);
                                            list->addChild($3);
                                            $$ = list;
                                            delete $1;
                                        }
    |	indexExp						{
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->addChild($1);
            $$ = list;
            COUT << "Resolving Index List Element ..." << ENDL;
        }
    |	indexRange						{
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->addChild($1);
            $$ = list;
            COUT << "Resolving Index List Range ..." << ENDL;
        }
    ;

indexRange:
    indexExp COLON indexExp				{
        $$ = new RangeNode($1, $3, currentFile, yyloc.first_line);
        COUT << "Resolving Index Range ..." << ENDL;
    }
    ;

// ================================= 
//	INDEX EXPRESSION
// =================================
	
indexExp:
        indexExp '+' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, currentFile, yyloc.first_line);
                                    COUT << "Index/Size adding ... " << ENDL;
                                }
    |	indexExp '-' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, currentFile, yyloc.first_line);
                                    COUT << "Index/Size subtracting ... " << ENDL;
                                }
    |	indexExp '*' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Multiply , $1, $3, currentFile, yyloc.first_line);
                                    COUT << "Index/Size multiplying ... " << ENDL;
                                }
    |	indexExp '/' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, currentFile, yyloc.first_line);
                                    COUT << "Index/Size dividing ... " << ENDL;
                                }
    |	'(' indexExp ')'        {
                                    $$ = $2;
                                    COUT << "Index/Size enclosure ..." << ENDL;
                                }
    |	indexComp               {
                                    $$ = $1;
                                }
	;

// ================================= 
//	VALUE LIST EXPRESSION
// =================================

valueListExp:
        valueListDef '+' valueExp       {
                                            $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Adding ... " << ENDL;
                                        }
    |	valueListDef '-' valueExp       {
                                            $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Subtracting ... " << ENDL;
                                        }
    |	valueListDef '*' valueExp       {
                                            $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Multiplying ... " << ENDL;
                                        }
    |	valueListDef '/' valueExp       {
                                            $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Dividing ... " << ENDL;
                                        }
    |	valueListDef AND valueExp       {
                                            COUT << "Logical AND ..." << ENDL;
                                        }
    |	valueListDef OR valueExp        {
                                            COUT << "Logical OR ... " << ENDL;
                                        }
    |	valueExp '+' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Add , $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Adding ... " << ENDL;
                                        }
    |	valueExp '-' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Subtracting ... " << ENDL;
                                        }
    |	valueExp '*' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Multiplying ... " << ENDL;
                                        }
    |	valueExp '/' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, currentFile, yyloc.first_line);
                                            COUT << "Dividing ... " << ENDL;
                                        }
    |	valueExp AND valueListDef       {
                                            COUT << "Logical AND ..." << ENDL;
                                        }
    |	valueExp OR valueListDef        {
                                            COUT << "Logical OR ... " << ENDL;
                                        }
    |	valueListDef '+' valueListDef   {
                                            COUT << "Adding Lists ... " << ENDL;
                                        }
    |	valueListDef '-' valueListDef	{
                                            COUT << "Subtracting Lists ... " << ENDL;
                                        }
    |	valueListDef '*' valueListDef	{
                                            COUT << "Multiplying Lists ... " << ENDL;
                                        }
    |	valueListDef '/' valueListDef	{
                                            COUT << "Dividing Lists ... " << ENDL;
                                        }
    |	valueListDef AND valueListDef	{
                                            COUT << "Logical AND Lists ... " << ENDL;
                                        }
    |	valueListDef OR valueListDef	{
                                            COUT << "Logical OR Lists ... " << ENDL;
                                        }
    |	valueListDef                    {
                                            $$ = $1;
                                        }
	;

// ================================= 
//	VALUE EXPRESSION
// =================================
	
valueExp:
        valueExp '+' valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, currentFile, yyloc.first_line);
                                        COUT << "Adding ... " << ENDL;
                                    }
    |	valueExp '-' valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, currentFile, yyloc.first_line);
                                        COUT << "Subtracting ... " << ENDL;
                                    }
    |	valueExp '*' valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, currentFile, yyloc.first_line);
                                        COUT << "Multiplying ... " << ENDL;
                                    }
    |	valueExp '/' valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, currentFile, yyloc.first_line);
                                        COUT << "Dividing ... " << ENDL;
                                    }
    |	valueExp AND valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::And, $1, $3, currentFile, yyloc.first_line);
                                        COUT << "Logical AND ... " << ENDL;
                                    }
    |	valueExp OR valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Or, $1, $3, currentFile, yyloc.first_line);
                                        COUT << "Logical OR ... " << ENDL;
                                    }
    |	'(' valueExp ')'            {
                                        $$ = $2;
                                        COUT << "Enclosure ..." << ENDL;
                                    }
    | 	'-' valueExp %prec UMINUS 	{
                                        $$ = new ExpressionNode(ExpressionNode::UnaryMinus, $2, currentFile, yyloc.first_line);
                                        COUT << "Unary minus ... " << ENDL;
                                    }
    | 	NOT valueExp %prec NOT 		{
                                        $$ = new ExpressionNode(ExpressionNode::LogicalNot, $2, currentFile, yyloc.first_line);
                                        COUT << "Logical NOT ... " << ENDL;
                                    }
    | 	valueComp                   {
                                        $$ = $1;
                                    }
	;

// ================================= 
//	STREAM EXPRESSION
// =================================
	
streamExp:
		streamComp STREAM streamExp	{
                                        $$ = new StreamNode($1, $3, currentFile, yyloc.first_line);
                                    }
    |	streamComp                  {
                                        $$ = $1;
                                    }
	;

// ================================= 
//	INDEX COMPONENTS
// =================================
	
indexComp:
        INT             {
                            $$ = new ValueNode($1, currentFile, yyloc.first_line);
                            COUT << "Index/Size Integer: " << $1 << ENDL;
                        }
    |	UVAR            {
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            $$ = new NameNode(s, currentFile, yyloc.first_line);
                            COUT << "Index/Size User variable: " << $1 << ENDL;
                            free($1);
                        }
    |	WORD DOT UVAR	{
            string ns;
            ns.append($1); /* string constructor leaks otherwise! */
            string s;
            s.append($3); /* string constructor leaks otherwise! */
            $$ = new NameNode(s, ns, currentFile, yyloc.first_line);
            COUT << "Index/Size User variable: " << $3 << " in NameSpace: " << $1 << ENDL;
            free($1);
            free($3);
                        }
    |	bundleDef       {
            BundleNode *bundle = $1;
            COUT << "Resolving indexed bundle ..." << bundle->getName() << ENDL;
        }
	;

// ================================= 
//	STREAM COMPONENTS
// =================================
	
streamComp:
        UVAR            {
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            $$ = new NameNode(s, currentFile, yyloc.first_line);
                            COUT << "User variable: " << $1 << ENDL;
                            COUT << "Streaming ... " << ENDL;
                            free($1);
                        }
    |	WORD DOT UVAR	{
                            string ns;
                            ns.append($1); /* string constructor leaks otherwise! */
                            string s;
                            s.append($3); /* string constructor leaks otherwise! */
                            $$ = new NameNode(s, ns, currentFile, yyloc.first_line);
                            COUT << "User variable: " << $3 << " in NameSpace: " << $1 << ENDL;
                            COUT << "Streaming ... " << ENDL;
                            free($1);
                            free($3);
                        }
    |	bundleDef       {
                            COUT << "Resolving indexed bundle ..." << ENDL;
                            COUT << "Streaming ... " << ENDL;
                        }
	|	functionDef     {
                            COUT << "Resolving function definition ... " << ENDL;
                            COUT << "Streaming ... " << ENDL;
                        }
    |	valueListDef	{
                            COUT << "Resolving list definition ... " << ENDL;
                            COUT << "Streaming ... " << ENDL;
                        }
	;

// ================================= 
//	VALUE COMPONENTS
// =================================

valueComp:
        INT             {
                            $$ = new ValueNode($1, currentFile, yyloc.first_line);
                            COUT << "Integer: " << $1 << ENDL;
                        }
    |	FLOAT           {
                            $$ = new ValueNode($1, currentFile, yyloc.first_line);
                            COUT << "Real: " << $1 << ENDL; }
    |	ON              {
                            $$ = new ValueNode(true, currentFile, yyloc.first_line);
                            COUT << "Keyword: on" << ENDL;
                        }
    |	OFF             {
                            $$ = new ValueNode(false, currentFile, yyloc.first_line);
                            COUT << "Keyword: off" << ENDL;
                        }
    |	STRING			{
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            $$ = new ValueNode(s, currentFile, yyloc.first_line);
                            COUT << "String: " << $1 << ENDL;
                            free($1);
                        }
    |	WORD            {
                            string s;
                            s.append($1);
                            $$ = new KeywordNode(s, currentFile, yyloc.first_line);
                            COUT << "Word: " << $1 <<  ENDL;
                            free($1);
                        }
    |	UVAR            {
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            $$ = new NameNode(s, currentFile, yyloc.first_line);
                            COUT << "User variable: " << $1 << ENDL;
                            free($1);
                        }
    |	WORD DOT UVAR	{
                            string ns;
                            ns.append($1); /* string constructor leaks otherwise! */
                            string s;
                            s.append($3); /* string constructor leaks otherwise! */
                            $$ = new NameNode(s, ns, currentFile, yyloc.first_line);
                            COUT << "User variable: " << $3 << " in NameSpace: " << $1 << ENDL;
                            free($1);
                            free($3);
                        }
    |	bundleDef       {
                            $$ = $1;
                            COUT << "Resolving indexed bundle ..." << ENDL;
                        }
	|	functionDef     {
                            $$ = $1;
                            COUT << "Resolving function definition ..." << ENDL;
                        }
	;
	
%%

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
//    newError.lineNumber = line;
    newError.lineNumber = yylineno;
    parseErrors.push_back(newError);
}

std::vector<LangError> getErrors() {
    return parseErrors;
}

AST *parse(const char *filename){
    FILE * file;
    AST *ast = NULL;

    char *lc;
    if (!(lc =setlocale (LC_ALL, "C"))) {
        COUT << "Error C setting locale.";
    }

    parseErrors.clear();
    file = fopen(filename, "r");

    if (!file){
        LangError newError;
        newError.type = LangError::SystemError;
        newError.errorTokens.push_back("File Not Found.");
        newError.errorTokens.push_back(std::string(filename));
    //    newError.lineNumber = line;
        newError.lineNumber = yylineno;
        parseErrors.push_back(newError);
        COUT << "Can't open " << filename << ENDL;;
        return NULL;
    }
    currentFile = filename;
	
    COUT << "Analysing: " << filename << ENDL;
    COUT << "===========" << ENDL;

    tree_head = new AST;
    yyin = file;
    yylineno = 1;
    yyparse();
    fclose(file);

    if (parseErrors.size() > 0){
        COUT << ENDL << "Number of Errors: " << parseErrors.size() << ENDL;
        tree_head->deleteChildren();
        delete tree_head;
        return ast;
    }
    ast = tree_head;
    COUT << "Completed Analysing: " << filename << ENDL;
    return ast;
}

