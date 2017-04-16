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
#include "typeChecker.h"
#include "generateLLVM.h"

#define CONVERT_CHAR 1
#define MAX_TYPE 50
#define MAX_CALL MAX_TYPE*20

int hasReturn   = 0;
int tempCount   = 0;
int labelCount  = 0;
int stringCount = 0;
//maximum 100 strings of 100 characters
char strings[100][100];
Type *currentFuncType;

FILE *f;

void GENNum(Num *num, int expTemp ,int multiplier) {
    switch(num->type->tag) {
	case simple: 
	    switch(num->type->u.typeBase) {
	    case intType:
			fprintf(f,"  %%t%d = add i32 0, %d\n",expTemp, num->u.intValue * multiplier);
			break;
	    case floatType: {
			fprintf(f,"  %%t%d = fadd float 0.0, 0x",expTemp);
			//this chunk below prints a float in hexa, conforming to llvm
			float floatValue = num->u.floatValue * multiplier;
            double d = floatValue;
			unsigned char buff[sizeof(double)];
			memcpy(buff, &d, sizeof(double));
			for (int i = sizeof(double) - 1; i >= 0; i--)
				fprintf(f,"%02X", buff[i]);
			fprintf(f,"\n");
			break;
		}
	    case charType:
			//Num can't be char, only Var can be
			printf("ERROR char\n");
			exit(1);
	    }
	    break;
	case indexed: {
		//handling string
		char *string = num->u.stringValue;
		strcpy(strings[stringCount],string);
		int stringTemp = ++stringCount;
		num->stringTemp = stringTemp;
		int size = strlen(string)+1;
	    fprintf(f,"  %%t%d = getelementptr inbounds [%d x i8], [%d x i8]* @str%d, i32 0, i32 0\n",expTemp, size, size, stringTemp);
        break;
		}
    }	
}

int GENExp(Exp *exp);

//store in buffer token
void getTokenFromType(Type *type, int convertChar, char *buffer) {
	if (type == NULL) { 
		strcpy(buffer,"void");
		return;
	}
	switch(type->tag) {
	case simple:
		switch(type->u.typeBase){
		case intType:
			strcpy(buffer,"i32");
			break;
		case charType:
			if (convertChar)
				strcpy(buffer,"i32");
			else
				strcpy(buffer,"i8");
			break;
		case floatType:
			strcpy(buffer,"float");
			break;
		}
		break;
	case indexed: {
		char tempBuffer[MAX_TYPE]; //the bigger possible string is float*, having 7 characters
		int dontCast = 0; //array of char is not casted to int
		getTokenFromType(type->u.type,dontCast,tempBuffer);
		strcpy(buffer, tempBuffer);
		strcat(buffer, "*");
		break;
		}
	}
}

//returns token for operation.
char* getTokenFromOperation(ExpE op, TypeBaseE typeBase) {
	switch(typeBase) {
	case intType:
	case charType:
		switch(op) {
		case  expE_expAdd:  return "add";
		case expE_expMinus: return "sub";
		case expE_expMul:   return "mul";
		case expE_expDiv:   return "sdiv";
		case expE_expMod:   return "srem";
		default: printf("Error: invalid operation\n"); exit(1);
		}
	case floatType:
		switch(op) {
		case  expE_expAdd:  return "fadd";
		case expE_expMinus: return "fsub";
		case expE_expMul:   return "fmul";
		case expE_expDiv:   return "fdiv";
		case expE_expMod:   return "frem";
		default: printf("Error: invalid operation\n"); exit(1);
		}
	}
}

