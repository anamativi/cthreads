#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/cthreadAux.h"

Thread_t* acharProximaThread(int x);
void dispatcher();
ucontext_t* HandleContext();
void StartNextThread();
int criarContextoDispatcher();
void Imprimelista();
void terminarThread();

// --- Variáveis Globais --- //

Thread_t* activeThread = NULL; // Thread atual
Thread_t* mainThread = NULL; // Thread main
ucontext_t* contextoDispatcher = NULL;
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
		//mainThread = CreateNewThread(initStruct);
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

		// Cria um novo contexto
		//mainThread->data.context = *HandleContext();

		//mainThread->data.state = PROCST_EXEC;
		activeThread = mainThread;

		//Marca a inicialização como pronta
		initStruct = TRUE;
		criarContextoDispatcher();
	}
	
	
	//Thread_t* newThread = CreateNewThread(initStruct);
	//newThread->data.context = *HandleContext();

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
		makecontext(disableThread, (void(*)(void)) terminarThread, 0, NULL);
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

	newThread->data.context.uc_link = disableThread;
	//newThread->data.context.uc_link = NULL;
	newThread->data.context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	if(newThread->data.context.uc_stack.ss_sp == NULL)
		return -1;
	newThread->data.context.uc_stack.ss_size = SIGSTKSZ;

	makecontext(&newThread->data.context, (void(*))start, 1, arg);

	//Adiciona a nova thread na fila de aptos
	if(AppendFila2(&filaAble, newThread))
		return -1;

	printf("FilaAble:\n");
	Imprimelista(&filaAble);

	return newThread->data.tid;
}

int cyield(void)
{
    // Salva um ponto de continuação para o contexto atual
	Thread_t* aux;
	aux = activeThread;
	//printf("Thread %d entrou no cyield\n", aux->data.tid);
    
    aux->data.state = PROCST_APTO;

    // Yield
	aux->yield = TRUE;
    
    //Fila de Aptos 	
	AppendFila2(&filaAble, aux);

	swapcontext(&activeThread->data.context, contextoDispatcher);
	return 0;///////////////////////////////////////////////////////////
}

int cjoin(int tid)
{
	//tid = identificador da thread cujo término está sendo aguardado
	Thread_t* targetThread;

	printf("filaBlocked:\n");
	Imprimelista(&filaBlocked);

	// Salva o contexto de execução atual
	SetCheckpoint(&activeThread->data.context);

	// Procura a thread-alvo na lista de aptos
	targetThread = SearchThreadByTid(tid, &filaAble);

	if (targetThread == NULL)	// Não achou a thread na lista de aptos
		targetThread = SearchThreadByTid(tid, &filaBlocked); // Procura na lista de bloqueados	

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

	AppendFila2(&filaBlocked, activeThread); // Coloca na fila de bloqueados

	// Executa a próxima thread da lista de aptos!!!!!!!

	printf("1filaBlocked:\n");
	Imprimelista(&filaBlocked);
	swapcontext(&activeThread->data.context, contextoDispatcher);

	printf("2filaBlocked:\n");
	Imprimelista(&filaBlocked);
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
	char grupo[] = "Ana Mativi\nAthos Lagemann\nRicardo Sabedra\n";
	int tamanho = sizeof(grupo);

	if (size <= 0){
		return -1;
	}

	memcpy(name, &grupo, tamanho); // Copia a string

	return 0;
}

/////////////////////////////////////////////////////////////////////

ucontext_t* HandleContext()
{
	// Cria um contexto para a thread
	ucontext_t* newContext = (ucontext_t*)malloc(sizeof(ucontext_t));
	char contextStack[SIGSTKSZ];

	if (newContext == NULL)
	{
		printf("newContext = NULL - malloc error\n");
		return (void *)-1;
	}

	newContext->uc_link = NULL;
	newContext->uc_stack.ss_size = sizeof(contextStack);
	newContext->uc_stack.ss_sp = contextStack;

	getcontext(newContext); // Pega o contexto atual
	makecontext(newContext, (void (*)(void)) StartNextThread, 0, NULL);

	return newContext;
}


