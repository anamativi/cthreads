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