char* getTokenFromCond(ExpE tag, TypeBaseE typeBase) {
	switch(typeBase) {
	case intType:
	case charType:
		switch(tag) {
		case expE_expEE:      return "eq";
		case expE_expNE:      return "ne";
		case expE_expGreater: return "sgt";
		case expE_expLess:    return "slt";
		case expE_expGE:      return "sge";
		case expE_expLE:      return "sle";
		default: printf("Error: invalid conditional\n"); exit(1);
		break;
		}
		break;
	case floatType:
		switch(tag) {
		case expE_expEE:      return "oeq";
		case expE_expNE:      return "one";
		case expE_expGreater: return "ogt";
		case expE_expLess:    return "olt";
		case expE_expGE:      return "oge";
		case expE_expLE:      return "ole";
		default: printf("Error: invalid conditional\n"); exit(1);
		break;
		}
		break;
	}
}

int GENExp(Exp *exp);

void GENCond(Exp *exp, int labelTrue, int labelFalse) {
	switch(exp->tag) {
	case expE_expOr: {
		int newLabel = ++labelCount;
		GENCond(exp->u.bin.exp1,labelTrue,newLabel);
		fprintf(f,"l%d:\n",newLabel);
		GENCond(exp->u.bin.exp2,labelTrue,labelFalse);
		break;
	}
    case expE_expAnd: {
		int newLabel = ++labelCount;
		GENCond(exp->u.bin.exp1,newLabel,labelFalse);
		fprintf(f,"l%d:\n",newLabel);
		GENCond(exp->u.bin.exp2,labelTrue,labelFalse);
		break;
	}
	case expE_expEE: 
    case expE_expNE: 
    case expE_expGreater: 
    case expE_expLess: 
    case expE_expGE: 
	case expE_expLE: {
		if (exp->u.bin.exp1->type->tag != simple) { printf("Error: Exp must be simple\n"); exit(1); }
		if (exp->u.bin.exp2->type->tag != simple) { printf("Error: Exp must be simple\n"); exit(1); }
		int t1 = GENExp(exp->u.bin.exp1);
		int t2 = GENExp(exp->u.bin.exp2);
		//if it is an int or char comparison
		if (exp->u.bin.exp1->type->u.typeBase == intType || exp->u.bin.exp1->type->u.typeBase == charType) {
			char *token = getTokenFromCond(exp->tag, intType);
			int expTemp = ++tempCount;
			fprintf(f,"  %%t%d = icmp %s i32 %%t%d, %%t%d\n",expTemp,token,t1,t2);
			fprintf(f,"  br i1 %%t%d, label %%l%d, label %%l%d\n",expTemp,labelTrue,labelFalse);
		} 
		//if it is a float comparison
		else {
			char *token = getTokenFromCond(exp->tag, floatType);
			int expTemp = ++tempCount;
			fprintf(f,"  %%t%d = fcmp %s float %%t%d, %%t%d\n",expTemp,token,t1,t2);
			fprintf(f,"  br i1 %%t%d, label %%l%d, label %%l%d\n",expTemp,labelTrue,labelFalse);
		}
		break;
	}
	case expE_expNot: 
		GENCond(exp->u.expNot,labelFalse,labelTrue); 
		break;
	default: {
		int t1 = GENExp(exp);
		int expTemp = ++tempCount;
		//Exp only can be int here (because it's a conditional)
		fprintf(f,"  %%t%d = icmp ne i32 %%t%d, 0\n",expTemp,t1);
		fprintf(f,"  br i1 %%t%d, label %%l%d, label %%l%d\n",expTemp,labelTrue,labelFalse);
		break;
		}
	}
}

//store in buffer string that makes the call
void GENCall(Call *call, char *buffer) {
	char args[200];
	strcpy(args,"");
	char argToken[50];
	for (Exp *arg = call->listExp; arg != NULL; arg = arg->next) {
		int argTemp = GENExp(arg);
		char argType[MAX_TYPE]; 
		getTokenFromType(arg->type,CONVERT_CHAR,argType);
		if (arg->next != NULL) {
			sprintf(argToken,"%s %%t%d, ",argType,argTemp);
		}
		else {
			sprintf(argToken,"%s %%t%d",argType,argTemp);
		}
		strcat(args,argToken);
	}
	char funcType[MAX_TYPE]; 
	getTokenFromType(call->typeInfo.dec->type,CONVERT_CHAR,funcType);
	sprintf(buffer,"call %s @%s( %s )",funcType,call->id,args);
}

