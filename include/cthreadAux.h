#ifndef cthreadAux
#define cthreadAux

#include "cdata.h"
#include "support.h"

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


#endif
