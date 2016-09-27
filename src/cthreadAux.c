#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/support.h"
#include "../include/cthreadAux.h"

int GetNewThreadTid()
{
	static int globalThreadTid = -1;

	return ++globalThreadTid;
}


Thread_t* CreateNewThread(BOOLEAN initStruct)
{
	Thread_t* newThread = (Thread_t*)malloc(sizeof(Thread_t));
	newThread->data.tid = GetNewThreadTid();
	newThread->data.state = 1;
	
	if (initStruct == FALSE){
		newThread->data.ticket = -1;
	}
	else {
		int x = Random2();
		while (x > 255){
		x = x - 255;
		} 
		newThread->data.ticket = x;
	}

	/*	
	newThread->threadData.state = PROCST_CRIACAO; // PROCST_BLOQ
	
	newThread->yieldVictim = FALSE;
	newThread->waitingThread = NULL;
	newThread->waitingForThread = FALSE;
	newThread->finishedExecution = FALSE;
	newThread->waitingForSignal = FALSE;
	*/
	return newThread;
}


//Retorna a thread que contém o tid desejado
//Caso não encontre, retorna NULL
Thread_t* SearchThreadByTid(int tid, PFILA2 fila)
{
	Thread_t* aThread = FirstFila2(fila);
	Thread_t* lastThread = LastFila2(fila);

	if (aThread == NULL)
		return NULL; //Fila não existe / está vazia

	if(aThread->data.tid == tid)
		return aThread; //Os tids correspondem, é a thread que estamos procurando


	while (TRUE) // Percorre a fila atrás da thread
	{
		aThread = NextFila2(fila);

		if (aThread->data.tid == tid) // Achou a thread desejada, sai da função
			return aThread;

		if (aThread->data.tid == lastThread->data.tid) // Chegou na última thread da fila, sai do loop
			break;
	}

	return NULL; // A thread não foi encontrada, retorna NULL
}

ucontext* CreateContext(Function(func), void *arg, Function(endFunc))
{
	// Cria um contexto para a thread
	ucontext newContext;
	char contextStack[SIGSTKSZ];

	typedef void(*PiFunc)(); // Para evitar warnings

	getcontext(&newContext); // Pega o contexto atual

	newContext.uc_stack.ss_size = sizeof(contextStack);
	newContext.uc_stack.ss_flags = 0;
	newContext.uc_stack.ss_sp = contextStack;

	// Seta os dados do novo contexto
	if (func == endFunc)
		newContext.uc_link = NULL;
	else
	{
		ucontext_t auxContext = CreateNewContext(endFunc, NULL, NULL);
        ucontext_t* aux = (ucontext_t*)malloc(sizeof(ucontext_t));
        memcpy(aux, &auxContext, sizeof(ucontext_t));
        newContext.uc_link = aux;
	}

	makecontext(&newContext, (PiFunc)func, 1, arg);

	return newContext;
}

void SetCheckpoint(ucontext_t* context)
{
	// Salva o contexto atual
	getcontext(context);
}


void FinishThread(Thread_t *activeThread, void *arg)
{
	if (activeThread->has_thread_waiting == TRUE)
		activeThread->waitingThread->is_waiting = FALSE; // Libera a thread que está esperando, se há alguma

	activeThread->data.state = 4;
}

int StartNextThread(PFILA2* filaAble)
{
	// Gera o ticket que será sorteado
	int jackpot = Random2();

	while (jackpot > 255)
	{
		jackpot = jackpot - 255; // Garante que jackpot <= 255
	}

	Thread_t* chosen_one; // Thread escolhida
	Thread_t* current_thread; // Thread que estamos olhando no momento
	Thread_t* last_thread; // Última thread da lista

	int distance; // Distância que o ticket da thread atual está do ticket escolhido
	int best_distance = 256; // Menor distância encontrada até o momento (inicia em um valor impossível, 256, de modo que é trocada logo na primeira iteração)

	// Percorre a lista de aptos atrás do ticket mais próximo ao ticket sorteado
	current_thread = FirstFila2(filaAble);
	last_thread = LastFila2(filaAble);

	if(current_thread == NULL)
	{
		return -1; // A lista está vazia, não há thread para escalonar [???????????????]
	}

	do
	{
		if(abs(jackpot - current_thread->data.ticket) < best_distance) // Compara a distância do ticket atual com a menor distância
		{
			chosen_one = current_thread;
			best_distance = abs(jackpot - current_thread->data.ticket);
		}

		if(abs(jackpot - current_thread->data.ticket) == best_distance) // tickets estão à mesma distância
		{
			if(current_thread->data.tid < chosen_one->data.tid) // Thread de menor tid ganha a cpu
				chosen_one = current_thread;
			// Caso a thread escolhida tenha o menor tid, não há necessidade de fazer nada
		}

		current_thread = NextFila2(filaAble);

	} while(current_thread->data.tid != last_thread->data.tid) // SERÁ QUE ELE ANALISA A ÚLTIMA THREAD????

	// Manda a thread escolhida para a execução
	activeThread = chosen_one;

	return 0;
}