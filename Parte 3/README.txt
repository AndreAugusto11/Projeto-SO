Estrutura das diretorias existentes:
	SO_EX2
		CircuitRouter-ParSolver
			inputs
		CircuitRouter-SeqSolver
			inputs
		lib
		results
			inputs
		doTest.sh
		README.txt


Passos para compilar:
	Entrar na pasta CircuitRouter-SeqSolver e executar o comando "make"
	Entrar na pasta CircuitRouter-ParSolver e executar o comando "make"

Passos para executar:
	Na pasta SO_EX2 executar o comando "./doTest N FILENAME", onde N é o número de tarefas e FILENAME é o nome do ficheiro de input a ser utilizado no teste.

Máquina usada para os testes:
	Oracle VM VirtualBox
	cpu cores	: 4
	cpu MHz		: 2591.992
	model		: 94
	model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
