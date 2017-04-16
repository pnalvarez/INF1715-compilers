/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#include "parser.h"
#include "typeChecker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Type *TYPE_INT; 
Type *TYPE_FLOAT;
Type *TYPE_CHAR;

int typeCmpAux(Type *t1, Type *t2, int depht) {
	if (depht > MAX_TYPE/2 ) {
		printf("Error: Array max depht is %d\n", MAX_TYPE/2);
		exit(1);
	}
	if (t1 == NULL && t2 == NULL) {
		return 1;
    }
    if (t1 == NULL || t2 == NULL) {
		return 0;
    }
    if (t1->tag == simple && t2->tag == simple && t1->u.typeBase == t2->u.typeBase) { 
		return 1;
    } 
    else if (t1->tag == simple && t2->tag == simple && t1->u.typeBase == charType && t2->u.typeBase == intType) { 
		return 1;
    } 
    else if (t1->tag == simple && t2->tag == simple && t2->u.typeBase == charType && t1->u.typeBase == intType) { 
		return 1;
    }
    else if (t1->tag == indexed && t2->tag == indexed) { 
		return typeCmpAux(t1->u.type,t2->u.type, depht+1);
    } else { 
		return 0;
    }
}
//return:
//	1: equal
//	0: different 
// 	Atention: char is considered equal to int
int typeCmp(Type *t1, Type *t2) {
    return typeCmpAux(t1, t2, 0);
}

//Auxiliar function of typeString(Type *t)
void typeStringAux(Type *t, char *s, char *buffer) {
    if (t->tag == simple) {
	switch(t->u.typeBase){
	case intType:
	    strcpy(buffer,"int");
	    break;
	case floatType:
	    strcpy(buffer,"float");
	    break;
	case charType:
	    strcpy(buffer,"char");
	    break;
	}
	strcat(buffer,s);
    } else {
		typeStringAux(t->u.type,"[]",buffer);
    }
}

//store in buffer type followed by []
//Examples:
//	1: int
//	2: char[][]
void typeString(Type *t, char *buffer) {
    if (t == NULL) {
		strcpy(buffer,"Void");
		return;
    }
    typeStringAux(t, "", buffer);
}

void throwIndexError(Type *t, int line) {
	char typeStr[MAX_TYPE];
	typeString(t, typeStr);
    printf("Error: index must be int, and is %s, Line %d\n",typeStr,line);
    exit(1);
}

void throwComparisonError(Type *t, int line) {
	char typeStr[MAX_TYPE];
	typeString(t, typeStr);
    printf("Error: Expression must be int or float, but is %s. Line %d\n",typeStr,line);
    exit(1);
}

void throwConditionalError(Type *t, int line) {
	char typeStr[MAX_TYPE];
	typeString(t, typeStr);
    printf("Error: Expression must be int, but is %s. Line %d\n",typeStr,line);
    exit(1);
}

Type* typeForIndex() {
    return TYPE_INT;
}

Type* newTypeForIndex() {
    return makeTypeSimple(intType);
}

Type* typeForConditional() {
    return TYPE_INT;
}

Type* newTypeForConditional() {
    return makeTypeSimple(intType);
}

Type* newTypeForComparison() {
    return makeTypeSimple(intType);
}

void makeComparison(Exp *exp1, Exp *exp2, Exp *parent) {
    parent->type = newTypeForComparison();
	//if exp1 is int and exp2 is float
    if (typeCmp(exp1->type,TYPE_INT) && typeCmp(exp2->type,TYPE_FLOAT)) {
		//make typecast
		Exp *tc = makeExpFromTypecast(exp1,makeTypeSimple(floatType),exp1->line);
		parent->u.bin.exp1 = tc;
		return;
    }
	//if exp2 is int and exp1 is float
    if (typeCmp(exp2->type,TYPE_INT) && typeCmp(exp1->type,TYPE_FLOAT)) {
		//make typecast
		Exp *tc = makeExpFromTypecast(exp2,makeTypeSimple(floatType),exp2->line);
		parent->u.bin.exp2 = tc;
		return;
    }
	//exp1 must be int or float
    if (typeCmp(exp1->type,TYPE_INT)==0 && typeCmp(exp1->type,TYPE_FLOAT)==0) {
		throwComparisonError(exp1->type,exp1->line);
    }
	//exp2 must be int or float
    if (typeCmp(exp2->type,TYPE_INT)==0 && typeCmp(exp2->type,TYPE_FLOAT)==0) {
		throwComparisonError(exp2->type,exp2->line);
    }
}

