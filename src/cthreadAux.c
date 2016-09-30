#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/support.h"
#include "../include/cthreadAux.h"

//ucontext_t* newContext = NULL;

int GetNewThreadTid()
{
	static int globalThreadTid = -1;

	return ++globalThreadTid;
}


Thread_t* CreateNewThread(BOOLEAN initStruct)
{
	Thread_t* newThread = (Thread_t*)malloc(sizeof(Thread_t));
	if (newThread == NULL)
	{
		return (Thread_t *)-1;
	}

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

	if (aThread == NULL)
		return NULL; //Fila não existe / está vazia

	if(aThread->data.tid == tid)
		return aThread; //Os tids correspondem, é a thread que estamos procurando


	while (aThread != NULL) // Percorre a fila atrás da thread
	{

		if (aThread->data.tid == tid) // Achou a thread desejada, sai da função
			return aThread;

		if(NextFila2(fila))
			break;

		aThread = (Thread_t*)GetAtIteratorFila2(fila);
	}

	return NULL; // A thread não foi encontrada, retorna NULL
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

int DeleteFromFila(int tid, PFILA2 fila)
{
	Thread_t* targetThread = NULL;

	FirstFila2(fila);
	targetThread = GetAtIteratorFila2(fila);

	while(targetThread != NULL)
	{
		if(targetThread->data.tid == tid)
		{
			DeleteAtIteratorFila2(fila);
			return 0;
		}
		else
		{
			NextFila2(fila);
			targetThread = GetAtIteratorFila2(fila);
		}

	}
	return -1;
}
