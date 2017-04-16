/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "printTree.h"
#include "typeChecker.h"

FILE *f;

void printSpaces(int level) {
    for (int j=0;j<level;j++) {
		fprintf(f,"  ");
    }
}
  
void printType(Type *type, int level) {
    printSpaces(level);
    if (type == NULL) {
		fprintf(f,"type void\n");
		return;
	} 
	if (type->tag == indexed) {
	    fprintf(f,"[]\n");
	    printType(type->u.type, level+1);
	}
	else {
	    switch(type->u.typeBase) {
	    case intType:
			fprintf(f,"int\n");
			break;
	    case floatType:
			fprintf(f,"float\n");
			break;
	    case charType:
			fprintf(f,"char\n");
			break;
	    }
	}   
}

void printNum(Num *num, int level) {
    printSpaces(level);
    fprintf(f,"Num");
    switch(num->type->tag) {
	case simple: 
	    if (num->type->u.typeBase == intType) {
			fprintf(f," %d\n",num->u.intValue);
	    } else if (num->type->u.typeBase == floatType) {
			fprintf(f," %f\n",num->u.floatValue); break;
	    }
	    break;
	case indexed: 
	    fprintf(f," %s\n",num->u.stringValue); 
	    break;
    }
    printType(num->type, level);
}


void printId(char *id, int level) {
    printSpaces(level);
    fprintf(f,"id %s\n",id);
}

void printVar(Var *v, int level) {
    printSpaces(level);
    fprintf(f,"Var\n");
    switch(v->tag) {
	case simple: 
		printId(v->u.id, level+1);
		break;
	case indexed: 
		printExp(v->u.indexed.expVar,level+1); 
		printExp(v->u.indexed.expIndexed,level+1);
		break;
    }
}

void printCall(Call *c, int level) {
    printSpaces(level);
    fprintf(f,"Call\n");
    printId(c->id, level+1);
    for (Exp *e = c->listExp; e!=NULL; e=e->next) {
		printExp(e,level+1);
    }
}

void printExpBin(Exp *exp, char *label, int level) {
    fprintf(f,"%s\n",label);
    printExp(exp->u.bin.exp1, level);
    printExp(exp->u.bin.exp2, level);
}
  
void printExp(Exp *exp, int level) {
    printSpaces(level);
    switch(exp->tag) {
    case expE_expOr: 
		printExpBin(exp,"ExpOr",level+1); 
		break;
    case expE_expAnd: 
		printExpBin(exp,"ExpAnd",level+1); 
		break;
    case expE_expEE: 
		printExpBin(exp,"ExpEE",level+1); 
		break;
    case expE_expNE: 
		printExpBin(exp,"ExpNE",level+1); 
		break;
    case expE_expGreater: 
		printExpBin(exp,"ExpGreater",level+1); 
		break;
    case expE_expLess: 
		printExpBin(exp,"ExpLess",level+1); 
		break;
    case expE_expGE: 
		printExpBin(exp,"ExpGE",level+1); 
		break;
    case expE_expLE: 
		printExpBin(exp,"ExpLE",level+1); 
		break;
    case expE_expAdd: 
		printExpBin(exp,"ExpAdd",level+1); 
		break;
    case expE_expMinus: 
		printExpBin(exp,"ExpMinus",level+1); 
		break;
    case expE_expMul: 
		printExpBin(exp,"ExpMul",level+1); 
		break;
    case expE_expDiv: 
		printExpBin(exp,"ExpDiv",level+1); 
		break;
    case expE_expMod: 
		printExpBin(exp,"ExpMod",level+1); 
		break;
    case expE_expNot: 
		fprintf(f,"expNot\n"); 
		printExp(exp->u.expNot, level+1); 
		break;
    case expE_expNew: 
		fprintf(f,"expNew\n"); 
		printType(exp->u.newVal.type, level+1); 
		printExp(exp->u.newVal.exp, level+1); 
		break;
    case expE_negativeNum: 
		fprintf(f,"-\n"); 
		printNum(exp->u.num,level+1); 
		break; 
    case expE_num: 
		fprintf(f,"expNum\n"); 
		printNum(exp->u.num,level+1); 
		break;
    case expE_var: 
		fprintf(f,"expVar\n"); 
		printVar(exp->u.var,level+1); 
		break;
    case expE_call: 
		fprintf(f,"expCall\n"); 
		printCall(exp->u.call,level+1); 
		break;
    case expE_typecast:
		fprintf(f,"expTypecast\n");
		printExp(exp->u.expCasted,level+1);
		break;
    }
    printType(exp->type,level);
}

void printDecVar(DecVar *decVar, int level){
    if (decVar != NULL) {
        printSpaces(level);
        fprintf(f,"DecVar\n");
		printId(decVar->id, level+1);
		printType(decVar->type, level+1);
        printDecVar(decVar->next, level);
    }
}

void printReturn(ReturnNode *r, int level) {
    printSpaces(level);
    fprintf(f,"Return\n");
	//if the function not returns void
    if (r->exp != NULL) {
        printExp(r->exp,level+1);
    }
}

void printCommand(Command *command, int level) {
    for(Command *c = command; c!=NULL; c=c->next) {
        printSpaces(level);
        switch(c->tag) {
	    case assignmentE: 
			fprintf(f,"Cmd-assignment\n");
			printVar(c->u.assignment.var,level+1);
			printExp(c->u.assignment.exp,level+1); 
			break;
	    case callE: 
			fprintf(f,"Cmd-call\n");
			printCall(c->u.call,level+1);
			break;
	    case returnCmdE: 
			fprintf(f,"Cmd-return\n");
			printReturn(c->u.returnNode,level+1);
			break;
	    case whileCmdE: 
			fprintf(f,"Cmd-while\n");
			printExp(c->u.whileCmd.exp,level+1);
			printCommand(c->u.whileCmd.command,level+1);
			break;
	    case blockE: 
			fprintf(f,"Cmd-block\n");
			printBlock(c->u.block,level+1);
			break;
	    case ifCmdE: 
			fprintf(f,"Cmd-if\n");
			printExp(c->u.ifCmd.exp,level+1);
			printCommand(c->u.ifCmd.command,level+1); 
			break;
	    case ifElseE: 
			fprintf(f,"Cmd-ifElse\n");
			printExp(c->u.ifElse.exp, level+1);
			printCommand(c->u.ifElse.command1,level+1); 
			printCommand(c->u.ifElse.command2,level+1);
			break;
		case print:
			fprintf(f,"Cmd-print\n");
			printExp(c->u.printExp, level+1);
			break;
       }
    }
}

void printBlock(Block *block, int level) {
    printSpaces(level);
    fprintf(f,"Block\n");
    printDecVar(block->decVar, level+1);
    printCommand(block->command, level+1);
}

void printDecFunc(DecFunc *decFunc, int level){
    printSpaces(level);
    fprintf(f,"DecFunc\n");
    printType(decFunc->type,level+1);
    printId(decFunc->id,level+1);
    printDecVar(decFunc->params,level+1);
    printBlock(decFunc->block,level+1);
}

void printDeclaration(Declaration *dec, int level){
    printSpaces(level);
    fprintf(f,"Declaration\n");
    switch(dec->tag) {
        case decVar: printDecVar(dec->u.dv,level+1); break;
        case decFunc: printDecFunc(dec->u.df,level+1); break;
    }
}

void printProgram(Program *p){
	f = fopen("AST.txt","w");
	fprintf(f,"Program\n");
	for(Declaration *d = p->dec;d!=NULL;d=d->next) {
		printDeclaration(d,1);
	}
	fclose(f);
}

