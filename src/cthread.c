#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/cthreadAux.h"

Thread_t* FindNextThread(int x);
void ExecuteThread();
int CreateExeContext();
void EndThread();

// --- Variáveis Globais --- //

Thread_t* activeThread = NULL; // Thread atual
Thread_t* mainThread = NULL; // Thread main
ucontext_t* contextExe = NULL;
static FILA2 filaAble; // Fila de aptos
static FILA2 filaBlocked; // Fila de bloqueados

// --- Funções --- //

int ccreate (void* (*start)(void*), void *arg)
{
	//Inicializa as estruturas (pela primeira vez apenas)
	static BOOLEAN initStruct = FALSE;
	static BOOLEAN finthread = FALSE;

	if (initStruct == FALSE)
	{	
		//Inicializa as três filas:

		//1. Aptos
		CreateFila2(&filaAble);
		
		//2. Bloqueados
		CreateFila2(&filaBlocked);
		
		//Cria a thread main (e salva seu contexto)
		mainThread = (Thread_t*) malloc(sizeof(Thread_t));

		if (mainThread == NULL)
			return -1;

		mainThread->data.state = PROCST_EXEC;
		mainThread->data.tid = GetNewThreadTid();
		mainThread->data.ticket = (Random2() % 256);

		mainThread->yield = FALSE;
		mainThread->has_thread_waiting = FALSE;
		mainThread->is_waiting = FALSE;

		getcontext(&mainThread->data.context);

		activeThread = mainThread;

		//Marca a inicialização como pronta
		initStruct = TRUE;
		// Cria um novo contexto
		CreateExeContext();
	}

	ucontext_t* disableThread = (ucontext_t*) malloc(sizeof(ucontext_t));
		if(disableThread == NULL)
			return -1;

	if (finthread == FALSE)
	{	

		disableThread->uc_link = NULL;
		disableThread->uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
		if(disableThread->uc_stack.ss_sp == NULL)
			return -1;
		
		disableThread->uc_stack.ss_size = SIGSTKSZ;

		getcontext(disableThread);
		makecontext(disableThread, (void(*)(void)) EndThread, 0, NULL);
		finthread = TRUE;
	}

	Thread_t* newThread = malloc(sizeof(Thread_t));
	if (newThread == NULL)
		return -1;

	newThread->data.tid = GetNewThreadTid();
	newThread->data.state = PROCST_APTO;
	newThread->data.ticket = (Random2() % 256);
	newThread->yield = FALSE;
	newThread->has_thread_waiting = FALSE;
	newThread->is_waiting = FALSE;

	getcontext(&newThread->data.context);

	newThread->data.context.uc_link = NULL;
	newThread->data.context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	
	if(newThread->data.context.uc_stack.ss_sp == NULL)
		return -1;
	
	newThread->data.context.uc_stack.ss_size = SIGSTKSZ;

	makecontext(&newThread->data.context, (void(*))start, 1, arg);

	//Adiciona a nova thread na fila de aptos
	if(AppendFila2(&filaAble, newThread))
		return -1;

	return newThread->data.tid;
}

int cyield(void)
{
	// Salva um ponto de continuação para o contexto atual
	Thread_t* aux;
	aux = activeThread;
	
	aux->data.state = PROCST_APTO;

	// Yield
	aux->yield = TRUE;
	
	//Fila de Aptos 	
	AppendFila2(&filaAble, aux);

	swapcontext(&activeThread->data.context, contextExe);

	return 0;
}

