/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

#ifndef typeChecker_seen
#define typeChecker_seen

#define MAX_TYPE 50
#define MAX_CALL MAX_TYPE*20

void checkTree(Program *p);
void typeString(Type *t, char *buffer);

#endif