void StartNextThread()
{
	// Gera o ticket que será sorteado
	int jackpot = Random2();
	
	while (jackpot > 255)
	{
		jackpot = jackpot - 255; // Garante que jackpot <= 255
	}
	printf("[StartNextThread 122] Jackpot gerado (%d)\n", jackpot);

	Thread_t* chosen_one; // Thread escolhida
	Thread_t* current_thread; // Thread que estamos olhando no momento
	//Thread_t* last_thread; // Última thread da lista

	int distance; // Distância que o ticket da thread atual está do ticket escolhido
	int best_distance = 256; // Menor distância encontrada até o momento

	// Percorre a lista de aptos atrás do ticket mais próximo ao ticket sorteado
	//LastFila2(filaAble);
	//last_thread = (Thread_t*)GetAtIteratorFila2(filaAble);

	if(FirstFila2(&filaAble))
	{
		printf("filaAble is NULL\n"); // A lista está vazia, não há thread para escalonar
		return;
	}

	
	while(TRUE)
	{
		current_thread = GetAtIteratorFila2(&filaAble);
		printf("inicio do while\n");

		distance = abs(jackpot - current_thread->data.ticket); // Calcula a distância do ticket
		printf("\t dist: %d \t best: %d \t TID: %d\n", distance, best_distance, current_thread->data.tid);

		if(distance == best_distance) // tickets estão à mesma distância
		{
			if(current_thread->data.tid < chosen_one->data.tid) // Thread de menor tid ganha a cpu
				chosen_one = current_thread;
			printf("conflito, pega a thread de menor tid (%d)\n", chosen_one->data.tid);
			// Caso a thread escolhida tenha o menor tid, não há necessidade de fazer nada
		}		

		if(distance < best_distance) // Compara a distância do ticket atual com a menor distância
		{
			chosen_one = current_thread;
			best_distance = distance;
			printf("troca dists (best = %d)\n", best_distance);
		}

		if(current_thread == NULL)
		{
			printf("current_thread is NULL\n");
		}

		printf("antes de passar pro próximo\n");
		
		if(!NextFila2(&filaAble))
		{
			current_thread = GetAtIteratorFila2(&filaAble);
			if (current_thread == NULL)
				break;
		}
	}
	printf("[StartNextThread 173] Saiu do while\n");

	// Manda a thread escolhida para a execução
	if (chosen_one == NULL)
	{
		printf("chosen is NULL\n");
		return;
	}
	printf("chosen TID = %d\n", chosen_one->data.tid);
	activeThread = chosen_one;
	activeThread->data.state = PROCST_EXEC;
	printf("[StartNextThread 178] Seta o contexto\n");
}

Thread_t* acharProximaThread(int x)
{
    int menor_diferenca = 256;
    Thread_t* aux = NULL;
    Thread_t* proximaThread = NULL;
    
    FirstFila2(&filaAble);
    
    aux = GetAtIteratorFila2(&filaAble);
    while (aux != NULL)
    {
        if (abs(aux->data.ticket - x) < menor_diferenca){
            menor_diferenca = abs(aux->data.ticket - x);
            proximaThread = aux;
        }
        NextFila2(&filaAble);
        aux = GetAtIteratorFila2(&filaAble); // DeleteAtIteratorFila2???
    }
    
    DeleteFromFila(proximaThread->data.tid, &filaAble);

    return proximaThread;
}

void dispatcher()
{
    int aleatorio = (Random2() % 256);
    Thread_t *proximaThread = NULL;
    proximaThread = acharProximaThread(aleatorio);
    if(proximaThread == NULL)
        return;
    
	//printf("Thread escolhida: %d, ticket: %d e o numero avaliado: %d \n", proximaThread->tid, proximaThread->ticket, aleatorio);
    
    activeThread = proximaThread;
    proximaThread->data.state = PROCST_EXEC;
    
    setcontext(&proximaThread->data.context);
}

int criarContextoDispatcher()
{
    contextoDispatcher = (ucontext_t*) malloc(sizeof(ucontext_t));
    if (contextoDispatcher == NULL) return -1; //erro no malloc
    
    contextoDispatcher->uc_link = NULL;
    contextoDispatcher->uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
    contextoDispatcher->uc_stack.ss_size = SIGSTKSZ;
    
    getcontext(contextoDispatcher);
    makecontext(contextoDispatcher, (void (*)(void)) dispatcher, 0, NULL);
    return 0;
}

void terminarThread(){
    
    //quemEspera* quemEspera = alguemEsperando(exeThread);
	Thread_t* waitingThread = activeThread->waitingThread;
    
    if (waitingThread != NULL)
    {
        //removerBloqueada(quemEspera->esperando);
     	DeleteFromFila(waitingThread->data.tid, &filaBlocked);
		waitingThread->data.state = PROCST_APTO;
		if(waitingThread->data.tid == 0)
			AppendFila2(&filaAble, mainThread);
		else
			AppendFila2(&filaAble, waitingThread);
    }

	activeThread = NULL;
	//dispatcher();   // Chama o dispatcher novamente
}

void Imprimelista(PFILA2 fila)
{
	FirstFila2(fila);
	Thread_t* aux = GetAtIteratorFila2(fila); 

	while(aux != NULL)
	{
		printf("\t TID: %d \n", aux->data.tid);
		NextFila2(fila);
		aux = GetAtIteratorFila2(fila);
	}

}

