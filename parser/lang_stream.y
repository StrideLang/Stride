%{
#include <iostream>

#include <cstdio>  // for fopen
#include <cstdarg>  //for var args

#include "ast.h"

using namespace std;

extern "C" int yylex();
extern "C" int yylineno;
extern "C" char * yytext;
extern "C" FILE *yyin;

void yyerror(const char *s, ...);

AST *tree_head;

AST *parse(const char *filename);

int error = 0;

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

%union {
	int 	ival;
    double 	fval;
    char *	sval;
    AST  *  ast;
    PlatformNode *platformNode;
    BlockNode *blockNode;
    StreamNode *streamNode;
    PropertyNode *propertyNode;
    BundleNode *bundleNode;
    FunctionNode *functionNode;
    ListNode *listNode;
    ExpressionNode *expressionNode;
}

/* declare types for nodes */
%type <platformNode> platformDef        // THIS NEEDS TO CHANGE
%type <platformNode> languagePlatform   // THIS NEEDS TO CHANGE
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
%type <bundleNode> bundleRangeDef
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
%token STREAMRATE

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
    | 	entry start     { cout << endl << "Grabbing Next ..." << endl ; }
	| 	entry SEMICOLON	{ cout << "Ignoring Semicolon!" << endl ; }	
	;

start:
        platformDef	{
                        tree_head->addChild($1);
                        cout << "Platform Definition Resolved!" << endl;
                    }
    |	importDef   {
                        cout << "Import Definition Resolved!" << endl;
                    }
    |   forDef		{
                        cout << "For Definition Resolved!" << endl;
                    }
    |	blockDef	{
                        tree_head->addChild($1);
                        cout << "Block Resolved!" << endl;
                    }

    |	streamDef	{
                        tree_head->addChild($1);
                        cout << "Stream Definition Resolved!" << endl;
                    }
    |	ERROR		{
                        yyerror("Unrecognised Character: ", $1);
                    }
;

// ================================= 
//	PLATFORM DEFINITION
// =================================

platformDef:
        languagePlatform targetPlatform						{}
    |	languagePlatform targetPlatform	WITH auxPlatformDef {}
    ;

languagePlatform:
        USE UVAR                {
                                    string s;
                                    s.append($2); /* string constructor leaks otherwise! */
                                    $$ = new PlatformNode(s, -1, yyloc.first_line);
                                    cout << "Platform: " << $2 << endl << " Using latest version!" << endl;
                                    free($2);
                                }
    |   USE UVAR VERSION FLOAT  {
                                    string s;
                                    s.append($2); /* string constructor leaks otherwise! */
                                    $$ = new PlatformNode(s, $4, yyloc.first_line);
                                    cout << "Platform: " << $2 << endl << "Version: " << $4 << " line " << yylineno << endl;
                                    free($2);
                                }
    ;

targetPlatform:
        ON UVAR					{ cout << "Target platform: " << $2 << endl << "Target Version: Current!" << endl; }
    |	ON UVAR VERSION FLOAT	{ cout << "Target platform: " << $2 << endl << "Target Version: " << $4 << endl; }
    ;

auxPlatformDef:
        UVAR                    { cout << "With additional platform: " << $1 << endl; }
    |	auxPlatformDef AND UVAR { cout << "With additional platform: " << $3 << endl; }
    ;

// =================================
//	IMPORT DEFINITION
// =================================

importDef:
        IMPORT WORD 		{ cout << "Importing: " << $2 << endl; }
    |	IMPORT WORD AS WORD { cout << "Importing: " << $2 << " as " << $4 << endl; }
    |	IMPORT UVAR AS WORD { cout << "Importing: " << $2 << " as " << $4 << endl; }
    ;

// =================================
//	FOR DEFINITION
// =================================

forDef:
        FOR forPlatformDef		{}
    ;

forPlatformDef:
        UVAR 					{ cout << "For platform: " << $1 << endl; }
    |	forPlatformDef AND UVAR	{ cout << "For platform: " << $3 << endl; }
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
                                        $$ = new BlockNode(uvar, word, $3, yyloc.first_line);
                                        AST *props = $3;
                                        delete props;
                                        cout << "Block: " << $1 << ", Labelled: " << $2 << endl;
                                        free($1);
                                        free($2);
                                    }
    |	WORD bundleDef blockType	{
                                        string s;
                                        s.append($1); /* string constructor leaks otherwise! */
                                        $$ = new BlockNode($2, s, $3, yyloc.first_line);
                                        AST *props = $3;
                                        delete props;
                                        cout << "Block Bundle ..." << endl;
                                        free($1);
                                    }
	;

