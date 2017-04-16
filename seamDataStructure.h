/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#ifndef seamDataStructure_seen
#define seamDataStructure_seen

#include "parser.h"

typedef enum errorSeam {
    noError, redefinitionError
}ErrorSeam;

void enterScope();
void leaveScope();
ErrorSeam insertDecVar(DecVar *d);
ErrorSeam insertDecFunc(DecFunc *d);
DecVar* findDecVar(char *id);
DecFunc* findDecFunc(char *id);
void freeAll();
void printScope();

#endif //seamDataStructure
