Este é um trabalho da disciplina Compiladores (INF1715) da PUC-Rio, em 2016.2, lecionada pelo professor Roberto Ierusalimschy.

O trabalho consiste na elaboração de um compilador para a linguagem Monga, especificada em:
http://www.inf.puc-rio.br/%7Eroberto/comp/lang.html

Autores:
Eduardo Tolmasquim
Pedro Alvarez

Execução:

Gerar o compilador
    - make

Rodar os testes
Atenção: os testes sobrescrevem os arquivos AST.txt e LLVM_CODE.ll, se existirem
    - make tests

Apagar os arquivos .o e o executável
Atenção: este comando apaga os arquivos AST.txt e LLVM_CODE.ll
	 caso você não queira apagar estes arquivos, renomeie-os antes de executar este comando.
    - make clean

Nome do executável
    - monga

Parâmetros:
    Gerar o arquivo AST.txt com a árvore abstrata: 
    -ast 

    Gerar o arquivo LLVM_CODE.ll com o código llvm:
    -llvm

    Exemplo 1, gerar a árvore em AST.txt:
	./monga -ast < prog.monga 

    Exemplo 2, gerar llvm no arquivo LLVM_CODE.ll:
	./monga -llvm < prog.monga

Sobre o código:
	lex.l
		O léxico da linguagem. Usa o Lex (http://dinosaur.compilertools.net/lex) para reconhecer os Tokens, Identificadores ou literais.

	parser.y
		Usa o Yacc (http://dinosaur.compilertools.net/yacc) para definir a gramática da linguagem.

	parser.c
		Tem as funções, usadas pelo parser.y, para construir a árvore abstrata.

	seamDataStructure.c
		Estrutura de dados usada para fazer a costura dos identificadores com as suas declarações. É uma lista duplamente encadeada de escopos, onde cada escopo tem um lista de declarações de funções e variáveis.

	sewTree.c
		Faz a costura dos identificadores com as suas declarações.

	typeChecker.c
		Verifica se os tipos estão corretos e faz conversão de tipos quando necessário.

	printTree.c
		Imprime a árvore abstrata

	generateLLVM.c
		Gera código LLVM (http://llvm.org).