void makeArithmetic(Exp *exp1, Exp *exp2, Exp *parent) {
    Type *t1 = exp1->type;
    Type *t2 = exp2->type;
	//if both are int
    if (typeCmp(t1,TYPE_INT) && typeCmp(t2,TYPE_INT) ) { 
		parent->type = makeTypeSimple(intType);
	// if exp1 is int and exp2 is float
    } else if (typeCmp(t1,TYPE_INT) && typeCmp(t2,TYPE_FLOAT)) {
		parent->type = makeTypeSimple(floatType);
		//make typecast
		Exp *tc = makeExpFromTypecast(exp1, makeTypeSimple(floatType), exp1->line);
		parent->u.bin.exp1 = tc;
	//if exp1 is float and exp2 is int
    } else if (typeCmp(t1,TYPE_FLOAT) && typeCmp(t2,TYPE_INT)) {
		parent->type = makeTypeSimple(floatType);
		//make typecast
		Exp *tc = makeExpFromTypecast(exp2, makeTypeSimple(floatType), exp2->line);
		parent->u.bin.exp2 = tc;
	//if exp1 and exp2 are float
    } else if (typeCmp(t1,TYPE_FLOAT) && typeCmp(t2,TYPE_FLOAT)) {
		parent->type = makeTypeSimple(floatType);
	//exp1 must be int or float
    } else if (typeCmp(t1,TYPE_FLOAT)==0 && typeCmp(t1,TYPE_INT)==0){
		char typeStr[MAX_TYPE];
		typeString(t1, typeStr);
		printf("Error: +,-,*,/ operations only can be done with int or float, but is %s. Line %d\n",typeStr,parent->line);
		exit(1);
	//exp2 must be int or float
    } else if (typeCmp(t2,TYPE_FLOAT)==0 && typeCmp(t2,TYPE_INT)==0){
		char typeStr[MAX_TYPE];
		typeString(t2, typeStr);
		printf("Error: +,-,*,/ operations only can be done with int or float, but is %s. Line %d\n",typeStr,parent->line);
		exit(1);
    }
}

void throwModError(Type *t, int line) {
	char typeStr[MAX_TYPE];
	typeString(t, typeStr);
    printf("Error: %% (mod) operation is an operation with int and int, but it got a %s. Line %d\n",typeStr,line);
    exit(1);
}

void checkCall(Call *c);

