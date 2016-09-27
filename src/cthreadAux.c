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


void FinishThread(Thread_t *activeThread, void *arg)
{
	if (activeThread->has_thread_waiting == TRUE)
		activeThread->waitingThread->is_waiting = FALSE; // Libera a thread que está esperando, se há alguma

	activeThread->data.state = 4;
}