int GENNew(int size, int indexTemp, char *typeToken) {
	int sizeTemp = ++tempCount;
	fprintf(f,"  %%t%d = mul i32 %%t%d, %d\n", sizeTemp, indexTemp, size);
	int mallocTemp = ++tempCount;
	fprintf(f,"  %%t%d = call i8* @malloc(i32 %%t%d)\n", mallocTemp, sizeTemp);
	return mallocTemp;
}

int GENNewWithCast(int size, int indexTemp, char *typeToken) {
	int mallocTemp = GENNew(size, indexTemp, typeToken);
	//cast
	int castTemp = ++tempCount;
	fprintf(f,"  %%t%d = bitcast i8* %%t%d to %s*\n", castTemp,mallocTemp,typeToken);
	return castTemp;
}

int castCharToInt(int expTemp) {
	int castTemp = ++tempCount;
	fprintf(f,"  %%t%d = sext i8 %%t%d to i32\n",castTemp,expTemp); 
	return castTemp;
}

int GENExp(Exp *exp) {
    switch(exp->tag) {
    case expE_expAdd: 
	case expE_expMinus: 
	case expE_expMul: 
	case expE_expDiv: 
	case expE_expMod: {
		int t1 = GENExp(exp->u.bin.exp1); 
		int t2 = GENExp(exp->u.bin.exp2);
		tempCount += 1;
		if (exp->type->tag != simple) { printf("Error: Exp must be simple\n"); exit(1); }
		TypeBaseE typeBase = exp->type->u.typeBase;
		char *tokenOp  = getTokenFromOperation(exp->tag, typeBase);
		char tokenType[MAX_TYPE];
		getTokenFromType(exp->type,CONVERT_CHAR,tokenType);
		fprintf(f,"  %%t%d = %s %s %%t%d, %%t%d\n",tempCount,tokenOp,tokenType,t1,t2);
		return tempCount; 
		break;
	}
	case expE_typecast: {
		int t1 = GENExp(exp->u.expCasted);
		tempCount += 1;
		//expCasted must be int
		if (exp->u.expCasted->type->tag != simple || exp->u.expCasted->type->u.typeBase != intType) { 
			printf("Error: ExpCasted must be int\n"); 
			exit(1); 
		}
		//exp must be float
		if (exp->type->tag != simple || exp->type->u.typeBase != floatType) { 
			printf("Error: exp must be float\n");
			exit(1); 
		}
		fprintf(f,"  %%t%d = sitofp i32 %%t%d to float\n",tempCount,t1);
		return tempCount;
		break;
	}
    case expE_expNew: {
		int indexTemp = GENExp(exp->u.newVal.exp);
		int size;
		char typeToken[MAX_TYPE];
		getTokenFromType(exp->u.newVal.type, !CONVERT_CHAR, typeToken);
		switch(exp->u.newVal.type->tag) {
			case indexed: {
				size = sizeof(void*);
				return GENNewWithCast(size, indexTemp, typeToken);
			}
			case simple:
				switch(exp->u.newVal.type->u.typeBase) {
				case intType: {
					size = sizeof(int);
					return GENNewWithCast(size, indexTemp, typeToken);
				}
				case floatType: {
					size = sizeof(float);
					return GENNewWithCast(size, indexTemp, typeToken);
				}
				case charType:
					size = sizeof(char);
					return GENNew(size, indexTemp, typeToken);
				}
		}
	}
	case expE_negativeNum: 
		tempCount += 1;
		GENNum(exp->u.num,tempCount,-1);
		return tempCount;
	case expE_num: 
		tempCount += 1;
		GENNum(exp->u.num,tempCount,1);
		return tempCount;
    case expE_var: {
		Var *var = exp->u.var;
		switch(var->tag) {
			case simple: {
				int newTemp = ++tempCount;
				char tokenType[MAX_TYPE];
				getTokenFromType(var->type,!CONVERT_CHAR, tokenType);
				int varTemp = var->decVar->temp;
				char *varScope = var->decVar->scope;
				fprintf(f,"  %%t%d = load %s, %s* %st%d\n",newTemp,tokenType,tokenType,varScope,varTemp);	
				//if var is char
				if(var->type->tag == simple && var->type->u.typeBase == charType) {
					return castCharToInt(newTemp);
				}
				return newTemp;
			}
			case indexed: {
				int expVarTemp = GENExp(var->u.indexed.expVar);
				int expIndexedTemp = GENExp(var->u.indexed.expIndexed);
				char typeExpVar[MAX_TYPE];
				getTokenFromType(var->u.indexed.expVar->type->u.type, !CONVERT_CHAR, typeExpVar);
				int elementPtrTemp = ++tempCount;
				fprintf(f,"  %%t%d = getelementptr inbounds %s, %s* %%t%d, i32 %%t%d\n", 
						elementPtrTemp, typeExpVar, typeExpVar, expVarTemp, expIndexedTemp);
				int expTemp = ++tempCount;
				fprintf(f,"  %%t%d = load %s, %s* %%t%d\n", expTemp, typeExpVar, typeExpVar, elementPtrTemp);  
				//if expVar is char
				if(var->u.indexed.expVar->type->tag == indexed && var->u.indexed.expVar->type->u.type->u.typeBase == charType) {
					return castCharToInt(expTemp);
				}
				return expTemp;
			}
		}
		break;
	}
    case expE_call: { 
		char call[MAX_CALL];
		GENCall(exp->u.call, call);
		int expTemp = ++tempCount;
		fprintf(f,"  %%t%d = %s\n",expTemp,call);
		return expTemp;
	} 
	default: {
		int tempOut = ++tempCount;
		int labelTrue = ++labelCount;
		int labelFalse = ++labelCount;
		
		//evaluates conditional
		GENCond(exp,labelTrue,labelFalse);

		//if conditional is true
		int labelOut = ++labelCount;
		fprintf(f,"l%d:\n",labelTrue);
		fprintf(f,"  br label %%l%d\n",labelOut);

		//if conditional is false
		fprintf(f,"l%d:\n",labelFalse);
		fprintf(f,"  br label %%l%d\n",labelOut);

		//continue flow
		fprintf(f,"l%d:\n",labelOut);
		fprintf(f,"  %%t%d = phi i32 [1, %%l%d], [0, %%l%d]\n",tempOut,labelTrue,labelFalse);
		
		return tempOut;
		}
    }
    return 0;//to remove warnings
}

