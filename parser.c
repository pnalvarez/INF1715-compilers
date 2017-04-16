/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

void makeProgram(void){
    program = (Program*)malloc(sizeof(Program));
    if(program == NULL) { exit(1); }
    program->dec = NULL;
}

Declaration* makeDeclarationFromVar(DecVar *dv) {
    Declaration *dec = (Declaration*) malloc(sizeof(Declaration));
    if(dec == NULL) { exit(1); }
    dec->tag = decVar;
    dec->next=NULL;
    dec->u.dv=dv;
      
    return dec;
}

Declaration* makeDeclarationFromFunc(DecFunc *df){
    Declaration *dec = (Declaration*) malloc(sizeof(Declaration));
    if(dec == NULL) { exit(1); }
    dec->tag = decFunc;
    dec->next=NULL;
    dec->u.df=df;

    return dec;
}

void bindDeclarationProgram(Declaration *dec){
    if (program->dec == NULL) {
        program->dec = dec;
    } else {
		//Insert at begining of the list
        dec->next = program->dec;
        program->dec = dec;
    }
}

//Creates one DecVar for each name in the name list and make them chained
DecVar* makeDecVar(Type *t,ListNames *ln, int line) {
    DecVar *previous = NULL;
    for (ListNames *head = ln; head != NULL; head = head->next) {
        DecVar *d = (DecVar*) malloc(sizeof(DecVar));
    	if (d == NULL) { exit(1);}
    	d->type = t;
    	d->id = strdup(head->id);
	d->line = line;
    	d->next = previous;
        previous = d;
    }
    return previous;
}

DecVar* appendToDecVar(DecVar *list, DecVar *decVar) {
    if (list->next == NULL) {
        list->next = decVar;
    } else {
        DecVar *last;
        for(last=list;last->next!=NULL;last=last->next);
        last->next = decVar;
    }
    return list; 
}

ListNames* makeListNames(char *id) {
    ListNames *ln = (ListNames*) malloc(sizeof(ListNames));
    if (ln  == NULL) { exit(1); }
    ln->next = NULL;
    ln->id = strdup(id);
    return ln;
}

ListNames* appendToListNames(char *id, ListNames *ln) {
    ListNames *next = makeListNames(id);
    if (ln->next == NULL) {
        ln->next = next;
    } else {
        ListNames *last;
        for (last = ln; last->next != NULL; last=last->next);
        last->next = next;
    }
    return ln;
}

DecFunc* makeDecFunc(Type *type, char *id, DecVar *params, Block *block, int line){
	DecFunc *df=(DecFunc*)malloc(sizeof(DecFunc));
	if(df==NULL) {
		exit(1);
	}
	df->type=type;
	df->id=strdup(id);
	df->params=params;
	df->block=block;
	df->line = line;
	return df;
}

DecVar* makeSingleDecVar(Type *type, char *id, int line){
    DecVar *decVar=(DecVar*)malloc(sizeof(DecVar));
    if(decVar == NULL) {
        exit(1);
	}
    decVar->line = line;
    decVar->type=type;
    decVar->id=strdup(id);
    decVar->next=NULL;
    return decVar;
}

Block* makeBlock(DecVar *decVar, Command *command) {
    Block *block = (Block*) malloc(sizeof(Block));
    if (block == NULL) { exit(1); }
    block->decVar = decVar;
    block->command = command;
    return block; 
}

Command* makeCommand() {
    Command *c = (Command*) malloc(sizeof(Command));
    if (c==NULL) { exit(1); }
    c->next = NULL;
    return c;
}

Command* appendToListCommand(Command* list, Command *command) {
    if (list->next == NULL) {
        list->next = command;
    } else {
        Command *last;
        for(last = list; last->next != NULL; last=last->next);
		last->next = command;
    } 
    return list;
}

Command* privateCreateCommand(CommandE tag) {
    Command *command = (Command*) malloc(sizeof(Command));
    if (command == NULL) { exit(1); }
    command->tag = tag;
    command->next = NULL;
    return command;
}

Command* makeCommandFromAssignment(Var *var, Exp *exp, int line) {
    Command *command = privateCreateCommand(assignmentE);
    command->u.assignment.var = var;
    command->u.assignment.exp = exp;
    command->line = line;
    return command;
}

Command* makeCommandFromCall(Call *call, int line) {
    Command *command = privateCreateCommand(callE);
    command->u.call = call;
    command->line = line;
    return command;
}

Command* makeCommandFromReturnNode(ReturnNode *ret, int line) {
    Command *command = privateCreateCommand(returnCmdE);
    command->u.returnNode = ret;
    command->line = line;
    return command;
}

Command* makeCommandFromBlock(Block *block, int line) {
    Command *command = privateCreateCommand(blockE);
    command->u.block = block;
    command->line = line;
    return command;
}

Command* makeCommandFromWhile(Exp *exp, Command *command, int line) {
    Command *newCommand = privateCreateCommand(whileCmdE);
    newCommand->u.whileCmd.exp = exp;
    newCommand->u.whileCmd.command = command;
    newCommand->line = line;
    return newCommand;
}

Command* makeCommandFromIf(Exp *exp, Command *command, int line) {
    Command *newCommand = privateCreateCommand(ifCmdE);
    newCommand->u.ifCmd.exp = exp;
    newCommand->u.ifCmd.command = command;
    newCommand->line = line;
    return newCommand;
}

