// Exemplo de c�culo de PI serial e paralelo
// Mostra o tempo decorrido em cada modalidade de computa��o
// Editor do c�digo
// IDE codeblocks https://www.codeblocks.org/downloads/binaries/
// Compilador C/C++
// msys2 https://github.com/msys2/msys2-installer/releases
// mpi da Microsoft
// https://www.microsoft.com/en-us/download/details.aspx?id=57467
// Instale os dois arquivos
// Primeiro o msmpisetup.exe
// Depois o msmpisdk.msi
// No codeblocks, em Settings, Compiler, Linker Settings,
// Adicione C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64\msmpi.lib
// No codeblocks, em Settings, Compiler, Search directories,
// Adicione C:\Program Files (x86)\Microsoft SDKs\MPI\Include
// no Windows � necess�rio configurar o caminho das vari�veis de ambiente
// V� em Painel de Controle, Sistema e Seguran�a, Sistema
// Na esquerda da tela, clique em Configura��es avan�adas do sistema
// Na tela que abrir clique em Vari�veis de ambiente
// Na pr�xima tela clique na parte de cima na esquerda em path no meio em
// Editar, na direita em Novo e cole isto C:\msys64\ucrt64\bin

#include <stdio.h> // bibliotecas padr�o C
#include <mpi.h> // msmpi (mpi da Microsoft)

int main() // se n�o usar argumentos na pr�xima linha defina NULL
{
	// Inicia o controle  das tarefas pelo SO
	MPI_Init(NULL,NULL);
	// Para sair do la�o while e encerrar o programa
	// int � um n�mero inteiro
	int terminou = 0;
	// N�mero do processo dentro do SO para este programa
	int idAtual;
	// Vai guardar o tamanho do processo na mem�ria
	int numprocs;
	// Usada nos la�os for
	int i;
	// flag, bandeira, sem�foro, aviso ao SO que o processo est� rodando
	int p;
	// double � um n�mero de 64 bits de precis�o dupla
	double piSomatorio, pi_serial = 0, pi_paralelo =  0, h, somatorio, x;
	// Para calcular o tempo decorrido dos c�lculos
	double partida = 0.0, chegada;
	// Usado no c�lculo de uma parcela
	double denominador = 1;

	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&idAtual);

	// Calculando PI serialmente, se for par soma, se for �mpar subtrai
	// Quanto maior o n�mero de itera��es maior a precis�o
	// Mas vai demorar muito mais, e pode parecer que est� travado
	// Pressione Ctrl + Shift + Esc e veja o consomatorio de tempo do processo

	// Marca o cron�metro
	partida = MPI_Wtime();
	// Abre o la�o e come�a o somat�rio
	for (i = 1;  i <= 1000000000; i++)  {
		if (i % 2 != 0) { // se o resto da divis�o por 2 for zero � par
			pi_serial = pi_serial + (4 / denominador);
		}
		else {
			pi_serial = pi_serial - (4 / denominador);
		}
		// Em um Celeron dual core 1.61GHz
		//denominador = denominador + 2; // com esse leva 6.354251s
		denominador += 2; // com esse leva 6.460633s devia ser mais r�pido, vai entender
	}

	printf("PI calculado serialmente por um processo: \n%.16f\n", pi_serial);
	chegada = MPI_Wtime();
	printf("Demorou %f segundos\n", chegada - partida);

	// Calculando dividindo as etapas pelos "cores" dispon�veis
	p = 0;
	while (!terminou) { // Enquanto n�o terminar continue somando as parcelas
		if (idAtual == 0) {
			if (p == 0) {
				p = 1024 * numprocs;
			}
			else {
				p = 0;
			}
			partida = MPI_Wtime();
		}

		// Avisa aos outros processos ativos as caracter�sticas deste atual
		MPI_Bcast(&p, 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (p == 0) {
			terminou = 1;
		}
		else {
			// Calcula uma parcela
			// double entre parenteses for�a o compilador a "encaixar" p em h
			// sem perda de precis�o
			h   = 1.0 / (double) p;
			somatorio = 0.0;
			for (i = idAtual + 1; i <= p; i += numprocs) {
				x = h * ((double)(i - 0.5));
				somatorio += (double)((4.0 / (1.0 + x * x)));
			}

			piSomatorio = h * somatorio;

			// Junta todos os c�lculos at� o momento
			MPI_Reduce(&piSomatorio, &pi_paralelo, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

			if (idAtual == 0) {
				printf("PI calculado em paralelo por n processos:\n%.16f\n", pi_paralelo);
				chegada = MPI_Wtime();
				printf("Demorou %f segundos\n", chegada - partida);
			}
		}
	}

	MPI_Finalize();

	return 0;
}