void GENReturn(ReturnNode *r) {
    hasReturn = 1;
    if (r->exp != NULL) {
		int expTemp = GENExp(r->exp);
		char tokenType[MAX_TYPE];
		getTokenFromType(r->exp->type,CONVERT_CHAR, tokenType);
		fprintf(f,"  ret %s %%t%d\n",tokenType,expTemp);
    } else {
		fprintf(f,"ret ");
		fprintf(f,"void\n");
    }
}

char* getTypeValue(Type *type) {
	switch(type->tag) {
	case indexed: return "null";
	case simple: return "0";
	}
}

void generateUselessReturn(int labelOut) {
	char retType[MAX_TYPE];
	getTokenFromType(currentFuncType, CONVERT_CHAR, retType);
	char *retValue = getTypeValue(currentFuncType);
	fprintf(f,"  ret %s %s\n", retType, retValue);
}

void GENBlock(Block *block);

void GENCommand(Command *command) {
    for(Command *c = command; c!=NULL; c=c->next) {
        switch(c->tag) {
	    case assignmentE: {
			int rightExpTemp = GENExp(c->u.assignment.exp);
			Var *var = c->u.assignment.var;
			switch(var->tag) {
			case simple: {
				int tempVar = var->decVar->temp;
				char *scopeVar = var->decVar->scope;
				//if var is char
				if (var->type->u.typeBase == charType) {
					//make cast
					int tempCast = ++tempCount;
					fprintf(f,"  %%t%d = trunc i32 %%t%d to i8\n",tempCast,rightExpTemp);
					char tokenType[MAX_TYPE];
					getTokenFromType(var->type,!CONVERT_CHAR, tokenType);
					fprintf(f,"  store %s %%t%d, %s* %st%d\n",tokenType,tempCast,tokenType,scopeVar,tempVar);
				} else {
					char tokenType[MAX_TYPE];
					getTokenFromType(var->type,!CONVERT_CHAR, tokenType);
					fprintf(f,"  store %s %%t%d, %s* %st%d\n",tokenType,rightExpTemp,tokenType,scopeVar,tempVar);
				}
				break;
			}
			case indexed: {
				int expVarTemp = GENExp(var->u.indexed.expVar);
				int expIndexedTemp = GENExp(var->u.indexed.expIndexed);
				char typeExpVar[MAX_TYPE];
				getTokenFromType(var->u.indexed.expVar->type->u.type, !CONVERT_CHAR, typeExpVar);
				int elementPtrTemp = ++tempCount;
				fprintf(f,"  %%t%d = getelementptr inbounds %s, %s* %%t%d, i32 %%t%d\n", 
						elementPtrTemp, typeExpVar, typeExpVar, expVarTemp, expIndexedTemp);
				//if expVar is char
				int tempCast;
				if (var->u.indexed.expVar->type->u.type->u.typeBase == charType) {
					//make cast
					tempCast = ++tempCount;
					fprintf(f,"  %%t%d = trunc i32 %%t%d to i8\n",tempCast,rightExpTemp);
					fprintf(f,"  store %s %%t%d, %s* %%t%d\n", typeExpVar, tempCast, typeExpVar, elementPtrTemp);
				} else {
					fprintf(f,"  store %s %%t%d, %s* %%t%d\n", typeExpVar, rightExpTemp, typeExpVar, elementPtrTemp);  
				}
				break;
				}
			} 
			break;
		}
	    case callE: {
			char call[MAX_CALL];
			GENCall(c->u.call, call);
			fprintf(f,"  %s\n",call);
			break;
		}
	    case returnCmdE: 
			GENReturn(c->u.returnNode);
			break;
	    case whileCmdE: {
			int labelCond = ++labelCount;

			//check condition
			fprintf(f,"  br label %%l%d\n",labelCond);
			fprintf(f,"l%d:\n",labelCond);
			int labelBody = ++labelCount;
			int labelOut = ++labelCount;
			GENCond(c->u.whileCmd.exp, labelBody, labelOut);

			//enter body
			fprintf(f,"l%d:\n",labelBody);
			GENCommand(c->u.whileCmd.command);
			//go back to condition
			fprintf(f,"  br label %%l%d\n",labelCond);

			//continue flow after while
			if (c->next != NULL) {
				fprintf(f,"\nl%d:\n",labelOut);
			} else {
				if (currentFuncType != NULL) {
					generateUselessReturn(labelOut);
				} else {
					fprintf(f,"\nl%d:\n",labelOut);
					fprintf(f,"  ret void\n");
				}
			}
			break;
		}
	    case blockE:
			GENBlock(c->u.block);
			break;
	    case ifCmdE: {
			int labelTrue = ++labelCount;
			int labelOut = ++labelCount;
			
			//check condition
			GENCond(c->u.ifCmd.exp,labelTrue,labelOut);
		
			//enter if
			fprintf(f,"\nl%d:\n",labelTrue);
			GENCommand(c->u.ifCmd.command);
			fprintf(f,"  br label %%l%d\n",labelOut);
 
			//continue flow after if
			if (c->next != NULL) {
				fprintf(f,"\nl%d:\n",labelOut);
			} else {
				if (currentFuncType != NULL) {
					generateUselessReturn(labelOut);
				} else {
					fprintf(f,"\nl%d:\n",labelOut);
					fprintf(f,"  ret void\n");
				}
			}
			break;
		}
	    case ifElseE: {
			int labelTrue = ++labelCount;
			int labelFalse = ++labelCount; 
			
			//check condition
			GENCond(c->u.ifElse.exp,labelTrue,labelFalse);

			//enter if
			fprintf(f,"\nl%d:\n",labelTrue);
			GENCommand(c->u.ifElse.command1);
			int labelOut = ++labelCount;
			fprintf(f,"  br label %%l%d\n",labelOut);

			//enter else
			fprintf(f,"\nl%d:\n",labelFalse);
			GENCommand(c->u.ifElse.command2);
			fprintf(f,"  br label %%l%d\n",labelOut);
		
			//continue flow after if or else
			if (c->next != NULL) {
				fprintf(f,"\nl%d:\n",labelOut);
			} else {
				if (currentFuncType != NULL) {
					generateUselessReturn(labelOut);
				} else {
					fprintf(f,"\nl%d:\n",labelOut);
					fprintf(f,"  ret void\n");
				}
			}
			break;
		}
		case print: {
			Exp *printExp = c->u.printExp;
			int expTemp = GENExp(printExp);
			int strTemp = ++tempCount;
			switch(printExp->type->tag) {
			case simple: {
				switch(printExp->type->u.typeBase) {
				case charType:
				case intType:
					fprintf(f,"  %%t%d = getelementptr inbounds [3 x i8], [3 x i8]* @strInt, i32 0, i32 0\n", strTemp);
					fprintf(f,"  call i32 (i8*, ...) @printf(i8* %%t%d, i32 %%t%d)\n",strTemp,expTemp);
					break;
				case floatType: {
					fprintf(f,"  %%t%d = getelementptr inbounds [3 x i8], [3 x i8]* @strFloat, i32 0, i32 0\n", strTemp);
					int castTemp = ++tempCount;
					fprintf(f," %%t%d = fpext float %%t%d to double\n",castTemp,expTemp);
					fprintf(f,"  call i32 (i8*, ...) @printf(i8* %%t%d, double %%t%d)\n",strTemp,castTemp);
					break;
					}
				}
				break;
			}
			case indexed:
			//assuming it is a string
				fprintf(f,"  %%t%d = getelementptr inbounds [3 x i8], [3 x i8]* @strArrayChar, i32 0, i32 0\n", strTemp);
				fprintf(f,"  call i32 (i8*, ...) @printf(i8* %%t%d, i8* %%t%d)\n",strTemp,expTemp);
				break;
			}
			break;
		}

       }
    }
}

