/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "printTree.h"
#include "sewTree.h"
#include "typeChecker.h"
#include "generateLLVM.h"
int yylex(void);
void yyerror(char *);
int yylineno;
%}    

%union {
    int i;
    float f;
    char *s;
    char *id;
    Declaration *def;
    DecVar *decVar;
    DecFunc *defFunc;
    Type *type;
    ListNames *listNames;
    Block *block;
    Command *command;
    ReturnNode *returnNode;
    Exp *exp;
    Call *call;
    Var *var;
    TypeBaseE typeBaseE;
    Num *num;
};

%token <id> TK_ID
%token <typeBaseE> TK_INT
%token <typeBaseE> TK_FLOAT
%token <typeBaseE> TK_CHAR
%token TK_GE
%token TK_LE
%token TK_EE
%token TK_VOID
%token TK_IF
%token TK_WHILE
%token TK_ELSE
%token TK_NEW
%token <i> TK_CONST_INT
%token <f> TK_CONST_FLOAT
%token <s> TK_CONST_STRING
%token TK_LOGIC_AND
%token TK_LOGIC_OR
%token TK_RETURN
%token TK_NE

%type <exp> expOrPriority expAndPriority expCompPriority expAddPriority expMulPriority expNotPriority exp listExp prim
%type<def> program declaration
%type<decVar> decVariable listDecVariable parameter parameters
%type<defFunc> decFunction
%type<type> type
%type<listNames> listNames
%type<block> block
%type<command> command listCommand commandIfElse
%type<returnNode> return
%type<call> call
%type<var> var
%type<typeBaseE> typeBase
%type<num> num
%%


program : declaration {bindDeclarationProgram($1);}				
         | declaration program {bindDeclarationProgram($1);}		
         ;

declaration : decVariable {$$=makeDeclarationFromVar($1);}		
          | decFunction {$$=makeDeclarationFromFunc($1);}				
          ;

command : var '=' exp ';' {$$=makeCommandFromAssignment($1,$3,yylineno);}				
        | call ';' {$$=makeCommandFromCall($1,yylineno);}					          
        | return ';' {$$=makeCommandFromReturnNode($1,yylineno);}			
        | TK_WHILE '(' exp ')' command	{$$=makeCommandFromWhile($3,$5,yylineno);}			
        | block	{$$=makeCommandFromBlock($1,yylineno);}				
        | TK_IF '(' exp ')' command	{$$=makeCommandFromIf($3,$5,yylineno);}			
        | TK_IF '(' exp ')' commandIfElse TK_ELSE command	{$$=makeCommandFromIfElse($3,$5,$7,yylineno);}
		| '@' exp ';' {$$=makeCommandFromPrint($2,yylineno);}
        ;

commandIfElse : var '=' exp ';' {$$=makeCommandFromAssignment($1,$3,yylineno);}
              | call ';' {$$=makeCommandFromCall($1,yylineno);}
              | return ';' {$$=makeCommandFromReturnNode($1,yylineno);}
              | TK_WHILE '(' exp ')' commandIfElse {$$=makeCommandFromWhile($3,$5,yylineno);}
              | block {$$=makeCommandFromBlock($1,yylineno);}
              | TK_IF '(' exp ')' commandIfElse TK_ELSE commandIfElse {$$=makeCommandFromIfElse($3,$5,$7,yylineno);}
			  | '@' exp ';' {$$=makeCommandFromPrint($2,yylineno);}
              ;


block : '{' listDecVariable  listCommand '}' {$$=makeBlock($2,$3);}
      | '{' listDecVariable '}' {$$=makeBlock($2,NULL);}
      | '{' listCommand '}' {$$=makeBlock(NULL,$2);}
      | '{' '}' {$$=makeBlock(NULL,NULL);}
      ;

listDecVariable : decVariable {$$=$1;}
                 | listDecVariable decVariable {$$=appendToDecVar($1,$2);}
                 ;

listCommand : command {$$=$1;}
             | listCommand command {$$=appendToListCommand($1,$2);}
             ;  

return : TK_RETURN {$$=makeReturn(NULL,yylineno);}
       | TK_RETURN exp {$$=makeReturn($2,yylineno);}
       ;

decVariable : type listNames ';' {$$=makeDecVar($1,$2,yylineno);}
            ;

decFunction : type TK_ID '(' parameters ')' block {$$=makeDecFunc($1,$2,$4,$6,yylineno);}
          | TK_VOID TK_ID '(' parameters ')' block {$$=makeDecFunc(NULL,$2,$4,$6,yylineno);}
          | type TK_ID '(' ')' block {$$=makeDecFunc($1,$2,NULL,$5,yylineno);}
          | TK_VOID TK_ID '(' ')' block {$$=makeDecFunc(NULL,$2,NULL,$5,yylineno);}
          ;

parameters : parameter {$$=$1;}
           | parameters ',' parameter {$$=appendToDecVar($1,$3);}
           ;