blockType: 	
        '{' '}'             {
                                $$ = NULL;
                                cout << "Default values assigned!" << endl;
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
                                                    $$ = new StreamNode($1, $3, yyloc.first_line);
                                                    cout << "Stream Resolved!" << endl;
                                                }
    |	valueListExp STREAM streamExp SEMICOLON	{
                                                    $$ = new StreamNode($1, $3, yyloc.first_line);
                                                    cout << "Stream Resolved!" << endl;
                                                }
    ;

	
// ================================= 
//	ARRAY DEFINITION
// =================================

bundleDef:
        UVAR '[' indexExp ']'           {
                                            string s;
                                            s.append($1); /* string constructor leaks otherwise! */
                                            $$ = new BundleNode(s, $3, yyloc.first_line);
                                            cout << "Bundle name: " << $1 << endl;
                                            free($1);
                                        }
    |	WORD DOT UVAR '[' indexExp ']'	{
                                            cout << "Bundle name: " << $3  << " in NameSpace: " << $1 << endl;
                                        }
	;

// =================================
//	ARRAY RANGE ACCESS
// =================================

bundleRangeDef:
        UVAR '[' indexExp COLON indexExp']'             {
                                                            string s;
                                                            s.append($1); /* string constructor leaks otherwise! */
                                                            $$ = new BundleNode(s, $3, $5, yyloc.first_line);
                                                            cout << "Bundle Range name: " << $1 << endl;
                                                            free($1);
                                                        }
    |	WORD DOT UVAR '[' indexExp COLON indexExp']'	{
                                                            cout << "Bundle name: " << $3 << " in NameSpace: " << $1 << endl;
                                                        }
    ;

// ================================= 
//	FUNCTION DEFINITION
// =================================

functionDef:
        UVAR '(' ')'                        {
                                                string s;
                                                s.append($1); /* string constructor leaks otherwise! */
                                                $$ = new FunctionNode(s, NULL, FunctionNode::UserDefined, yyloc.first_line);
                                                cout << "User function: " << $1 << endl;
                                                free($1);
                                            }
    |	UVAR '(' properties ')'             {
                                                string s;
                                                s.append($1); /* string constructor leaks otherwise! */
                                                $$ = new FunctionNode(s, $3, FunctionNode::UserDefined, yyloc.first_line);
                                                AST *props = $3;
                                                delete props;
                                                cout << "Properties () ..." << endl;
                                                cout << "User function: " << $1 << endl;
                                                free($1);
                                            }
    |	WORD DOT UVAR '(' ')' 				{
                                                cout << "Function: " << $3 << " in NameSpace: " << $1 << endl;
                                            }
    |	WORD DOT UVAR '(' properties ')' 	{
                                                cout << "Properties () ..." << endl;
                                                cout << "Function: " << $3 << " in NameSpace: " << $1 << endl;
                                            }
    ;
	
// ================================= 
//	PROPERTIES DEFINITION
// =================================
	
properties: 	
        properties property SEMICOLON   {
                                            AST *temp = new AST();
                                            AST *props = $1;
                                            props->giveChildren(temp);
                                            temp->addChild($2);
                                            $$ = temp;
                                            props->deleteChildren();
                                            delete props;
                                            cout << "Ignoring semicolon!" << endl;
                                        }
    |	properties property             {
                                            AST *temp = new AST();
                                            AST *props = $1;
                                            props->giveChildren(temp);
                                            temp->addChild($2);
                                            $$ = temp;
                                            props->deleteChildren();
                                            delete props;
                                        }
    |	property SEMICOLON              {
                                            AST *temp = new AST();
                                            temp->addChild($1);
                                            $$ = temp;
                                            cout << "Ignoring semicolon!" << endl;
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
                                    $$ = new PropertyNode(s, $3, yyloc.first_line);
                                    cout << "Property: " << $1 << endl << "New property ... " << endl;
                                    free($1);
                                }
	;
	
