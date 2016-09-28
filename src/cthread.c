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
PFILA2 *filaAble; // Fila de aptos
PFILA2 *filaBlocked;

// --- Funções --- //

int ccreate (void* (*start)(void*), void *arg)
{
	//Inicializa as estruturas (pela primeira vez apenas)
	static BOOLEAN initStruct = FALSE;
	Thread_t* newThread = CreateNewThread(initStruct);

	if (initStruct == FALSE)
	{	
		
		//Inicializa as três filas:

		//1. Aptos
		filaAble = malloc(sizeof(PFILA2));
		CreateFila2(*filaAble);
		
		//3. Bloqueados
		filaBlocked = malloc(sizeof(PFILA2));
		CreateFila2(*filaBlocked);
		
		//Cria a thread main (e salva seu contexto)
		//Inicialização do TCB da thread main tid = 0
		//Thread_t* newThread = CreateNewThread(initStruct);

		//Seta a nova thread para a thread atual
		activeThread = newThread;

		// Cria um novo contexto
		newThread->data.context = CreateContext(start, (void (*)(void))arg, 0);

		//Marca a inicialização como pronta
		initStruct = TRUE;
	}
	newThread->data.context = CreateContext(start, (void (*)(void))arg, 0);
	
	
	//Cria contexto 
	
	//Adiciona a nova thread na fila de aptos
	AppendFila2(*filaAble, newThread);

	return newThread->data.tid;
}

int cyield(void)
{
    // Salva um ponto de continuação para o contexto atual
	SetCheckpoint(&activeThread->data.context);
    
    // Yield
	activeThread->yield = TRUE;
    // Estado Apto
   	activeThread->data.state = 1;
    //Fila de Aptos 	
	AppendFila2(*filaAble, activeThread);


	//Thread_t* &teste = *GetAtIteratorFila2(*filaExec);


    // Isso nunca deve ocorrer
    //return -1;
	//return 0;
}

int cjoin(int tid)
{
	//tid = identificador da thread cujo término está sendo aguardado
	Thread_t* targetThread;

	// Salva o contexto de execução atual
	SetCheckpoint(&activeThread->data.context);

	// Procura a thread-alvo na lista de aptos
	targetThread = SearchThreadByTid(tid, *filaAble);

	if (targetThread == NULL)	// Não achou a thread na lista de aptos
		targetThread = SearchThreadByTid(tid, *filaBlocked); // Procura na lista de bloqueados	

	if (targetThread == NULL)
		return -1; // Thread não foi encontrada em nenhuma fila (não existe ou já terminou de executar)


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

    AppendFila2(*filaBlocked, activeThread); // Coloca na fila de bloqueados

    // Executa a próxima thread da lista de aptos!!!!!!!
    //activeThread -> NovaThreadQueSeráExecutada
    return 0;
}

int csem_init(csem_t *sem, int count)
{
	// Inicia o semaforo
	sem->count = 1;
	sem->fila = (FILA2*)malloc(sizeof(FILA2));
	CreateFila2(sem->fila);

    return 0;
}

int cwait(csem_t *sem)
{
	
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

    return 0;
}

int csignal(csem_t *sem)
{
    // Incrementa o contador do semaforo
    sem->count++;

    // Verifica se existe threads esperando o recurso
    if(sem->count <= 0)
    {
		FirstFila2(sem->fila);
		Thread_t* thread = GetAtIteratorFila2(sem->fila);
		DeleteAtIteratorFila2(sem->fila);
		thread-> data.state = 1;
		AppendFila2(*filaAble, thread);
		return 0;
    }
 	
 	else 
 	{
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
