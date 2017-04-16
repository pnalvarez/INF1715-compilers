/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "seamDataStructure.h"

typedef struct c_node {
    DecE tag;
    union {
		DecVar *decVar;
		DecFunc *decFunc;
    }u;
    struct c_node *next;
}c_Node;

typedef struct scope {
    c_Node *node;
    struct scope *next;
    struct scope *previous;
}Scope;

Scope* scopeStack = NULL;

void freeNodeList(c_Node *n) {
    if (n == NULL) { return; }
    if (n->next != NULL) {
		freeNodeList(n->next);
    }
    free(n);
}

void freeScopeList(Scope *s) {
    if (s == NULL) { return; }
    freeNodeList(s->node);
    if (s->next != NULL) {
		freeScopeList(s->next);
    }
    if (s->previous != NULL) {
		s->previous->next = NULL;
    }
    free(s);
}

Scope* lastScope(Scope *s) {
    if (s == NULL) {
		return NULL;
    }
    if (s->next == NULL) {
		return s;
    }
    return lastScope(s->next);
}

c_Node* lastNodeFromScope(c_Node *n) {
    if (n == NULL) { return NULL; }
    if (n->next == NULL) {
		return n;
    }
    return lastNodeFromScope(n->next);
}

void enterScope() {
    Scope *s = (Scope*) malloc(sizeof(Scope));
    if (s==NULL) { exit(1); }
    s->node = NULL;
    s->next = NULL;
    if (scopeStack == NULL) {
		s->previous = NULL;
        scopeStack = s;
    } else {
	Scope *last = lastScope(scopeStack);
        last->next = s;
        s->previous = last;
    }
}

void leaveScope() {
    if (scopeStack == NULL) { return; }
    Scope *last = lastScope(scopeStack);
    if (last == NULL) { return; }
    if (last == scopeStack) {
        scopeStack = NULL;
    }
    freeScopeList(last);
}

void* findCurrentScope(char *id, DecE tag) {
    Scope *currentScope = lastScope(scopeStack);
    if (currentScope == NULL) { return NULL; }
    c_Node *node;
    for (node = currentScope->node ; node != NULL ; node = node->next) {
		switch(node->tag) {
		case decVar:
		    if (strcmp(node->u.decVar->id,id) == 0 && node->tag == tag) {
		        return node->u.decVar;
            }
	    break;
		case decFunc:
	 	   if (strcmp(node->u.decFunc->id,id) == 0 && node->tag == tag) {
	  	      return node->u.decFunc;
           }
	    break;
		}
    }
    return NULL;
}

DecVar* findDecVarCurrentScope(char *id) {
    return (DecVar*) findCurrentScope(id, decVar);
}

DecFunc* findDecFuncCurrentScope(char *id) {
    return (DecFunc*) findCurrentScope(id, decFunc);
}

void appendToEndOfListInThisScope(c_Node *n) {
    Scope *currentScope = lastScope(scopeStack);
    if (currentScope == NULL) { return; }
    c_Node *lastNode = lastNodeFromScope(currentScope->node);
    if (lastNode == NULL) { 
		currentScope->node = n; 
    } else {
        lastNode->next = n;
    }
}

ErrorSeam insertDecFunc(DecFunc *d) {
    if (findDecFuncCurrentScope(d->id) != NULL) {
		return redefinitionError;
    }
    //Create node
    c_Node *n = (c_Node*) malloc(sizeof(c_Node));
    if (n == NULL) { printf("out of memory\n"); exit(1);}
    n->tag = decFunc;
    n->u.decFunc = d;
    n->next = NULL;
    
    //Append to end of the list in current scope
    appendToEndOfListInThisScope(n);
    return noError;
}

ErrorSeam insertDecVar(DecVar *d) {
    if (findDecVarCurrentScope(d->id) != NULL) {
		return redefinitionError;
    }
    //Create node
    c_Node *n = (c_Node*) malloc(sizeof(c_Node));
    if (n == NULL) { printf("out of memory\n"); exit(1);}
    n->tag = decVar;
    n->u.decVar = d;
    n->next = NULL;
    
    //Append to end of the list in current scope
    appendToEndOfListInThisScope(n);
    return noError;
}

void* find(char *id, DecE tag) {
    if (id == NULL) { return NULL; }
    Scope *scope = lastScope(scopeStack);
    if (scope == NULL) { return NULL; }
    int i=0;
	//for each scope, from last to first
    for (;scope != NULL; scope = scope->previous) {
		//for each node in scope
    	for (c_Node *n = scope->node ; n!=NULL ; n = n->next) {
	    	switch(n->tag) {
	    	case decVar:
	        	if (strcmp(id,n->u.decVar->id) == 0 && n->tag == tag) {
                    return n->u.decVar;
                }
			break;
	    	case decFunc:
				if (strcmp(id,n->u.decFunc->id) == 0 && n->tag == tag) {
                    return n->u.decFunc;
            	}
			break;
	    	}
		}
        i++;
    }
    return NULL;
}

DecVar* findDecVar(char *id) {
    return (DecVar*) find(id,decVar);
}

DecFunc* findDecFunc(char *id) {
    return (DecFunc*) find(id,decFunc);
}

void freeAll() {
    freeScopeList(scopeStack); 
    scopeStack = NULL;
}

//Code below just for tests


void printScope() {
    Scope *s;
    c_Node *n;
    int i;
    for(s=scopeStack,i=0;s!=NULL;s=s->next,i++) {
        printf("scope %d\n\t",i);
	for (n = s->node; n!=NULL; n=n->next) {
	    if (n->tag == decVar) {
	    	printf("%s ",n->u.decVar->id);
	    } else {
		printf("%s ",n->u.decFunc->id);
	    }
	}
	printf("\n");
    }
}
/*
int main() {
    enterScope();
    int option = 1;
    DecVar *dv = NULL;
    DecVar *res = NULL;
    while(option >= 1 && option <= 6) {
	    printScope();
	    printf("Digite a opção\n");
	    printf("[1] - enterScope\n");
	    printf("[2] - leaveScope\n");
	    printf("[3] - insert\n");
	    printf("[4] - findCurrentScope\n");
	    printf("[5] - find\n");
	    printf("[6] - free\n");
	    scanf("%d",&option);
	    char id[100];
	    switch(option) {
		case 1: enterScope(); break;
		case 2: leaveScope(); break;
		case 3:
		    printf("digite o id\n");
		    scanf("%s",id);
	  	    dv = (DecVar*) malloc(sizeof(DecVar));
    		    if (dv == NULL) { exit(1); }
		    dv->id = strdup(id);
		    insertDecVar(dv); 
		    break;
		case 4: 
		    printf("id para verificar:\n");
		    scanf("%s",id);
		    res = findDecVarCurrentScope(id);
		    if (res == NULL) {
			printf("Não achou!\n");
		    } else {
			printf("Achou!\n");
		    }
		    break;
		case 5:
		    printf("id para verificar:\n");
		    scanf("%s",id);
		    res = findDecVar(id);
		    if (res == NULL) {
			printf("Não achou!\n");
		    } else {
			printf("Achou!\n");
		    }
		    break;
		case 6: freeScopeList(scopeStack); scopeStack = NULL; break;
		default: printf("tchau\n"); break;
	    }
    }
    return 0;
}
*/
