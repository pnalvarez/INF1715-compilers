/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#ifndef printTree_seen
#define printTree_seen

void printSpaces(int level);
void printNum(Num *num, int level);
void printType(Type *type, int level);
void printId(char *id, int level);
void printVar(Var *v, int level);
void printCall(Call *c, int level);
void printExpBin(Exp *exp, char *label, int level);
void printExp(Exp *exp, int level);
void printDecVar(DecVar *decVar, int level);
void printReturn(ReturnNode *r, int level);
void printCommand(Command *command, int level);
void printBlock(Block *block, int level);
void printDecFunc(DecFunc *decFunc, int level);
void printDeclaration(Declaration *dec, int level);
void printProgram(Program *p);

#endif //printTree_seen