//calls recursively all decVar list
void GENDecVarAux(DecVar *decVar, int isParameter, int paramTemp) {
	if (decVar != NULL) {
		decVar->temp = ++tempCount;
		decVar->scope = "%";
		char tokenType[MAX_TYPE];
		getTokenFromType(decVar->type,!CONVERT_CHAR, tokenType);
		fprintf(f,"  %%t%d = alloca %s\n",tempCount,tokenType);
		if (isParameter) {
			//if parameter is char
			if (decVar->type->tag == simple && decVar->type->u.typeBase == charType) {
				//make casting from argument
				int argTemp = ++tempCount;
				fprintf(f,"  %%t%d = trunc i32 %%%d to i8\n",argTemp,paramTemp);
				fprintf(f,"  store %s %%t%d, %s* %%t%d\n",tokenType,argTemp,tokenType,decVar->temp);
			} else {
				fprintf(f,"  store %s %%%d, %s* %%t%d\n",tokenType,paramTemp,tokenType,decVar->temp);
			}
		}
		GENDecVarAux(decVar->next, isParameter, paramTemp+1);
    }
}

void GENDecVar(DecVar *decVar, int isParameter){
	//call func below for all params
    GENDecVarAux(decVar, isParameter, 0);
}

//call recursively all decVar list
void GENGlobalDecVar(DecVar *decVar) {
	if (decVar != NULL) {
		decVar->temp = ++tempCount;
		decVar->scope = "@";
		//initializes with arbitrary value
		char typeToken[MAX_TYPE];
		getTokenFromType(decVar->type,!CONVERT_CHAR, typeToken);
		char *initialValue = getTypeValue(decVar->type);
		fprintf(f,"@t%d = global %s %s\n",decVar->temp, typeToken, initialValue);
		GENGlobalDecVar(decVar->next);
	}
}