void checkExp(Exp *exp) {
    switch(exp->tag) {
	case expE_expOr:
	case expE_expAnd: {
	    checkExp(exp->u.bin.exp1);
	    checkExp(exp->u.bin.exp2);
	    Type *t1 = exp->u.bin.exp1->type;
	    Type *t2 = exp->u.bin.exp2->type;
	    if (typeCmp(t1,typeForConditional())==0) {
			throwConditionalError(t1,exp->line);
	    }
	    if (typeCmp(t2,typeForConditional())==0) {
		throwConditionalError(t2,exp->line);
	    }
	    exp->type = newTypeForConditional();
	    break; 
	}
	case expE_expEE: 
	case expE_expNE: 
	case expE_expGreater: 
	case expE_expLess: 
	case expE_expGE: 
	case expE_expLE: 	
	    checkExp(exp->u.bin.exp1);
	    checkExp(exp->u.bin.exp2);
	    makeComparison(exp->u.bin.exp1,exp->u.bin.exp2,exp);
	    break;
	case expE_expAdd: 
	case expE_expMinus: 
	case expE_expMul: 
	case expE_expDiv:  
	    checkExp(exp->u.bin.exp1);
	    checkExp(exp->u.bin.exp2); 
	    makeArithmetic(exp->u.bin.exp1,exp->u.bin.exp2,exp);
	    break;
	case expE_expMod: {
	    checkExp(exp->u.bin.exp1);
	    checkExp(exp->u.bin.exp2);
	    Type *t1 = exp->u.bin.exp1->type;
	    Type *t2 = exp->u.bin.exp2->type;
		//exp1 and exp2 must be int
	    if (typeCmp(t1,TYPE_INT)==0) {
			throwModError(t1,exp->line);
	    }
	    if (typeCmp(t2,TYPE_INT)==0) {
			throwModError(t2,exp->line);
	    }
	    exp->type = makeTypeSimple(intType);
	    break;
	}
	case expE_expNot:   
	    checkExp(exp->u.expNot); 
	    exp->type = exp->u.expNot->type;
	    if (typeCmp(exp->type, typeForConditional()) == 0) {
			throwConditionalError(exp->u.expNot->type,exp->line);
	    }
	    break;
	case expE_expNew: {
	    checkExp(exp->u.newVal.exp);
	    Type *t1 = exp->u.newVal.exp->type;
	    if (typeCmp(t1,typeForIndex())==0) {
			throwIndexError(t1,exp->line);
	    }
	    exp->type = makeTypeIndexed(exp->u.newVal.type);
	    break;
	}
	case expE_negativeNum:
	    exp->type = exp->u.num->type;
	    break; 
	case expE_num:
	    exp->type = exp->u.num->type;
	    break;
	case expE_var:
	    switch(exp->u.var->tag) {
	    case simple: {
			Type *t1 = exp->u.var->type;
			if (typeCmp(t1,TYPE_CHAR)) {
		    	exp->type = makeTypeSimple(intType);
			} else {
		    	exp->type = t1;
			}
			break;
		}
	    case indexed: {
			checkExp(exp->u.var->u.indexed.expVar);
			checkExp(exp->u.var->u.indexed.expIndexed);
			//sew indexed var
			exp->u.var->type = exp->u.var->u.indexed.expVar->type->u.type;
			Type *typeExpVar = exp->u.var->u.indexed.expVar->type;
			Type *typeExpIndexed = exp->u.var->u.indexed.expIndexed->type;
			if (typeCmp(typeExpIndexed,typeForIndex())==0) {
		    	throwIndexError(typeExpIndexed,exp->line);
			}
			switch(typeExpVar->tag) {
			case simple: {
				char typeStr[MAX_TYPE];
				typeString(typeExpVar, typeStr);
		   		printf("Error: Expression in var indexed must be an array, but is %s. Line %d\n",typeStr,exp->line);
		    	exit(1);
		    	break;
			}
			case indexed:	
		    	exp->type = typeExpVar->u.type;
		    	break;
			}
			break;
			}
	    } 
	    break;
	case expE_call:
	    checkCall(exp->u.call);
	    exp->type = exp->u.call->typeInfo.dec->type;
	    break;
	case expE_typecast:
	    checkExp(exp->u.expCasted);
	    break;
    }
}

void checkCall(Call *c) {
    Exp *e = NULL;
    DecVar *param = NULL;
	//for all parameters and arguments
    for (e = c->listExp, param = c->typeInfo.dec->params; e!=NULL; e=e->next, param = param->next) {
		checkExp(e);
		//more arguments than parameters
		if (param == NULL) {
	    	printf("Error: More arguments that necessary on call. Line %d\n",e->line);
	    	exit(1);
		}
		Type *t1 = e->type;
		Type *t2 = param->type;
		//check if parameter and argument types are equal
		if (typeCmp(t1,t2) == 0) {	
			char typeStr1[MAX_TYPE];
			typeString(t1, typeStr1);
			char typeStr2[MAX_TYPE];
			typeString(t2, typeStr2);
	   		printf("Error: The argument of type %s can't be set to parameter of type %s. Line %d\n",
					typeStr1,typeStr2,e->line);
	    	exit(1);
		}
    }
	//more parameters than arguments
    if (e == NULL && param != NULL) {
		printf("Error: Missing arguments in call. Line %d\n",c->line);
		exit(1);
    }
}

void checkReturn(ReturnNode *r) {
    int cmp;
	//if returns nothing, the func must be void
    if (r->exp == NULL) {
    	cmp = typeCmp(NULL, r->decFunc->type);
	if (cmp == 0) {
	    printf("Error: Expected expression in return. Line %d\n",r->line);
	    exit(1);
	}
    } else {
		//check if return type is equal than function type
		checkExp(r->exp);
		Type *t1 = r->exp->type;
        Type *t2 = r->decFunc->type;
		cmp = typeCmp(t1,t2);
	if (cmp == 0) {
		char typeStr1[MAX_TYPE];
		typeString(t1, typeStr1);
		char typeStr2[MAX_TYPE];
		typeString(t2, typeStr2);
	    printf("Error: The function must return a %s, but is returning a %s, Line %d\n",typeStr2,typeStr1,r->line);
	    exit(1);
	}
    }
}

void checkBlock(Block *b);

