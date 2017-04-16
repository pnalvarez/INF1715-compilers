/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#ifndef parser_seen
#define parser_seen

//MARK: enumerations

typedef enum decE{
    decVar,decFunc
}DecE;

typedef enum varE{
    simple,indexed
}VarE;

typedef enum typeBaseE{
	intType,floatType,charType
}TypeBaseE;

typedef enum expE{
    expE_expOr,
    expE_expAnd,
    expE_expEE,
    expE_expNE,
    expE_expGreater,
    expE_expLess,
    expE_expGE,
    expE_expLE,
    expE_expAdd,
    expE_expMinus,
    expE_expMul,
    expE_expDiv,
    expE_expMod,
    expE_expNot,
    expE_expNew,
    expE_num,
    expE_negativeNum,
    expE_var,
    expE_call,
    expE_typecast
}ExpE;

typedef enum commandE {
    assignmentE,
    callE,
    returnCmdE,
    whileCmdE,
    blockE,
    ifCmdE,
    ifElseE,
	print
}CommandE;

//MARK: structures

//is a list
typedef struct declaration{
    DecE tag;
    struct declaration *next;
    union{
        struct decVar *dv;
        struct decFunc *df;
    }u;
}Declaration;

typedef struct program{
	Declaration *dec;
}Program;

//is a list
typedef struct type {
    VarE tag;
    union {
		TypeBaseE typeBase;
		struct type *type;
    }u;
} Type;

//is a list
typedef struct listNames {
    char *id;
    struct listNames *next;
} ListNames;

//is a list
typedef struct decVar{
    Type *type;
    char *id;
    struct decVar *next;
    int line;
	//to generate llvm code	
	int temp;
	char* scope;
}DecVar;

typedef struct decFunc{
    struct type *type; /*When type is NULL, the function returns void*/
    char *id;
    struct decVar *params;
    struct block *block;
    int line;
}DecFunc;

typedef struct block {
    struct decVar *decVar;
    struct command *command;
}Block;

//is a list
typedef struct exp{
    struct exp *next;
    ExpE tag;
    Type *type;
    int line;
    union {
		struct {
	   		Type *type;
	   		struct exp *exp; 
		}newVal;
		struct {
	    	struct exp *exp1;
	    	struct exp *exp2;
		}bin;
		struct exp *expNot;
		struct num *num; 
		struct var *var;
		struct call *call;
		struct exp *expCasted;
    }u;
}Exp;

typedef struct var{
    VarE tag;
    Type *type;
	DecVar *decVar;
    int line;
    union{
        char *id;
		struct{
	    	struct exp *expVar;
	    	struct exp *expIndexed;
		}indexed;
    }u;
}Var;

typedef struct call{
    char *id;
    struct exp *listExp;
    union {
		struct decFunc *dec;
		Type *type;
    }typeInfo;
    int line;
}Call;

//is a list
typedef struct command {
    CommandE tag;
    int line;
    union {
        struct {
            Var *var;
            Exp *exp;
        }assignment;
        Call *call;
        struct returnNode *returnNode;
        Block *block;
		Exp *printExp;
        struct {
	    	Exp *exp;
            struct command *command;
		}whileCmd;
        struct {
 	    	Exp *exp;
	    	struct command *command;
		}ifCmd;
		struct {
	    	Exp *exp;
	    	struct command *command1;
	    	struct command *command2;
		}ifElse;
    }u;
    struct command *next;
}Command;

typedef struct returnNode{
    Exp *exp; /*When exp is NULL, the function returns void*/
    DecFunc *decFunc;
    int line;
}ReturnNode;

typedef struct num{
    Type *type;
    union {
        int intValue;
        float floatValue;
        char *stringValue;
    }u;
	int stringTemp;
}Num;

//MARK: Program

Program *program;

void makeProgram(void);

//MARK: Declaration

Declaration* makeDeclarationFromVar(DecVar *df);

Declaration* makeDeclarationFromFunc(DecFunc *df);

void bindDeclarationProgram(Declaration *dec);

//MARK: DevcVar

DecVar* makeDecVar(Type *t,ListNames *ln, int line);

DecVar* appendToDecVar(DecVar *list, DecVar *decVar);

DecVar* makeSingleDecVar(Type *type, char *id, int line);

ListNames* makeListNames(char *id);

ListNames* appendToListNames(char *id, ListNames *ln);

//MARK: DecFunc

DecFunc* makeDecFunc(Type *t, char *id, DecVar *params, Block *block, int line);

//MARK: Block

Block* makeBlock(DecVar *decVar, Command *command);

//MARK: Command

Command* appendToListCommand(Command* list, Command *command);

Command* makeCommandFromAssignment(Var *var, Exp *exp, int line);

Command* makeCommandFromCall(Call *call, int line);

Command* makeCommandFromReturnNode(ReturnNode *ret, int line);

Command* makeCommandFromBlock(Block *block, int line);

Command* makeCommandFromWhile(Exp *exp, Command *cmd, int line);

Command* makeCommandFromIf(Exp *exp, Command *command, int line);

Command* makeCommandFromIfElse(Exp *exp, Command *command1, Command *command2, int line);

Command* makeCommandFromPrint(Exp *exp, int line);

//MARK: Var

Var* makeVarSimple(char *id, int line);

Var* makeVarIndexed(Exp *expVar , Exp *expIndexed, int line);

//MARK: Call

Call* makeCall(char *id, Exp* listExp, int line);

//MARK: ReturnNode

ReturnNode* makeReturn(Exp *exp, int line);

//MARK: Type

Type* makeTypeSimple(TypeBaseE typeBase);

Type* makeTypeIndexed(Type *type);

//MARK: Num

Num* makeNumFromInt(int val);

Num* makeNumFromFloat(float val);

Num* makeNumFromString(char* val);

//MARK: Exp

Exp* makeExpFromNum(Num *num, int line);

Exp* makeExpFromNegativeNum(Num *num, int line);

Exp* makeExpFromVar(Var *var, int line);

Exp* makeExpFromCall(Call *call, int line);

Exp* appendToListExp(Exp *list, Exp *exp);

Exp* makeExpBin(Exp *exp1, Exp *exp2, ExpE tag, int line);

Exp* makeExpFromNot(Exp *exp, int line);

Exp* makeExpFromNew(Type *type,Exp *exp, int line);

Exp* makeExpFromTypecast(Exp *exp, Type *type, int line);

#endif // parser_seen 

