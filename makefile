CFLAGS = -Wall -std=c99
all : y.tab.o lex.yy.o printTree.o seamDataStructure.o parser.o sewTree.o typeChecker.o generateLLVM.o
	gcc ${CFLAGS} -o monga parser.o y.tab.o lex.yy.o printTree.o seamDataStructure.o sewTree.o typeChecker.o generateLLVM.o

y.tab.h : 
	lex lex.l
	yacc -d parser.y

y.tab.o : y.tab.h parser.h printTree.h sewTree.h typeChecker.h generateLLVM.h
	gcc ${CFLAGS} -c y.tab.c

lex.yy.o : parser.h y.tab.h
	gcc ${CFLAGS} -c lex.yy.c

printTree.o : parser.h printTree.h
	gcc ${CFLAGS} -c printTree.c

seamDataStructure.o : seamDataStructure.h parser.h
	gcc ${CFLAGS} -c seamDataStructure.c

parser.o : parser.h
	gcc ${CFLAGS} -c parser.c 

sewTree.o : parser.h sewTree.h
	gcc ${CFLAGS} -c sewTree.c

typeChecker.o : parser.h typeChecker.h
	gcc ${CFLAGS} -c typeChecker.c

generateLLVM.o : parser.h generateLLVM.h typeChecker.h
	gcc ${CFLAGS} -c generateLLVM.c
	
tests:	
	#Atribuição incorreta
	./monga < Testes/teste2.monga || true
	#
	#Retorno com tipo incorreto
	./monga < Testes/teste3.monga || true
	#
	#Atribuição incorreta para tipo array
	./monga < Testes/teste4.monga || true
	#
	#Atribuição com tipo incorreto e typecasting
	./monga < Testes/teste5.monga || true
	#
	#Parâmetro com tipo incorreto
	./monga < Testes/teste6.monga || true
	#
	#Atribuição incorreta para var indexada
	./monga < Testes/teste7.monga || true
	#
	#Argumetos a mais
	./monga < Testes/teste8.monga || true
	#
	#Argumentos a menos
	./monga < Testes/teste9.monga || true
	#
	#Função sem retorno
	./monga < Testes/teste10.monga || true
	#
	#Retorno do tipo errado
	./monga < Testes/teste11.monga || true
	#
	#Variável indefinida
	./monga < Testes/teste12.monga || true
	#
	#Função indefinida
	./monga < Testes/teste13.monga || true
	#
	#Redefinição de variável
	./monga < Testes/teste14.monga || true
	#
	#Redefinição de função
	./monga < Testes/teste15.monga || true
	#
	#Índice não inteiro
	./monga < Testes/teste16.monga || true
	#
	#Expressão incorreta em comparação
	./monga < Testes/teste17.monga || true
	#
	#Condicional não inteira
	./monga < Testes/teste18.monga || true
	#
	#Operação aritmética inválida
	./monga < Testes/teste19.monga || true
	#
	#Módulo inválido
	./monga < Testes/teste20.monga || true
	#
	#Var indexed sem ser do tipo array
	./monga < Testes/teste21.monga || true
	#
	#Retorno sem expressão
	./monga < Testes/teste22.monga || true
	#
	#Teste para compilar sem erros
	./monga -ast -llvm < Testes/teste1.monga
	#
	#Teste para executar sem erros
	./monga -llvm < Testes/teste0.monga
	clang -o prog LLVM_CODE.ll
	./prog
	
clean:
	rm sewTree.o parser.o seamDataStructure.o printTree.o lex.yy.o y.tab.o typeChecker.o generateLLVM.o monga y.tab.c y.tab.h lex.yy.c LLVM_CODE.ll AST.txt prog