void checkCommand(Command *command) {
    for(Command *c = command; c!=NULL; c=c->next) {
		int cmp;
		Type *t1;
		Exp *expIndexed;
		Exp *expVar;
		if (c->tag == assignmentE) {
	    	expIndexed = c->u.assignment.var->u.indexed.expIndexed;
	   		expVar = c->u.assignment.var->u.indexed.expVar;
		}
        switch(c->tag) {
	    case assignmentE: {
			Type *varType;
			Exp *exp; 
			checkExp(c->u.assignment.exp);
			//check var
			switch(c->u.assignment.var->tag) {
			case simple:
		    	varType = c->u.assignment.var->type;
		    	break;
			case indexed:
		    	checkExp(expVar);
		    	checkExp(expIndexed);
				//check index type
		    	if (typeCmp(expIndexed->type,typeForIndex())==0) {
		        	throwIndexError(expIndexed->type,expIndexed->line);
		    	}
				//check exp (must be of type array)
		    	switch(expVar->type->tag) {
		    	case simple: {
					char typeExpVar[MAX_TYPE];
					typeString(expVar->type, typeExpVar);
		        	printf("Error: Expression in var indexed must be an array, but is %s. Line %d\n",
							typeExpVar,expVar->line);
		        	exit(1);
		        	break;
				}
		    	case indexed:	
					//all ok
		        	break;
		    	}//end switch

		    	//sew indexed var
		    	c->u.assignment.var->type = expVar->type->u.type;
		    	varType = c->u.assignment.var->type;
		    	break;
			}//end switch

			exp = c->u.assignment.exp;
	
			//if is a float = int assignment
			if (typeCmp(varType,TYPE_FLOAT) && typeCmp(exp->type,TYPE_INT)) {
				//make typecast
				Exp *tc = makeExpFromTypecast(exp, makeTypeSimple(floatType), exp->line);
				c->u.assignment.exp = tc;
				exp = tc;
			}
			cmp = typeCmp(varType,exp->type);
			if (cmp == 0) {
				char varTypeString[MAX_TYPE];
				typeString(varType, varTypeString);
				char expTypeString[MAX_TYPE];
				typeString(exp->type, expTypeString);
		    	printf("Error: The variable of type %s doesn't accept %s. Line %d\n",varTypeString,expTypeString,c->line);
		    	exit(1);
			}  
			break;
		}//end case
	    case callE:
			checkCall(c->u.call);
			break;
	    case returnCmdE: 
			checkReturn(c->u.returnNode);
			break;
	    case whileCmdE:
			checkExp(c->u.whileCmd.exp);
			checkCommand(c->u.whileCmd.command);
			t1 = c->u.whileCmd.exp->type;
			cmp = typeCmp(t1, typeForConditional());
			if (cmp == 0) {
		    	throwConditionalError(t1,c->line);
			} 
			break;
	    case blockE: 
			checkBlock(c->u.block);
			break;
	    case ifCmdE: 
			checkExp(c->u.ifCmd.exp);
			checkCommand(c->u.ifCmd.command);
			t1 = c->u.ifCmd.exp->type;
			cmp = typeCmp(t1, typeForConditional());
			if (cmp == 0) {
		    	throwConditionalError(t1,c->line);
			} 
			break;
	    case ifElseE:
			checkExp(c->u.ifElse.exp);
			checkCommand(c->u.ifElse.command1); 
			checkCommand(c->u.ifElse.command2);
			t1 = c->u.ifElse.exp->type;
			cmp = typeCmp(t1, typeForConditional());
			if (cmp == 0) {
		    	throwConditionalError(t1,c->line);
			} 
			break;
		case print:
			checkExp(c->u.printExp);
			Type *expType = c->u.printExp->type;
			//if is an array but not an array of char
			if (expType->tag == indexed && !(expType->u.type->tag == simple && expType->u.type->u.typeBase == charType)) {
				printf("Error: cannot print arrays that are not char[]\n");
				exit(1);
			}
			break;
       }//end switch
    }//end for
}//end func

void checkBlock(Block *block) {
    checkCommand(block->command);
}

void checkDecFunc(DecFunc *decFunc){
    checkBlock(decFunc->block);
}


void checkDeclaration(Declaration *dec){
    switch(dec->tag) {
    case decFunc: 
	    checkDecFunc(dec->u.df); 
	    break;
	case decVar: 
	    break;
    }
}

void checkTree(Program *p) {
    TYPE_INT = makeTypeSimple(intType);
    TYPE_FLOAT = makeTypeSimple(floatType);
    TYPE_CHAR = makeTypeSimple(charType);
    for(Declaration *d = p->dec;d!=NULL;d=d->next) {
        checkDeclaration(d);
    }
    free(TYPE_INT);
    free(TYPE_FLOAT);
    free(TYPE_CHAR);
}
