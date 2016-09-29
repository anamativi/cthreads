#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/support.h"
#include "../include/cthreadAux.h"

ucontext_t* newContext = NULL;

int GetNewThreadTid()
{
	static int globalThreadTid = -1;

	return ++globalThreadTid;
}


Thread_t* CreateNewThread(BOOLEAN initStruct)
{
	Thread_t* newThread = (Thread_t*)malloc(sizeof(Thread_t));
	if (newThread == NULL)
		return NULL;

	newThread->data.state = 1;

	int newTicket = Random2();
	
	if (initStruct == FALSE){
		newThread->data.tid = 0; // TID da Main é 0
		newThread->data.ticket = -1; // Ticket que não existe
	}
	else {
		newThread->data.tid = GetNewThreadTid();
	
		while (newTicket > 255)
		{
			newTicket = newTicket - 255;
		}
		newThread->data.ticket = newTicket;
	}
	
	newThread->data.state = PROCST_CRIACAO; // PROCST_BLOQ
	newThread->yield = FALSE;
	newThread->has_thread_waiting = FALSE;
	newThread->is_waiting = FALSE;
	newThread->waitingThread = NULL;

	return newThread;
}


//Retorna a thread que contém o tid desejado
//Caso não encontre, retorna NULL
Thread_t* SearchThreadByTid(int tid, PFILA2 fila)
{
	FirstFila2(fila);
	Thread_t* aThread = (Thread_t*)GetAtIteratorFila2(fila);

	LastFila2(fila);
	Thread_t* lastThread = (Thread_t*)GetAtIteratorFila2(fila);

	if (aThread == NULL)
		return NULL; //Fila não existe / está vazia

	if(aThread->data.tid == tid)
		return aThread; //Os tids correspondem, é a thread que estamos procurando


	while (TRUE) // Percorre a fila atrás da thread
	{
		NextFila2(fila);
		aThread = (Thread_t*)GetAtIteratorFila2(fila);

		if (aThread->data.tid == tid) // Achou a thread desejada, sai da função
			return aThread;

		if (aThread->data.tid == lastThread->data.tid) // Chegou na última thread da fila, sai do loop
			break;
	}

	return NULL; // A thread não foi encontrada, retorna NULL
}

ucontext_t* HandleContext()
{
	// Cria um contexto para a thread
	newContext = (ucontext_t*)malloc(sizeof(ucontext_t));
	char contextStack[SIGSTKSZ];

	/////////////// SE DER ERRO? (NEWCONTEXT = NULL) ///////////////

	newContext->uc_link = NULL;
	newContext->uc_stack.ss_size = sizeof(contextStack);
	newContext->uc_stack.ss_sp = contextStack;

	getcontext(newContext); // Pega o contexto atual
	makecontext(newContext, (void (*)(void)) StartNextThread, 0, NULL);

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

void StartNextThread(Thread_t *activeThread, PFILA2 filaAble)
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
	Thread_t* last_thread; // Última thread da lista

	int distance; // Distância que o ticket da thread atual está do ticket escolhido
	int best_distance = 256; // Menor distância encontrada até o momento

	// Percorre a lista de aptos atrás do ticket mais próximo ao ticket sorteado
	LastFila2(filaAble);
	last_thread = (Thread_t*)GetAtIteratorFila2(filaAble);

	if(filaAble == NULL)
	{
		printf("filaAble is NULL\n"); // A lista está vazia, não há thread para escalonar
	}

	FirstFila2(filaAble);
	while(TRUE)
	{
		current_thread = GetAtIteratorFila2(filaAble);
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
		
		if(!NextFila2(filaAble))
		{
			current_thread = GetAtIteratorFila2(filaAble);
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
	setcontext(&activeThread->data.context);
}