propertyType: 	
        NONE			{
                            $$ = new ValueNode(yyloc.first_line);
                            cout << "Keyword: none" << endl;
                        }
    |	valueExp		{
                            $$ = $1;
                            cout << "Value expression as property value!" << endl;
                        }
    |	blockType		{
                            $$ = new BlockNode("", "" , $1, yyloc.first_line);
                            AST *props = $1;
                            delete props;
                            cout << "Block as property value!" << endl;
                        }
    |	listDef			{
                            $$ = $1;
                        }
    |	valueListExp	{
                            $$ = $1;
                            cout << "List expression as property value!" << endl;
                        }
	;

// =================================
//	VALUE LIST DEFINITION
// =================================

valueListDef:
        '[' valueList ']'       {
                                    $$ = $2;
                                }
    |	'[' valueListList ']'	{
                                    $$ = $2;
                                }
    ;

valueList:
        valueList COMMA valueExp	{
                                        ListNode *list = new ListNode(NULL, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($3);
                                        $$ = list;
                                        cout << "Value expression ..." << endl;
                                        cout << "New list item ... " << endl;
                                    }
    |	valueExp                    {
                                        $$ = new ListNode($1, yyloc.first_line);
                                        cout << "Value expression ..." << endl;
                                        cout << "New list item ... " << endl;
                                    }
    ;

valueListList:
        valueListList COMMA valueListDef    {
                                                ListNode *list = new ListNode(NULL, yyloc.first_line);
                                                list->stealMembers($1);
                                                ListNode *oldList = $1;
                                                oldList->deleteChildren();
                                                delete oldList;
                                                list->addChild($3);
                                                $$ = list;
                                                cout << "List of lists ..." << endl;
                                                cout << "New list item ... " << endl;
                                            }
    |	valueListDef                        {
                                                $$ = new ListNode($1, yyloc.first_line);
                                                cout << "List of lists ..." << endl;
                                                cout << "New list item ... " << endl;
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
                                        ListNode *list = new ListNode(NULL, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($3);
                                        $$ = list;
                                        cout << "Block definition ... " << endl;
                                        cout << "New list item ... " << endl;
                                    }
    |	blockDef                    {
                                        $$ = new ListNode($1, yyloc.first_line);
                                        cout << "Block definition ... " << endl;
                                        cout << "New list item ... " << endl;
                                    }
	;
	
streamList:
        streamList COMMA streamDef	{
                                        ListNode *list = new ListNode(NULL, yyloc.first_line);
                                        list->stealMembers($1);
                                        ListNode *oldList = $1;
                                        oldList->deleteChildren();
                                        delete oldList;
                                        list->addChild($3);
                                        $$ = list;
                                        cout << "Stream definition ... " << endl;
                                        cout << "New list item ... " << endl;
                                    }
    |	streamDef                   {
                                        $$ = new ListNode($1, yyloc.first_line);
                                        cout << "Stream definition ... " << endl;
                                        cout << "New list item ... " << endl;
                                    }
	;
	
listList:
        listList COMMA listDef  {
                                    ListNode *list = new ListNode(NULL, yyloc.first_line);
                                    list->stealMembers($1);
                                    ListNode *oldList = $1;
                                    oldList->deleteChildren();
                                    delete oldList;
                                    list->addChild($3);
                                    $$ = list;
                                    cout << "List of lists ..." << endl;
                                    cout << "New list item ... " << endl;
                                }
    |	listDef                 {
                                    $$ = new ListNode($1, yyloc.first_line);
                                    cout << "List of lists ..." << endl;
                                    cout << "New list item ... " << endl;
                                }
	;
	


// ================================= 
//	INDEX EXPRESSION
// =================================
	
indexExp:
        indexExp '+' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, yyloc.first_line);
                                    cout << "Index/Size adding ... " << endl;
                                }
    |	indexExp '-' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                    cout << "Index/Size subtracting ... " << endl;
                                }
    |	indexExp '*' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Multiply , $1, $3, yyloc.first_line);
                                    cout << "Index/Size multiplying ... " << endl;
                                }
    |	indexExp '/' indexExp 	{
                                    $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                    cout << "Index/Size dividing ... " << endl;
                                }
    |	'(' indexExp ')'        {
                                    $$ = $2; cout << "Index/Size enclosure ..." << endl;
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
                                            $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, yyloc.first_line);
                                            cout << "Adding ... " << endl;
                                        }
    |	valueListDef '-' valueExp       {
                                            $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                            cout << "Subtracting ... " << endl;
                                        }
    |	valueListDef '*' valueExp       {
                                            $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, yyloc.first_line);
                                            cout << "Multiplying ... " << endl;
                                        }
    |	valueListDef '/' valueExp       {
                                            $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                            cout << "Dividing ... " << endl;
                                        }
    |	valueExp '+' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Add , $1, $3, yyloc.first_line);
                                            cout << "Adding ... " << endl;
                                        }
    |	valueExp '-' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                            cout << "Subtracting ... " << endl;
                                        }
    |	valueExp '*' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, yyloc.first_line);
                                            cout << "Multiplying ... " << endl;
                                        }
    |	valueExp '/' valueListDef       {
                                            $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                            cout << "Dividing ... " << endl;
                                        }
    |	valueListDef '+' valueListDef   {
                                            cout << "Adding Lists ... " << endl;
                                        }
    |	valueListDef '-' valueListDef	{
                                            cout << "Subtracting Lists ... " << endl;
                                        }
    |	valueListDef '*' valueListDef	{
                                            cout << "Multiplying Lists ... " << endl;
                                        }
    |	valueListDef '/' valueListDef	{
                                            cout << "Dividing Lists ... " << endl;
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
                                        $$ = new ExpressionNode(ExpressionNode::Add, $1, $3, yyloc.first_line);
                                        cout << "Adding ... " << endl;
                                    }
    |	valueExp '-' valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Subtract, $1, $3, yyloc.first_line);
                                        cout << "Subtracting ... " << endl;
                                    }
    |	valueExp '*' valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Multiply, $1, $3, yyloc.first_line);
                                        cout << "Multiplying ... " << endl;
                                    }
    |	valueExp '/' valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Divide, $1, $3, yyloc.first_line);
                                        cout << "Dividing ... " << endl;
                                    }
    |	valueExp AND valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::And, $1, $3, yyloc.first_line);
                                        cout << "Logical AND ... " << endl;
                                    }
    |	valueExp OR valueExp 		{
                                        $$ = new ExpressionNode(ExpressionNode::Or, $1, $3, yyloc.first_line);
                                        cout << "Logical OR ... " << endl;
                                    }
    |	'(' valueExp ')'            {
                                        $$ = $2;
                                        cout << "Enclosure ..." << endl;
                                    }
    | 	'-' valueExp %prec UMINUS 	{
                                        $$ = new ExpressionNode(ExpressionNode::UnaryMinus, $2, yyloc.first_line);
                                        cout << "Unary minus ... " << endl;
                                    }
    | 	NOT valueExp %prec NOT 		{
                                        $$ = new ExpressionNode(ExpressionNode::LogicalNot, $2, yyloc.first_line);
                                        cout << "Logical NOT ... " << endl;
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
                                        $$ = new StreamNode($1, $3, yyloc.first_line);
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
                            $$ = new ValueNode($1, yyloc.first_line);
                            cout << "Index/Size Integer: " << $1 << endl;
                        }
    |	UVAR            {
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            $$ = new NameNode(s, yyloc.first_line);
                            cout << "Index/Size User variable: " << $1 << endl;
                            free($1);
                        }
    |	WORD DOT UVAR	{
                            cout << "Index/Size User variable: " << $3 << " in NameSpace: " << $1 << endl;
                        }
    |	bundleDef       {
                            cout << "Resolving indexed array ..." << endl;
                        }
	;