void GENBlock(Block *block) {
	GENDecVar(block->decVar,0);
	if (block->command != NULL) {
    	GENCommand(block->command);
	}
}

void GENDecFunc(DecFunc *decFunc){
	tempCount = 0;
	labelCount = 0;
	currentFuncType = decFunc->type;
    fprintf(f,"\ndefine ");
    if (decFunc->type == NULL) {
		hasReturn = 0;
        fprintf(f,"void ");
    } else {
	hasReturn = 1;
        char paramToken[MAX_TYPE];
		getTokenFromType(decFunc->type,CONVERT_CHAR, paramToken);
		fprintf(f,"%s ",paramToken);
    }
    fprintf(f,"@%s ",decFunc->id);
    if (decFunc->params == NULL) {    
		fprintf(f,"()");
		fprintf(f," {\n");
    } else{
		fprintf(f,"(");
		for (DecVar *d = decFunc->params; d != NULL; d = d->next) {
			char paramToken[MAX_TYPE];
			getTokenFromType(d->type,CONVERT_CHAR, paramToken);
			if (d->next == NULL) {
				fprintf(f,"%s)",paramToken);
				fprintf(f," {\n");
			} else {
				fprintf(f,"%s,",paramToken);
			}
		}
		GENDecVar(decFunc->params,1);
    }
    GENBlock(decFunc->block);
    if (hasReturn == 0) {
		fprintf(f,"  ret void\n");
    }
    fprintf(f,"}\n");
}

