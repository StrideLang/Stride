%{
#include <iostream>
#include <vector>
#include <cstdio>   // For fopen
#include <cerrno>   // For error codes
#include <cstring>  // For strerror

#include "ast.h"

using namespace std;

#ifdef __GNUC__
    extern "C" int yylex();
    extern "C" FILE *yyin;
    extern "C" int yylineno;
    extern "C" char * yytext;
    extern "C" int errno;
#else
    extern "C" {
        int yylex();
        FILE *yyin;
        int yylineno;
        char * yytext;
//        int errno;        // THIS IS CAUSING A LINKING WARNING WITH MSVC 2015
    }
#endif

extern void yyrestart (FILE *input_file );

std::vector<LangError> parseErrors;

void yyerror(const char *s);

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
%code requires { #include "blocknode.h" }
%code requires { #include "bundlenode.h" }
%code requires { #include "declarationnode.h" }
%code requires { #include "expressionnode.h" }
%code requires { #include "functionnode.h" }
%code requires { #include "importnode.h" }
%code requires { #include "keywordnode.h" }
%code requires { #include "listnode.h" }
%code requires { #include "platformnode.h" }
%code requires { #include "portpropertynode.h" }
%code requires { #include "propertynode.h" }
%code requires { #include "rangenode.h" }
%code requires { #include "scopenode.h" }
%code requires { #include "streamnode.h" }
%code requires { #include "valuenode.h" }

%union {
    int     ival;
    double  fval;
    char    *sval;
    AST     *ast;
    SystemNode *systemNode;
    DeclarationNode *declarationNode;
    StreamNode *streamNode;
    PropertyNode *propertyNode;
    BundleNode *bundleNode;
    FunctionNode *functionNode;
    ExpressionNode *expressionNode;
    ListNode *listNode;
    ImportNode *importNode;
    RangeNode *rangeNode;
    KeywordNode *keywordNode;
    ScopeNode *scopeNode;
    PortPropertyNode *portPropertyNode;
}

/* declare types for nodes */
%type <systemNode> systemDef
%type <systemNode> languagePlatform
%type <importNode> importDef
%type <declarationNode> blockDef
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
%type <listNode> streamListDef
%type <listNode> valueListList
%type <listNode> valueList
%type <listNode> valueListDef
%type <listNode> blockList
%type <listNode> streamList
%type <listNode> listList
%type <ast> valueExp
%type <ast> valueListExp
%type <ast> scopeDef
%type <scopeNode> scope
%type <portPropertyNode> portPropertyDef


/* declare tokens */
%token  <ival>  INT
%token  <fval>  REAL
%token  <sval>  UVAR
%token  <sval>  WORD
%token  <sval>  STRING
%token  <sval>  ERROR

%token '[' ']' '{' '}'
%token DOT COMMA COLON COLONCOLON SEMICOLON
%token USE VERSION WITH IMPORT AS FOR NONE ON OFF
%token BITAND BITOR BITNOT
%token STREAMRATE

%left STREAM
%left AND OR
%left BITAND BITOR
%left '+' '-' 
%left '*' '/'
%right NOT
%right BITNOT
%right UMINUS
%left '(' ')'

%locations

%%

entry: 
        /*epsilon*/     {}
    |   entry start     { COUT << ENDL << "Grabbing Next ..." << ENDL ; }
    |   entry SEMICOLON { COUT << "Ignoring Semicolon!" << ENDL ; }
    ;

start:
        systemDef {
            tree_head->addChild(std::shared_ptr<SystemNode>($1));
            COUT << "System Definition Resolved!" << ENDL;
        }
    |   importDef   {
            tree_head->addChild(std::shared_ptr<ImportNode>($1));
            COUT << "Import Definition Resolved!" << ENDL;
        }
    |   blockDef    {
            tree_head->addChild(std::shared_ptr<DeclarationNode>($1));
            COUT << "Block Resolved!" << ENDL;
        }
    |   streamDef   {
            tree_head->addChild(std::shared_ptr<StreamNode>($1));
            COUT << "Stream Definition Resolved!" << ENDL;
        }
    |   ERROR       {
            COUT << "Unrecognized Character: " << $1 << ENDL;
            yyerror($1);
        }
    ;

// ================================= 
//  PLATFORM DEFINITION
// =================================

systemDef:
        languagePlatform {
            SystemNode *systemNode = $1;
            $$ = systemNode;
        }
    ;

languagePlatform:
        USE UVAR                {
            string s;
            s.append($2); /* string constructor leaks otherwise! */
            $$ = new SystemNode(s, -1, -1, currentFile, yyloc.first_line);
            COUT << "Platform: " << $2 << ENDL << " Using latest version!" << ENDL;
            free($2);
        }
    |   USE UVAR VERSION REAL  {
            string s;
            s.append($2); /* string constructor leaks otherwise! */
            int major = int($4);
            int minor = int(($4 - int($4))*10);
            $$ = new SystemNode(s, major, minor, currentFile, yyloc.first_line);
            COUT << "Platform: " << $2 << ENDL << "Version: " << $4 << " line " << yylineno << ENDL;
            free($2);
        }
    ;


// =================================
//  IMPORT DEFINITION
// =================================

importDef:
        IMPORT UVAR             {
            string word;
            word.append($2); /* string constructor leaks otherwise! */
            $$ = new ImportNode(word, NULL, currentFile, yyloc.first_line);
            COUT << "Importing: " << $2 << ENDL;
            free($2);
        }
    |   IMPORT scopeDef UVAR    {
            string word;
            word.append($3); /* string constructor leaks otherwise! */
            $$ = new ImportNode(word, std::shared_ptr<AST>($2), currentFile, yyloc.first_line);
            COUT << "Importing: " << $3 << " in scope!" << ENDL;
            free($3);
        }
    |   IMPORT UVAR AS UVAR {
            string word;
            word.append($2); /* string constructor leaks otherwise! */
            string alias;
            alias.append($4); /* string constructor leaks otherwise! */
            $$ = new ImportNode(word, NULL, currentFile, yyloc.first_line, alias);
            COUT << "Importing: " << $2 << " as " << $4 << ENDL;
            free($2);
            free($4);
        }
    |   IMPORT scopeDef UVAR AS UVAR {
            string word;
            word.append($3); /* string constructor leaks otherwise! */
            string alias;
            alias.append($5); /* string constructor leaks otherwise! */
            $$ = new ImportNode(word, std::shared_ptr<AST>($2), currentFile, yyloc.first_line, alias);
            COUT << "Importing: " << $3 << " as " << $5 << " in scope!" << ENDL;
            free($3);
            free($5);
        }
    ;

// ================================= 
//  BLOCK DEFINITION
// =================================

blockDef:
         WORD UVAR blockType                    {
            string word;
            word.append($1); /* string constructor leaks otherwise! */
            string uvar;
            uvar.append($2); /* string constructor leaks otherwise! */
            $$ = new DeclarationNode(uvar, word, std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Block: " << $1 << ", Labelled: " << $2 << ENDL;
            free($1);
            free($2);
        }
    |   WORD UVAR '[' indexExp ']' blockType    {
            string name;
            name.append($2); /* string constructor leaks otherwise! */
            std::shared_ptr<ListNode> list = std::make_shared<ListNode>(std::shared_ptr<AST>($4), currentFile, yyloc.first_line);
            std::shared_ptr<BundleNode> bundle = std::make_shared<BundleNode>(name, list, currentFile, yyloc.first_line);
            COUT << "Bundle name: " << name << ENDL;
            string type;
            type.append($1); /* string constructor leaks otherwise! */
            $$ = new DeclarationNode(bundle, type, std::shared_ptr<AST>($6), currentFile, yyloc.first_line);
            COUT << "Block Bundle: " << $1 << ", Labelled: " << $2 << ENDL;
            free($2);
            free($1);
         }
    ;

blockType:
        '{' '}'             {
            $$ = NULL;
            COUT << "Default values assigned!" << ENDL;
        }
    |   '{' properties '}'  {
            $$ = $2;
        }
    ;

// ================================= 
//  STREAM DEFINITION
// =================================

streamDef:
        valueExp STREAM streamExp SEMICOLON         {
            $$ = new StreamNode(std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Stream Resolved!" << ENDL;
        }
    |   valueListExp STREAM streamExp SEMICOLON     {
            $$ = new StreamNode(std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Stream Resolved!" << ENDL;
        }
    |   streamListDef STREAM streamExp SEMICOLON    {
            $$ = new StreamNode(std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Stream Resolved!" << ENDL;
        }
    ;

// =================================
//  SCOPE DEFINITION
// =================================

scopeDef:
        scopeDef scope  {
            AST *scope = $1;
            scope->addChild(std::shared_ptr<AST>($2));
            $$ = scope;
        }
    |   scope           {
            AST *temp = new AST();
            temp->addChild(std::shared_ptr<AST>($1));
            $$ = temp;
        }
    ;

scope:
        UVAR COLONCOLON {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new ScopeNode(s, currentFile, yyloc.first_line);
            COUT << "Scope: " << $1 << ENDL;
            free($1);
        }
    ;

// ================================= 
//  BUNDLE DEFINITION
// =================================

bundleDef:
        UVAR '[' indexList ']'          {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new BundleNode(s, std::shared_ptr<ListNode>($3), currentFile, yyloc.first_line);
            COUT << "Bundle name: " << $1 << ENDL;
            free($1);
        }
    |   scopeDef UVAR '[' indexList ']' {
            string s;
            s.append($2); /* string constructor leaks otherwise! */
            $$ = new BundleNode(s, std::shared_ptr<AST>($1), std::shared_ptr<ListNode>($4), currentFile, yyloc.first_line);
            COUT << "Bundle name: " << $2 << " in scope!" << ENDL;
            COUT << "Streaming ... " << ENDL;
            free($2);
        }
    ;

// ================================= 
//  FUNCTION DEFINITION
// =================================

functionDef:
        UVAR '(' ')'                        {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new FunctionNode(s, NULL, currentFile, yyloc.first_line);
            COUT << "User function: " << $1 << ENDL;
            free($1);
        }
    |   scopeDef UVAR '(' ')'               {
            string s;
            s.append($2);
            $$ = new FunctionNode(s, std::shared_ptr<AST>($1), NULL, currentFile, yyloc.first_line);
            COUT << "User function: " << $2 << " in scope!" << ENDL;
            free($2);
        }
    |   UVAR '(' properties ')'             {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new FunctionNode(s, std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Properties () ..." << ENDL;
            COUT << "User function: " << $1 << ENDL;
            free($1);
        }
    |   scopeDef UVAR '(' properties ')'               {
            string s;
            s.append($2);
            $$ = new FunctionNode(s, std::shared_ptr<AST>($1), std::shared_ptr<AST>($4), currentFile, yyloc.first_line);
            COUT << "Properties () ..." << ENDL;
            COUT << "User function: " << $2 << " in scope!" << ENDL;
            free($2);
        }
    ;

// ================================= 
//  PROPERTIES DEFINITION
// =================================

properties:
        properties property SEMICOLON   {
            AST *props = $1;
            props->addChild(std::shared_ptr<AST>($2));
            $$ = props;
            COUT << "Ignoring semicolon!" << ENDL;
        }
    |   properties property             {
            AST *props = $1;
            props->addChild(std::shared_ptr<AST>($2));
            $$ = props;
        }
    |   property SEMICOLON              {
            AST *temp = new AST();
            temp->addChild(std::shared_ptr<AST>($1));
            $$ = temp;
            COUT << "Ignoring semicolon!" << ENDL;
        }
    |   property                        {
            AST *temp = new AST();
            temp->addChild(std::shared_ptr<AST>($1));
            $$ = temp;
        }
    ;

property:
        WORD COLON propertyType {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new PropertyNode(s, std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Property: " << $1 << ENDL << "New property ... " << ENDL;
            free($1);
        }
    |   WORD COLON STREAMRATE   {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            PropertyNode * node = new PropertyNode(s, std::make_shared<ValueNode>((string) "streamRate", __FILE__, __LINE__), currentFile, yyloc.first_line);
            $$ = node;
            COUT << "Property: " << $1 << ENDL << "New property ... " << ENDL;
            free($1);
        }
    ;

propertyType:
        NONE                {
            $$ = new ValueNode(currentFile, yyloc.first_line);
            COUT << "Keyword: none" << ENDL;
        }
    |   valueExp            {
            $$ = $1;
            COUT << "Value expression as property value!" << ENDL;
        }
    |   blockType           {
            $$ = new DeclarationNode("", "" , std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "Block as property value!" << ENDL;
        }
    |   listDef             {
            $$ = $1;
        }
    |   valueListExp        {
            $$ = $1;
            COUT << "List expression as property value!" << ENDL;
        }
    |   streamDef           {
            $$ = $1;
            COUT << "Stream as property value!" << ENDL;
        }
    ;

// =================================
//  PORT PROPERTY DEFINITION
// =================================

portPropertyDef:
        UVAR DOT WORD   {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            string p;
            p.append($3); /* string constructor leaks otherwise! */
            $$ = new PortPropertyNode(s, p, currentFile, yyloc.first_line);
            COUT << "Port Name: " << $1 << ENDL << "Port Property: " << $3 << ENDL;
            free($1);
            free($3);
        }
    ;

// =================================
//  VALUE LIST DEFINITION
// =================================

valueListDef:
        '[' valueList ']'       {
            $$ = $2;
            COUT << "New list ... " << ENDL;
        }
    |   '[' valueListList ']'   {
            $$ = $2;
            COUT << "New list of lists ... " << ENDL;
        }
    |   '[' ']'                 {
            $$ = new ListNode(NULL, currentFile, yyloc.first_line);
            COUT << "New empty list ...  " << ENDL;
        }
    ;

valueList:
        valueList COMMA valueExp    {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            ListNode *oldList = $1;
            delete oldList;
            list->addChild(std::shared_ptr<AST>($3));
            $$ = list;
            COUT << "Value expression ..." << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    |   valueExp                    {
            $$ = new ListNode(std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "Value expression ..." << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    ;

valueListList:
        valueListList COMMA valueListDef    {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            ListNode *oldList = $1;
            delete oldList;
            list->addChild(std::shared_ptr<AST>($3));
            $$ = list;
        }
    |   valueListDef                        {
            $$ = new ListNode(std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
        }
    ;

// ================================= 
//  LIST DEFINITION
// =================================

listDef:
        '[' blockList  ']'  { $$ = $2; }
    |   streamListDef       { }
    |   '[' listList   ']'  { $$ = $2; }
    ;


streamListDef:
        '[' streamList ']'  { $$ = $2; }
    ;

blockList:
        blockList COMMA blockDef    {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            ListNode *oldList = $1;
            delete oldList;
            list->addChild(std::shared_ptr<AST>($3));
            $$ = list;
            COUT << "Block definition ... " << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    |   blockList blockDef          {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            ListNode *oldList = $1;
            delete oldList;
            list->addChild(std::shared_ptr<AST>($2));
            $$ = list;
            COUT << "Block definition ... " << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    |   blockDef                    {
            $$ = new ListNode(std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "Block definition ... " << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    ;

streamList:
        streamList COMMA streamDef  {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            ListNode *oldList = $1;
            delete oldList;
            list->addChild(std::shared_ptr<AST>($3));
            $$ = list;
            COUT << "Stream definition ... " << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    |   streamList streamDef        {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            ListNode *oldList = $1;
            delete oldList;
            list->addChild(std::shared_ptr<AST>($2));
            $$ = list;
            COUT << "Stream definition ... " << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    |   streamDef                   {
            $$ = new ListNode(std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "Stream definition ... " << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    ;

listList:
        listList COMMA listDef  {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            ListNode *oldList = $1;
            delete oldList;
            list->addChild(std::shared_ptr<AST>($3));
            $$ = list;
            COUT << "List of lists ..." << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    |   listDef                 {
            $$ = new ListNode(std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "List of lists ..." << ENDL;
            COUT << "New list item ... " << ENDL;
        }
    ;

// =================================
//  BUNDLE INDEX DEFINITION
// =================================

indexList:
        indexList COMMA indexExp        {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            list->addChild(std::shared_ptr<AST>($3));
            $$ = list;
            delete $1;
            COUT << "Resolving Index List Element ..." << ENDL;
        }
    |   indexList COMMA indexRange      {
            COUT << "Resolving Index List Element ..." << ENDL;
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->stealMembers($1);
            list->addChild(std::shared_ptr<AST>($3));
            $$ = list;
            delete $1;
        }
    |   indexExp                        {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->addChild(std::shared_ptr<AST>($1));
            $$ = list;
            COUT << "Resolving Index List Element ..." << ENDL;
        }
    |   indexRange                      {
            ListNode *list = new ListNode(NULL, currentFile, yyloc.first_line);
            list->addChild(std::shared_ptr<AST>($1));
            $$ = list;
            COUT << "Resolving Index List Range ..." << ENDL;
        }
    ;

// =================================
//  INDEX RANGE DEFINITION
// =================================

indexRange:
        indexExp COLON indexExp         {
            $$ = new RangeNode(std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Resolving Index Range ..." << ENDL;
        }
    ;

// ================================= 
//  INDEX EXPRESSION
// =================================

indexExp:
        indexExp '+' indexExp           {
            $$ = new ExpressionNode(ExpressionNode::Add, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size adding ... " << ENDL;
        }
    |   indexExp '-' indexExp           {
            $$ = new ExpressionNode(ExpressionNode::Subtract, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size subtracting ... " << ENDL;
        }
    |   indexExp '*' indexExp           {
            $$ = new ExpressionNode(ExpressionNode::Multiply, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size multiplying ... " << ENDL;
        }
    |   indexExp '/' indexExp           {
            $$ = new ExpressionNode(ExpressionNode::Divide, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size dividing ... " << ENDL;
        }
    |   indexExp BITAND indexExp        {
           $$ = new ExpressionNode(ExpressionNode::BitAnd, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise and ... " << ENDL;
        }
    |   indexExp BITOR indexExp         {
            $$ = new ExpressionNode(ExpressionNode::BitOr, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise or ... " << ENDL;
        }
    |   indexExp BITNOT indexExp        {
            $$ = new ExpressionNode(ExpressionNode::BitNot, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise not ... " << ENDL;
        }
    |   '(' indexExp ')'                {
            $$ = $2;
            COUT << "Index/Size enclosure ..." << ENDL;
        }
    |   indexComp                       {
            $$ = $1;
        }
    ;

// ================================= 
//  VALUE LIST EXPRESSION
// =================================

valueListExp:
        valueListDef '+' valueExp               {
            $$ = new ExpressionNode(ExpressionNode::Add, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Adding ... " << ENDL;
        }
    |   valueListDef '-' valueExp               {
            $$ = new ExpressionNode(ExpressionNode::Subtract, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Subtracting ... " << ENDL;
        }
    |   valueListDef '*' valueExp               {
            $$ = new ExpressionNode(ExpressionNode::Multiply, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Multiplying ... " << ENDL;
        }
    |   valueListDef '/' valueExp               {
            $$ = new ExpressionNode(ExpressionNode::Divide, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Dividing ... " << ENDL;
        }
    |   valueListDef BITAND valueExp            {
            $$ = new ExpressionNode(ExpressionNode::BitAnd , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise and ... " << ENDL;
        }
    |   valueListDef BITOR valueExp             {
            $$ = new ExpressionNode(ExpressionNode::BitOr , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise or ... " << ENDL;
        }
    |   valueListDef BITNOT valueExp            {
            $$ = new ExpressionNode(ExpressionNode::BitNot , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise not ... " << ENDL;
        }
    |   valueListDef AND valueExp               {
            $$ = new ExpressionNode(ExpressionNode::And, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical AND ..." << ENDL;
        }
    |   valueListDef OR valueExp                {
            $$ = new ExpressionNode(ExpressionNode::Or, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical OR ... " << ENDL;
        }
    |   valueExp '+' valueListDef               {
            $$ = new ExpressionNode(ExpressionNode::Add , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Adding ... " << ENDL;
        }
    |   valueExp '-' valueListDef               {
            $$ = new ExpressionNode(ExpressionNode::Subtract, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Subtracting ... " << ENDL;
        }
    |   valueExp '*' valueListDef               {
            $$ = new ExpressionNode(ExpressionNode::Multiply, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Multiplying ... " << ENDL;
        }
    |   valueExp '/' valueListDef               {
            $$ = new ExpressionNode(ExpressionNode::Divide, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Dividing ... " << ENDL;
        }
    |   valueExp BITAND valueListDef            {
            $$ = new ExpressionNode(ExpressionNode::BitAnd , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise and ... " << ENDL;
        }
    |   valueExp BITOR valueListDef             {
            $$ = new ExpressionNode(ExpressionNode::BitOr , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size bitwise or ... " << ENDL;
        }
    |   valueExp BITNOT valueListDef            {
            $$ = new ExpressionNode(ExpressionNode::BitNot , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise not ... " << ENDL;
        }
    |   valueExp AND valueListDef               {
            $$ = new ExpressionNode(ExpressionNode::And, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical AND ..." << ENDL;
        }
    |   valueExp OR valueListDef                {
            $$ = new ExpressionNode(ExpressionNode::Or, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical OR ... " << ENDL;
        }
    |   valueListDef '+' valueListDef           {
            $$ = new ExpressionNode(ExpressionNode::Add, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Adding Lists ... " << ENDL;
        }
    |   valueListDef '-' valueListDef           {
            $$ = new ExpressionNode(ExpressionNode::Subtract, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Subtracting Lists ... " << ENDL;
        }
    |   valueListDef '*' valueListDef           {
            $$ = new ExpressionNode(ExpressionNode::Multiply, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Multiplying Lists ... " << ENDL;
        }
    |   valueListDef '/' valueListDef           {
            $$ = new ExpressionNode(ExpressionNode::Divide, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Dividing Lists ... " << ENDL;
        }
    |   valueListDef BITAND valueListDef        {
            $$ = new ExpressionNode(ExpressionNode::BitAnd , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise And ... " << ENDL;
        }
    |   valueListDef BITOR valueListDef         {
            $$ = new ExpressionNode(ExpressionNode::BitOr , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise Or ... " << ENDL;
        }
    |   valueListDef BITNOT valueListDef        {
            $$ = new ExpressionNode(ExpressionNode::BitNot , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise Not ... " << ENDL;
        }
    |   valueListDef AND valueListDef           {
            $$ = new ExpressionNode(ExpressionNode::And, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical AND Lists ... " << ENDL;
        }
    |   valueListDef OR valueListDef            {
            $$ = new ExpressionNode(ExpressionNode::Or, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical OR Lists ... " << ENDL;
        }
    |   valueListDef                            {
            $$ = $1;
        }
    ;

// ================================= 
//  VALUE EXPRESSION
// =================================

valueExp:
        valueExp '+' valueExp           {
            $$ = new ExpressionNode(ExpressionNode::Add, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Adding ... " << ENDL;
        }
    |   valueExp '-' valueExp           {
            $$ = new ExpressionNode(ExpressionNode::Subtract, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Subtracting ... " << ENDL;
        }
    |   valueExp '*' valueExp           {
            $$ = new ExpressionNode(ExpressionNode::Multiply, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Multiplying ... " << ENDL;
        }
    |   valueExp '/' valueExp           {
            $$ = new ExpressionNode(ExpressionNode::Divide, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Dividing ... " << ENDL;
        }
    |   valueExp AND valueExp           {
            $$ = new ExpressionNode(ExpressionNode::And, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical AND ... " << ENDL;
        }
    |   valueExp OR valueExp            {
            $$ = new ExpressionNode(ExpressionNode::Or, std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Logical OR ... " << ENDL;
        }
    |   valueExp BITAND valueExp        {
            $$ = new ExpressionNode(ExpressionNode::BitAnd , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise and ... " << ENDL;
        }
    |   valueExp BITOR valueExp         {
            $$ = new ExpressionNode(ExpressionNode::BitOr , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise or ... " << ENDL;
        }
    |   valueExp BITNOT valueExp        {
            $$ = new ExpressionNode(ExpressionNode::BitNot , std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
            COUT << "Index/Size Bitwise not ... " << ENDL;
        }
    |   '(' valueExp ')'                {
            $$ = $2;
            COUT << "Enclosure ..." << ENDL;
        }
    |   '-' valueExp %prec UMINUS       {
            $$ = new ExpressionNode(ExpressionNode::UnaryMinus, std::shared_ptr<AST>($2), currentFile, yyloc.first_line);
            COUT << "Unary minus ... " << ENDL;
        }
    |   NOT valueExp %prec NOT          {
            $$ = new ExpressionNode(ExpressionNode::LogicalNot, std::shared_ptr<AST>($2), currentFile, yyloc.first_line);
            COUT << "Logical NOT ... " << ENDL;
        }
    |   valueComp                       {
            $$ = $1;
        }
    ;

// ================================= 
//  STREAM EXPRESSION
// =================================

streamExp:
        streamComp STREAM streamExp {
            $$ = new StreamNode(std::shared_ptr<AST>($1), std::shared_ptr<AST>($3), currentFile, yyloc.first_line);
        }
    |   streamComp                  {
            $$ = $1;
        }
    ;

// ================================= 
//  INDEX COMPONENTS
// =================================

indexComp:
        INT             {
            $$ = new ValueNode($1, currentFile, yyloc.first_line);
            COUT << "Index/Size Integer: " << $1 << ENDL;
        }
    |   UVAR            {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new BlockNode(s, currentFile, yyloc.first_line);
            COUT << "Index/Size User variable: " << $1 << ENDL;
            free($1);
        }
    |   scopeDef UVAR   {
            string s;
            s.append($2);
            $$ = new BlockNode(s, std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "Index/Size User variable: " << $2 << " in scope!" << ENDL;
            free($2);
        }
    |   bundleDef       {
            BundleNode *bundle = $1;
            COUT << "Resolving indexed bundle ..." << bundle->getName() << ENDL;
        }
    |   portPropertyDef       {
        }
    ;

// ================================= 
//  STREAM COMPONENTS
// =================================

streamComp:
        UVAR            {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new BlockNode(s, currentFile, yyloc.first_line);
            COUT << "User variable: " << $1 << ENDL;
            COUT << "Streaming ... " << ENDL;
            free($1);
        }
    |   scopeDef UVAR   {
            string s;
            s.append($2);
            $$ = new BlockNode(s, std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "User variable: " << $2 << " in scope!" << ENDL;
            COUT << "Streaming ... " << ENDL;
            free($2);
        }
    |   bundleDef       {
            COUT << "Resolving indexed bundle ..." << ENDL;
            COUT << "Streaming ... " << ENDL;
        }
    |   functionDef     {
            COUT << "Resolving function definition ... " << ENDL;
            COUT << "Streaming ... " << ENDL;
        }
    |   valueListDef    {
            COUT << "Resolving list definition ... " << ENDL;
            COUT << "Streaming ... " << ENDL;
        }
    |   streamListDef   {
            COUT << "Resolving list definition ... " << ENDL;
            COUT << "Streaming ... " << ENDL;
        }
    ;

// ================================= 
//  VALUE COMPONENTS
// =================================

valueComp:
        INT             {
            $$ = new ValueNode($1, currentFile, yyloc.first_line);
            COUT << "Integer: " << $1 << ENDL;
        }
    |   REAL           {
            $$ = new ValueNode($1, currentFile, yyloc.first_line);
            COUT << "Real: " << $1 << ENDL;
        }
    |   ON              {
            $$ = new ValueNode(true, currentFile, yyloc.first_line);
            COUT << "Keyword: on" << ENDL;
        }
    |   OFF             {
            $$ = new ValueNode(false, currentFile, yyloc.first_line);
            COUT << "Keyword: off" << ENDL;
        }
    |   STRING          {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new ValueNode(s, currentFile, yyloc.first_line);
            COUT << "String: " << $1 << ENDL;
            free($1);
        }
    |   WORD            {
            string s;
            s.append($1);
            $$ = new KeywordNode(s, currentFile, yyloc.first_line);
            COUT << "Word: " << $1 <<  ENDL;
            free($1);
        }
    |   UVAR            {
            string s;
            s.append($1); /* string constructor leaks otherwise! */
            $$ = new BlockNode(s, currentFile, yyloc.first_line);
            COUT << "User variable: " << $1 << ENDL;
            free($1);
        }
    |   scopeDef UVAR   {
            string s;
            s.append($2);
            $$ = new BlockNode(s, std::shared_ptr<AST>($1), currentFile, yyloc.first_line);
            COUT << "User variable: " << $2 << " in scope!" << ENDL;
            free($2);
        }
    |   bundleDef       {
            $$ = $1;
            COUT << "Resolving indexed bundle ..." << ENDL;
        }
    |   functionDef     {
            $$ = $1;
            COUT << "Resolving function definition ..." << ENDL;
        }
    |   portPropertyDef     {
            COUT << "Resolving port property definition ... " << ENDL;
        }
    ;

%%

void yyerror(const char *s){

//    This function is called by the lexer. We do not know how many arguments exist when
//    called. It is safer not to get the arguments to avoid an out of bound read.

//    va_list ap;
//    va_start(ap, s);

//    if(yylloc.first_line)
//      fprintf(stderr, "%d.%d-%d.%d: error: ", yylloc.first_line, yylloc.first_column,
//          yylloc.last_line, yylloc.last_column);
//    vfprintf(stderr, s, ap);
//    fprintf(stderr, "\n");
//    fprintf(stderr, "On file %s.\n", currentFile);

    cout << "Parser reported error: " << s << endl;
    cout << "Unexpected token: " << yytext << " on line: " <<  yylineno << endl;

    LangError newError;
    newError.type = LangError::Syntax;
    newError.errorTokens.push_back(std::string(yytext));
    newError.filename = string(currentFile);
    newError.lineNumber = yylineno;
    parseErrors.push_back(newError);
}

std::vector<LangError> getErrors() {
    return parseErrors;
}

AST *parse(const char *filename, const char*sourceFilename){
    FILE * file;
    AST *ast = NULL;
    if (sourceFilename == nullptr) {
        sourceFilename = filename;
    }

    char *lc;
    if (!(lc =setlocale (LC_ALL, "C"))) {
        COUT << "Error C setting locale.";
    }

    parseErrors.clear();
    file = fopen(filename, "r");

    if (!file){
        LangError newError;
        newError.type = LangError::SystemError;
        newError.errorTokens.push_back(std::strerror(errno));
        newError.errorTokens.push_back(std::string(filename));
        newError.lineNumber = yylineno;
        parseErrors.push_back(newError);
        COUT << "Can't open " << filename << ENDL;;
        return NULL;
    }

    currentFile = sourceFilename;

    COUT << "Analysing: " << filename << ENDL;
    COUT << "===========" << ENDL;

    tree_head = new AST;
    yyin = file;
    yylineno = 1;
    yyrestart(file);
    yyparse();
    fclose(file);

    if (parseErrors.size() > 0) {
        COUT << ENDL << "Number of Errors: " << parseErrors.size() << ENDL;
        delete tree_head;
        return NULL;
    }
    ast = tree_head;
    COUT << "Completed Analysing: " << filename << ENDL;
    return ast;
}