Command* makeCommandFromIfElse(Exp *exp, Command *command1, Command *command2, int line) {
    Command *command = privateCreateCommand(ifElseE);
    command->u.ifElse.exp = exp;
    command->u.ifElse.command1 = command1;
    command->u.ifElse.command2 = command2;
    command->line = line;
    return command;
}

Command* makeCommandFromPrint(Exp *exp, int line) {
	Command *command = privateCreateCommand(print);
	command->u.printExp = exp;
	command->line = line;
	return command;
}

Var* makeVarSimple(char *id, int line) {
    Var *var = (Var*) malloc(sizeof(Var));
    if (var == NULL) { exit(1); }
    var->tag = simple;
    var->u.id = strdup(id);
    var->line = line;
    return var;
}

Var* makeVarIndexed(Exp* expVar, Exp *expIndexed, int line) {
    Var *newVar = (Var*) malloc(sizeof(Var));
    if (newVar == NULL) { exit(1); }
    newVar->tag = indexed;
    newVar->u.indexed.expVar = expVar;
    newVar->u.indexed.expIndexed = expIndexed;
    newVar->line = line;
    return newVar;
}

Exp* appendToListExp(Exp *list, Exp *exp) {
    if (list->next == NULL) {
        list->next = exp;
    } else {
        Exp *last;
        for(last = list; last->next != NULL; last=last->next);
        last->next = exp;
    }
    return list;
}

Call* makeCall(char *id, Exp* listExp, int line) {
    Call *c = (Call*) malloc(sizeof(Call));
    if (c==NULL) { exit(1); }
    c->id = strdup(id);
    c->listExp = listExp;
    c->line = line;
    return c;
}

ReturnNode* makeReturn(Exp *exp, int line) {
    ReturnNode *ret = (ReturnNode*) malloc(sizeof(ReturnNode));
    if (ret == NULL) { exit(1); }
    ret->exp = exp;
    ret->decFunc = NULL;
    ret->line = line;
    return ret;
}

Type* makeTypeSimple(TypeBaseE typeBase) {
    Type *t = (Type*) malloc(sizeof(Type));
    if (t == NULL) { exit(1); }
    t->tag = simple;
    t->u.typeBase = typeBase;
    return t;
}

Type* makeTypeIndexed(Type *type) {
    Type *t = (Type*) malloc(sizeof(Type));
    if (t == NULL) { exit(1); }
    t->tag = indexed;
    t->u.type = type;
    return t;
}

Num* makeNumFromInt(int val) {
    Num *n = (Num*) malloc(sizeof(Num));
    if (n==NULL) { exit(1); }
    n->type = makeTypeSimple(intType);
    n->u.intValue = val;
    return n;
}

Num* makeNumFromFloat(float val) {
    Num *n = (Num*) malloc(sizeof(Num));
    if (n==NULL) { exit(1); }
    n->type = makeTypeSimple(floatType);
    n->u.floatValue = val;
    return n;
}

Num* makeNumFromString(char *val) {
    Num *n = (Num*) malloc(sizeof(Num));
    if (n==NULL) { exit(1); }
    Type *charSimple = makeTypeSimple(charType);
    n->type = makeTypeIndexed(charSimple);
    n->u.stringValue = val;
    return n;
}

//Private func
Exp* privateCreateExp(ExpE tag) {
    Exp *exp = (Exp*) malloc(sizeof(Exp));
    if (exp == NULL) { exit(1); }
    exp->next = NULL;
    exp->tag = tag;
    return exp;
}

Exp* makeExpFromNum(Num *num, int line){
    Exp *e = privateCreateExp(expE_num);
    e->u.num = num;
    e->line = line;  
    return e;
}

Exp* makeExpFromNegativeNum(Num *num, int line){
    Exp *e = privateCreateExp(expE_negativeNum);
    e->u.num = num;
    e->line = line;
    return e;
}

Exp* makeExpFromVar(Var *var, int line){
    Exp *e = privateCreateExp(expE_var);
    e->u.var = var;
    e->line = line;
    return e;
}

Exp* makeExpFromCall(Call *call, int line){
    Exp *e = privateCreateExp(expE_call);
    e->u.call = call;
    e->line = line;
    return e;
}

Exp* makeExpFromNew(Type *type,Exp *exp, int line) {
    Exp *e = privateCreateExp(expE_expNew);
    e->u.newVal.exp = exp;
    e->u.newVal.type = type;
    e->line = line;
    return e;
}

Exp* makeExpFromNot(Exp *exp, int line) {
    Exp *e = privateCreateExp(expE_expNot);
    e->u.expNot = exp;
    e->line = line;
    return e;
}

Exp* makeExpBin(Exp *exp1, Exp *exp2, ExpE tag, int line) {
    Exp *e = privateCreateExp(tag);
    e->u.bin.exp1 = exp1;
    e->u.bin.exp2 = exp2;
    e->line = line;
    return e;
}

Exp* makeExpFromTypecast(Exp *exp, Type *type, int line) {
    Exp *e = privateCreateExp(expE_typecast);
    e->u.expCasted = exp;
    e->type = type;
    e->line = line;
    return e;
}

