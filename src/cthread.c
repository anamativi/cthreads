#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/cthreadAux.h"

// --- Variáveis Globais --- //

Thread_t* activeThread; // Thread atual
static FILA2 filaAble; // Fila de aptos
static FILA2 filaBlocked; // Fila de bloqueados

// --- Funções --- //

int ccreate (void* (*start)(void*), void *arg)
{
	//Inicializa as estruturas (pela primeira vez apenas)
	static BOOLEAN initStruct = FALSE;

	if (initStruct == FALSE)
	{	
		printf("entrou (initStruct is false)\n");
		//Inicializa as três filas:

		//1. Aptos
		CreateFila2(&filaAble);
		printf("filaAble criada com sucesso!\n");
		
		//2. Bloqueados
		CreateFila2(&filaBlocked);
		printf("filaBlocked criada com sucesso!\n");
		
		//Cria a thread main (e salva seu contexto)
		Thread_t* mainThread = CreateNewThread(initStruct);

		// Cria um novo contexto
		mainThread->data.context = *HandleContext();

		mainThread->data.state = PROCST_EXEC;
		activeThread = mainThread;
		printf("criou a main!\n");

		//Marca a inicialização como pronta
		initStruct = TRUE;
	}
	Thread_t* newThread = CreateNewThread(initStruct);
	newThread->data.context = *HandleContext();
	printf("criou a thread\n");
	
	//Adiciona a nova thread na fila de aptos
	if(!AppendFila2(&filaAble, newThread))
		printf("colocou na fila de aptos (ok)\n");
	else return -1;

	return newThread->data.tid;
}

int cyield(void)
{
	printf("Entrou no cyield\n");
    // Salva um ponto de continuação para o contexto atual
	SetCheckpoint(&activeThread->data.context);
    
    // Yield
	activeThread->yield = TRUE;
    // Estado Apto
   	activeThread->data.state = 1;
    //Fila de Aptos 	
	AppendFila2(&filaAble, activeThread);


	//Thread_t* &teste = *GetAtIteratorFila2(*filaExec);

	StartNextThread(activeThread, &filaAble);
    // Isso nunca deve ocorrer
    //return -1;
	printf("Saiu no cyield\n");
	return 0;///////////////////////////////////////////////////////////
}

int cjoin(int tid)
{
	printf("entrou na cjoin!\n");
	//tid = identificador da thread cujo término está sendo aguardado
	Thread_t* targetThread;

	// Salva o contexto de execução atual
	SetCheckpoint(&activeThread->data.context);
	printf("[cjoin 91] checkpoint setado\n");

	// Procura a thread-alvo na lista de aptos
	targetThread = SearchThreadByTid(tid, &filaAble);

	if (targetThread == NULL)	// Não achou a thread na lista de aptos
		targetThread = SearchThreadByTid(tid, &filaBlocked); // Procura na lista de bloqueados	

	if (targetThread == NULL)
		return -1; // Thread não foi encontrada em nenhuma fila (não existe ou já terminou de executar)

	printf("[cjoin 95] target found:\n\t TID wanted: %d\n\t TID found.: %d\n", tid, targetThread->data.tid);

	// Testa se a thread-alvo terminou de executar 
	if (targetThread->data.state == 4)
		return 0;

	// Testa se a thread-alvo está sendo esperada por outra
	if (targetThread->has_thread_waiting == TRUE)
		return -1; // Há outra thread esperando por ela -> erro!

	// Todos os testes ok, adiciona a thread atual para a espera da thread alvo
	targetThread->has_thread_waiting = TRUE; // Avisa a thread-alvo que há alguma thread esperando por ela 
	targetThread->waitingThread = activeThread; // Aponta a thread esperando para a thread atual (em execução)
	
	activeThread->is_waiting = TRUE; // Agora estamos "oficialmente" esperando por uma thread
	activeThread->data.state = 3; // Estado = bloqueada (3)
	SetCheckpoint(&activeThread->data.context);

	AppendFila2(&filaBlocked, activeThread); // Coloca na fila de bloqueados
	printf("[cjoin 120] Colocou a thread na lista de espera e chama StartNextThread.\n");

	// Executa a próxima thread da lista de aptos!!!!!!!
	StartNextThread(activeThread, &filaAble);
	return 0;
}

int csem_init(csem_t *sem, int count)
{
	printf("Entrou no csem_init\n");
	// Inicia o semaforo
	sem->count = 1;
	sem->fila = (FILA2*)malloc(sizeof(FILA2));
	CreateFila2(sem->fila);

	printf("Saiu do csem_init\n");
    return 0;
}

int cwait(csem_t *sem)
{
	printf("Entrou no cwait\n");
	// Salva um ponto de continuação para o contexto atual
	SetCheckpoint(&activeThread->data.context);

    // Verifica se temos alguma thread ja acessando este semaforo
    if(sem -> count > 0)
    {
        //Decrementa o contador
        sem->count--;
        // Retorna sem problemas
        return 0;
    }
    
    //Decrementa o contador mesmo que ja esteja menor que zero
    sem->count--;

    //Coloca a thread no estado de bloqueado
    activeThread->data.state = 3;
    
    // Adiciona a thread atual na fila do semaforo em questao ja que este semaforo esta bloqueado
    AppendFila2(sem->fila, activeThread);



    // Incrementa o contador
    //sem->count++;

    // Coloca que a thread atual esta esperando por um semaforo
    //s_CurrentThread->waitingForSignal = TRUE;

    // Procura uma nova thread para ser executada (insere a atual novamente na lista de bloqueadas)
    //FindNextThread(s_ThreadBlockedList);
	printf("Saiu do cwait\n");
	return 0;
}

int csignal(csem_t *sem)
{
	printf("Entrou no csignal\n");
    // Incrementa o contador do semaforo
    sem->count++;

    // Verifica se existe threads esperando o recurso
    if(sem->count <= 0)
    {
		FirstFila2(sem->fila);
		Thread_t* thread = GetAtIteratorFila2(sem->fila);
		DeleteAtIteratorFila2(sem->fila);
		thread-> data.state = 1;
		AppendFila2(&filaAble, thread);
		printf("Saiu do cwait\n");
		return 0;
    }
 	
 	else 
 	{
		printf("Saiu do cwait\n");
  		return 0;
  	}
}

int cidentify (char *name, int size)
{
	//int i = 0;
	char grupo[] = "Ana Mativi\nAthos Lagemann\nRicardo Sabedra\n";

	if (size > sizeof(grupo))
		return -1;

	memcpy(name, &grupo, size); // Copia a string

	name[sizeof(grupo)+1] = '\0'; // Garante que há um \0 no final da string passada

	return 0;
}
