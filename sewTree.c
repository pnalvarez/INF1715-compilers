/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#include "parser.h"
#include "seamDataStructure.h"
#include <stdio.h>
#include <stdlib.h>

DecFunc *currentFunc = NULL;
ReturnNode *returnCounter = 0;


void sewExp(Exp *exp);

void sewVar(Var *v) {
    DecVar *dec = findDecVar(v->u.id);
    switch(v->tag) {
    case simple:
        if (dec == NULL) {
	    printf("Error: undefined variable %s. Line %d\n",v->u.id, v->line);
	    exit(1);
        }
	v->type = dec->type;
	v->decVar = dec;
	break;
    case indexed:
		v->decVar = NULL;
		sewExp(v->u.indexed.expVar);
		sewExp(v->u.indexed.expIndexed);
		//var indexed is sewed in typeChecker.c;
	break;
    }
}

void sewCall(Call *c);

void sewExp(Exp *exp) {
    switch(exp->tag) {
	case expE_expOr:
	case expE_expAnd: 
	case expE_expEE: 
	case expE_expNE: 
	case expE_expGreater: 
	case expE_expLess: 
	case expE_expGE: 
	case expE_expLE: 
	case expE_expAdd: 
	case expE_expMinus: 
	case expE_expMul: 
	case expE_expDiv: 
	case expE_expMod:
	    sewExp(exp->u.bin.exp1);
	    sewExp(exp->u.bin.exp2); 
	    break;
	case expE_expNot: 
	    sewExp(exp->u.expNot); 
	    break;
	case expE_expNew: 
	    sewExp(exp->u.newVal.exp);
	    break;
	case expE_negativeNum: 
	    break; 
	case expE_num: 
	    break;
	case expE_var: 
	    sewVar(exp->u.var); 
	    break;
	case expE_call: 
	    sewCall(exp->u.call); 
	    break;
	case expE_typecast:
	    break;
    }
}

void sewCall(Call *c) {
    c->typeInfo.dec = findDecFunc(c->id);
    if (c->typeInfo.dec == NULL) {
		printf("Error: undefined function %s. Line %d\n",c->id,c->line);
		exit(1);
    }
    for (Exp *e = c->listExp; e!=NULL; e=e->next) {
		sewExp(e);
    }
}

void sewDecVar(DecVar *decVar){
    if (decVar != NULL) {
		if (insertDecVar(decVar) == redefinitionError) {
	    	printf("Error: redefinition of variable %s. Line %d\n",decVar->id, decVar->line);
	    	exit(1);
		}
		sewDecVar(decVar->next);
    }
}

void sewReturn(ReturnNode *r) {
    //This must never happen, because it's a sintax error.
    if (currentFunc == NULL) {
		printf("Error: return with no function. Line %d\n", r->line);
		exit(1);
    }
    returnCounter++;
    r->decFunc = currentFunc;
    if (r->exp != NULL) {
        sewExp(r->exp);
    }
}

void sewBlock(Block *b, int openScope);

void sewCommand(Command *command) {
    for(Command *c = command; c!=NULL; c=c->next) {
        switch(c->tag) {
	    case assignmentE: 
			sewVar(c->u.assignment.var);
			sewExp(c->u.assignment.exp); 
			break;
	    case callE: 
			sewCall(c->u.call);
			break;
	    case returnCmdE: 
			sewReturn(c->u.returnNode);
			break;
	    case whileCmdE: 
			sewExp(c->u.whileCmd.exp);
			sewCommand(c->u.whileCmd.command);
			break;
	    case blockE: 
			sewBlock(c->u.block, 1); //enter a new scope
			break;
	    case ifCmdE: 
			sewExp(c->u.ifCmd.exp);
			sewCommand(c->u.ifCmd.command); 
			break;
	    case ifElseE: 
			sewExp(c->u.ifElse.exp);
			sewCommand(c->u.ifElse.command1); 
			sewCommand(c->u.ifElse.command2);
			break;
		case print:
			sewExp(c->u.printExp);
			break;
       }
    }
}

//param int openScope:
//	0: Doesn't open
//	1: Do open
void sewBlock(Block *block, int openScope) {
    if (openScope) {
		enterScope();
		sewDecVar(block->decVar);
		sewCommand(block->command);
		leaveScope();
    }
    else {
		sewDecVar(block->decVar);
		sewCommand(block->command);
    }
}

void sewDecFunc(DecFunc *decFunc){
    if (insertDecFunc(decFunc) == redefinitionError) {
		printf("Error: redefinition of function %s. Line %d\n",decFunc->id,decFunc->line);
		exit(1);
    } else {
		currentFunc = decFunc;
		returnCounter = 0; //initializes return counter
		enterScope();
		sewDecVar(decFunc->params);
		sewBlock(decFunc->block, 0); //Do not enter scope again, because parameters can't be overwritten
		leaveScope();
		//if it doesn't have return but is not a void function
		if (returnCounter == 0 && decFunc->type != NULL) {
	    	printf("Error: function %s without return. Line %d\n",decFunc->id,decFunc->line);
	   	 	exit(1);
		}
    }
}

void sewDeclaration(Declaration *dec){
    switch(dec->tag) {
        case decVar: sewDecVar(dec->u.dv); break;
        case decFunc: sewDecFunc(dec->u.df); break;
    }
}

void sewTree(Program *p) {
    enterScope();
    for(Declaration *d = p->dec;d!=NULL;d=d->next) {
        sewDeclaration(d);
    }
    freeAll();
}