parameter : type TK_ID {$$=makeSingleDecVar($1,strdup($2),yylineno);}
          ;

listNames : TK_ID {$$=makeListNames($1);}
          | TK_ID ',' listNames {$$=appendToListNames($1,$3);}
          ; 

exp : expOrPriority {$$=$1;}
    ;

expOrPriority : expOrPriority TK_LOGIC_OR expAndPriority {$$=makeExpBin($1,$3,expE_expOr,yylineno);}
              | expAndPriority {$$=$1;}
              ;

expAndPriority : expAndPriority TK_LOGIC_AND expCompPriority {$$=makeExpBin($1,$3,expE_expAnd,yylineno);}
               | expCompPriority {$$=$1;}
               ;

expCompPriority : expCompPriority TK_EE expAddPriority {$$=makeExpBin($1,$3,expE_expEE,yylineno);}
                | expCompPriority TK_NE expAddPriority {$$=makeExpBin($1,$3,expE_expNE,yylineno);}
                | expCompPriority '<' expAddPriority {$$=makeExpBin($1,$3,expE_expLess,yylineno);}
                | expCompPriority '>' expAddPriority {$$=makeExpBin($1,$3,expE_expGreater,yylineno);}
                | expCompPriority TK_GE expAddPriority {$$=makeExpBin($1,$3,expE_expGE,yylineno);}
				| expCompPriority TK_LE expAddPriority {$$=makeExpBin($1,$3,expE_expLE,yylineno);}
                | expAddPriority {$$=$1;}
                ;

expAddPriority : expAddPriority '+' expMulPriority {$$=makeExpBin($1,$3,expE_expAdd,yylineno);}
               | expAddPriority '-' expMulPriority {$$=makeExpBin($1,$3,expE_expMinus,yylineno);}
               | expMulPriority {$$=$1;}
               ;

expMulPriority : expMulPriority '/' expNotPriority {$$=makeExpBin($1,$3,expE_expDiv,yylineno);}
     	       | expMulPriority '*' expNotPriority {$$=makeExpBin($1,$3,expE_expMul,yylineno);}
               | expMulPriority '%' expNotPriority {$$=makeExpBin($1,$3,expE_expMod,yylineno);}
     	       | expNotPriority {$$=$1;}
     	       ;

expNotPriority : '!' expNotPriority {$$=makeExpFromNot($2,yylineno);}
               | TK_NEW type '[' exp ']' {$$=makeExpFromNew($2,$4,yylineno);}
               | prim {$$=$1;}


prim : '-' num {$$=makeExpFromNegativeNum($2,yylineno);}
     | num {$$=makeExpFromNum($1,yylineno);}
     | '('exp')' {$$=$2;}
     | var {$$=makeExpFromVar($1,yylineno);}
     | call {$$=makeExpFromCall($1,yylineno);}
     ;

num  : TK_CONST_INT {$$=makeNumFromInt($1);}
     | TK_CONST_FLOAT {$$=makeNumFromFloat($1);}
     | TK_CONST_STRING {$$=makeNumFromString($1);}
     ;

var : TK_ID {$$=makeVarSimple($1,yylineno);}
    | prim '[' exp ']' {$$=makeVarIndexed($1,$3,yylineno);}
    ;

type : typeBase {$$=makeTypeSimple($1);}
     | type '[' ']' {$$=makeTypeIndexed($1);}
     ;

typeBase : TK_INT {$$=intType;}
         | TK_CHAR {$$=charType;}
         | TK_FLOAT {$$=floatType;}
         ;

call    : TK_ID '(' listExp ')' {$$=makeCall($1,$3,yylineno);}
        ;

listExp  : exp {$$=$1;}
         | listExp ',' exp {$$=appendToListExp($1,$3);}
         | /*empty*/ {$$=NULL;}
         ;

%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
    printf("error at line: %d\n", yylineno);
}

//The program can receive -ast to print the AST and -llvm, to generate and print llvm code.
int main(int argc, char* argv[]) {
    if (argc > 3 ) {
		printf("at most two arguments: -ast or -llvm\n");
		exit(1);
    }
    int willPrintAST = 0;
    int willPrintLLVM = 0;
    for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i],"./monga")==0) {
	    	continue;
		}
		else if (strcmp(argv[i],"-ast")==0) {
	   		willPrintAST = 1;
		}
		else if (strcmp(argv[i],"-llvm")==0) {
	    	willPrintLLVM = 1;
		} 
		else {
	    	printf("Error: %s is not a valid argument (only -ast and -llvm)\n", argv[i]);
	    	exit(1);
		}
    }
    makeProgram();
    yyparse();
    sewTree(program);
    checkTree(program);
    if (willPrintAST) {
		printf("created AST.txt\n");
		printProgram(program);  
    }
    if (willPrintLLVM) {
		printf("created LLVM_CODE.txt\n");
		generateLLVM(program);  
    }
    return 0;
}