int cjoin(int tid)
{
	//tid = identificador da thread cujo término está sendo aguardado
	Thread_t* targetThread;

	// Procura a thread-alvo na lista de aptos
	targetThread = SearchThreadByTid(tid, &filaAble);

	if (targetThread == NULL)	// Não achou a thread na lista de aptos
		targetThread = SearchThreadByTid(tid, &filaBlocked); // Procura na lista de bloqueados	

	if (targetThread == NULL)
	{
		return -1; // Thread não foi encontrada em nenhuma fila (não existe ou já terminou de executar)
	}

	// Testa se a thread-alvo terminou de executar 
	if (targetThread->data.state == 4)
	{
		return 0;
	}

	// Testa se a thread-alvo está sendo esperada por outra
	if (targetThread->has_thread_waiting == TRUE)
	{
		return -1; // Há outra thread esperando por ela -> erro!
	}

	// Todos os testes ok, adiciona a thread atual para a espera da thread alvo
	targetThread->has_thread_waiting = TRUE; // Avisa a thread-alvo que há alguma thread esperando por ela 
	targetThread->waitingThread = activeThread; // Aponta a thread esperando para a thread atual (em execução)
	
	activeThread->is_waiting = TRUE; // Agora estamos "oficialmente" esperando por uma thread
	activeThread->data.state = PROCST_BLOQ; // Estado = bloqueada (3)

	AppendFila2(&filaBlocked, activeThread); // Coloca na fila de bloqueados

	// Executa a próxima thread da lista de aptos

	swapcontext(&activeThread->data.context, contextExe);

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

	AppendFila2(filaBlocked, activeThread);
	
	swapcontext(&activeThread->data.context, contextExe);

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
		AppendFila2(&filaAble, thread);
		return 0;
	}
	
	else 
	{
		return 0;
	}
}

int cidentify (char *name, int size)
{
	char grupo[] = "Ana Mativi\nAthos Lagemann\nRicardo Sabedra\n";
	int tamanho = sizeof(grupo);

	if (size <= 0){
		return -1;
	}

	memcpy(name, &grupo, tamanho); // Copia a string

	return 0;
}

// Funcoes auxiliares

Thread_t* FindNextThread(int x)
{
	int best_dist = 256;
	Thread_t* aux = NULL;
	Thread_t* nextThread = NULL;
	
	FirstFila2(&filaAble);
	
	aux = GetAtIteratorFila2(&filaAble);
	while (aux != NULL)
	{
		if (abs(aux->data.ticket - x) < best_dist){
			best_dist = abs(aux->data.ticket - x);
			nextThread = aux;
		}
		NextFila2(&filaAble);
		aux = GetAtIteratorFila2(&filaAble);
	}
	
	DeleteFromFila(nextThread->data.tid, &filaAble);

	return nextThread;
}

void ExecuteThread()
{
	int randomTicket = Random2(); // Gera um ticket
	
	while(randomTicket > 255)
		randomTicket = randomTicket - 255; // Aplica a mascara

	Thread_t* nextThread = FindNextThread(randomTicket); // Procura pela thread com ticket mais proximo
	if(nextThread == NULL)
		return;
	
	activeThread = nextThread; // Passa pra proxima
	nextThread->data.state = PROCST_EXEC; // Seta o estado de execucao
	
	setcontext(&nextThread->data.context); // Ativa o contexto
}

int CreateExeContext()
{
	contextExe = (ucontext_t*) malloc(sizeof(ucontext_t));
	if (contextExe == NULL) 
		return -1;
	
	contextExe->uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	contextExe->uc_stack.ss_size = SIGSTKSZ;
	contextExe->uc_link = NULL;
	
	getcontext(contextExe);
	makecontext(contextExe, (void (*)(void)) ExecuteThread, 0, NULL);
	return 0;
}

void EndThread()
{
	Thread_t* waitingThread = activeThread->waitingThread;
	
	if (waitingThread != NULL)
	{
		DeleteFromFila(waitingThread->data.tid, &filaBlocked);

		waitingThread->data.state = PROCST_APTO;
		
		if(waitingThread->data.tid == 0)
			AppendFila2(&filaAble, mainThread); // Caso seja a main
		else
			AppendFila2(&filaAble, waitingThread); // Caso contrario
	}

	activeThread = NULL;
	ExecuteThread();   // Chama o ExecuteThread novamente
}
