/************
* Rio,2016
* Autores:
* Eduardo Tiomno Tolmasquim - dudutt@gmail.com
* Pedro Alvarez - pedroneves62@hotmail.com
*************/

%option yylineno
%option nounput
%option noinput

%{
#include <string.h>
#include "parser.h"
#include "y.tab.h"

/*
 * func:
 *     evaluate int literal
 *     store it in yylval 
 * params:
 *     char *s: the literal in string format
 * return: 
 *		token type
 */
int handle_const_int(char *s) {
	//hexadecimal
	if(yytext[1] == 'x') {
		yylval.i = strtol(&yytext[2],NULL,16);
	} 
	//decimal
	else {
		yylval.i = strtol(yytext,NULL,10);
	} 
	return TK_CONST_INT;
}

/*
 * func:
 *     handle specials characters
 * 	   store string in yylval
 * params:
 *     char *s: the string to handle
 *	   int length: length of the string
 * return: 
 *		token type
 */
int handle_string(char *s, int length) {
	char* result = (char *) malloc(length * sizeof(char));
	if (result == NULL) { exit(1); }
	int i;
	s[length-1] = '\0'; //removing last commas
	s++;
	//i starts equals 1 to remove first commas
	for (i = 1; i < length-1 && *s != '\0'; i++) {
		if (*s == '\\') {
			switch(*(s+1)) {
				case '"': result[i-1] = '\"'; s++; break;
				case 't': result[i-1] = '\t'; s++; break;
				case 'n': result[i-1] = '\n'; s++; break;
				default:  result[i-1] = *s;        break;
			}
		} else {
			result[i-1] = *s;
		}
		s++;
	}
	result[i-1] = '\0'; 
	yylval.s = result;
	return TK_CONST_STRING;
}

/*
 * func:
 *     limit length of id's
 *	   store it in yylval
 * params:
 *     char *s: the string to handle
 *	   int length: length of the string
 * return: 
 *		token type
 */
int handle_id(char*s) {
	int length = strlen(s);
	if (length >= 300) {
		printf("Syntax error: ID can have at most 300 characters\n");
		exit(1);
	} else {
		yylval.id = strdup(s);
		return TK_ID;
	}
}

%}
%%
" "			;
[\t\n]			;
"0x"[0-9A-Fa-f]+|[0-9]+	 					{ return handle_const_int(yytext); }
[0-9]+"\."[0-9]+([eE][-+]?[0-9]+)?			{ 
												yylval.f = strtof(yytext,NULL); 
												return TK_CONST_FLOAT;
											}
\"(\\.|[^\\"])*\"							{ return handle_string(yytext,strlen(yytext)); }
\"[ˆ\"]*									{ 
												printf("Syntax error: string not closed!\n"); 
												exit(1); 
											}		
"char"										return TK_CHAR;
"int"										return TK_INT;
"float"										return TK_FLOAT;
"void"										return TK_VOID;
"while"										return TK_WHILE;
"if"										return TK_IF;
"else"										return TK_ELSE;
"new"										return TK_NEW;
"return"									return TK_RETURN;
">="										return TK_GE;
"<="										return TK_LE;
"=="										return TK_EE;
"!="										return TK_NE;
"&&"										return TK_LOGIC_AND;
"||"										return TK_LOGIC_OR;
[a-zA-Z][a-zA-Z0-9]*						{ return handle_id(yytext); }
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]			;
"//".*\n									;
.					return yytext[0];
%%

int yywrap(void)
{
	return 1;
}