void GENDeclaration(Declaration *dec){
    switch(dec->tag) {
        case decVar:
			GENGlobalDecVar(dec->u.dv);
			break;
        case decFunc: 
			GENDecFunc(dec->u.df); 
			break;
    }
}

//print string, handling special characters
void printString(char *str) { 
	int i;
	for (i=0; i < strlen(str); i++) {
		if (str[i] == '\n')
			fprintf(f,"\\0A");
		else if (str[i] == '\\')
			fprintf(f,"\\5C");
		else if (str[i] == '\"')
			fprintf(f,"\\22");
		else if (str[i] == '\t')
			fprintf(f,"\\09");
		else
			fprintf(f,"%c",str[i]);
	}
}

void generateLLVM(Program *p){
	f = fopen("LLVM_CODE.ll","w");

	fprintf(f,"\ndeclare i8* @malloc(i32)\n");
	fprintf(f,"declare i32 @printf(i8*, ...)\n");
	fprintf(f,"@strFloat = private unnamed_addr constant [3 x i8] c\"%%f\\00\"\n");
	fprintf(f,"@strInt = private unnamed_addr constant [3 x i8] c\"%%d\\00\"\n");
	fprintf(f,"@strChar = private unnamed_addr constant [3 x i8] c\"%%c\\00\"\n");
	fprintf(f,"@strArrayChar = private unnamed_addr constant [3 x i8] c\"%%s\\00\"\n");
	for(Declaration *d = p->dec;d!=NULL;d=d->next) {
		GENDeclaration(d);
	}
	fprintf(f,"\n");
	int i;
	for (i=0; i<stringCount; i++) {
		fprintf(f,"@str%d = private unnamed_addr global [%d x i8] c\"", i+1, (int) strlen(strings[i])+1);
		printString(strings[i]);	
		fprintf(f,"\\00\"\n");
	}

	fclose(f);
}