// ================================= 
//	STREAM COMPONENTS
// =================================
	
streamComp:
        UVAR            {
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            $$ = new NameNode(s, yyloc.first_line);
                            cout << "User variable: " << $1 << endl;
                            cout << "Streaming ... " << endl;
                            free($1);
                        }
    |	WORD DOT UVAR	{
                            cout << "User variable: " << $3 << " in NameSpace: " << $1 << endl;
                            cout << "Streaming ... " << endl;
                        }
    |	bundleDef       {
                            cout << "Resolving indexed array ..." << endl;
                            cout << "Streaming ... " << endl;
                        }
    |   bundleRangeDef  {
                            cout << "Resolving indexed array range ..." << endl;
                            cout << "Streaming ... " << endl;
                        }
    |	functionDef     {
                            cout << "Resolving function definition ... " << endl;
                            cout << "Streaming ... " << endl;
                        }
    |	valueListDef	{
                            cout << "Resolving list definition ... " << endl;
                            cout << "Streaming ... " << endl;
                        }
	;

// ================================= 
//	VALUE COMPONENTS
// =================================

valueComp:
        INT             {
                            $$ = new ValueNode($1, yyloc.first_line);
                            cout << "Integer: " << $1 << endl;
                        }
    |	FLOAT           {
                            $$ = new ValueNode($1, yyloc.first_line);
                            cout << "Real: " << $1 << endl; }
    |	UVAR            {
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            cout << "User variable: " << $1 << endl;
                            free($1);
                        }
    |	ON              {   $$ = new ValueNode(true, yyloc.first_line);
                            cout << "Keyword: on" << endl;
                        }
    |	OFF             {
                            $$ = new ValueNode(false, yyloc.first_line);
                            cout << "Keyword: off" << endl;
                        }
    |	STRING			{
                            string s;
                            s.append($1); /* string constructor leaks otherwise! */
                            $$ = new ValueNode(s, yyloc.first_line);
                            cout << "String: " << $1 << endl;
                            free($1);
                        }
    |	STREAMRATE      {
                            cout << "Rate: streamRate" << endl;
                        }
    |	WORD DOT UVAR	{
                            cout << "User variable: " << $3 << " in NameSpace: " << $1 << endl;
                        }
    |	bundleDef       {
                            $$ = $1;
                            cout << "Resolving indexed array ..." << endl;
                        }
    |   bundleRangeDef  {
                            cout << "Resolving indexed array range ..." << endl;
                        }
    |	functionDef     {
                            cout << "Resolving function definition ..." << endl;
                        }
	;
	
%%

void yyerror(const char *s, ...){
	va_list ap;
	va_start(ap, s);
        cout << endl << endl << "ERROR: " << s ; // << " => " << va_arg(ap, char*) << endl;
        cout << "Unexpected token: \"" << yytext << "\" on line: " <<  yylineno << endl;
	va_end(ap);
	error++;
}

AST *parse(const char *filename){
	FILE * file;
        AST *ast = NULL;
        char const * fileName = filename;

        char *lc;
        if (!(lc =setlocale (LC_ALL, "C"))) {
                cout << "Error C setting locale.";
        }

        error = 0;
        file = fopen(fileName, "r");

	if (!file){
		cout << "Can't open " << fileName << endl;;
                return NULL;
	}
	
	cout << "Analysing: " << fileName << endl;
	cout << "===========" << endl;

        tree_head = new AST;
	yyin = file;
        yylineno = 1;
	yyparse();
	
	if (error > 0){
                cout << endl << "Number of Errors: " << error << endl;
                tree_head->deleteChildren();
                delete tree_head;
                return ast;
        }
        ast = tree_head;
	cout << "Completed Analysing: " << fileName << endl;
        return ast;
}

