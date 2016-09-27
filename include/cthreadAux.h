#ifndef cthreadAux
#define cthreadAux

#include "cdata.h"
#include "support.h"

int GetNewThreadTid();

Thread_t* CreateNewThread(BOOLEAN initStruct);

Thread_t* SearchThreadByTid(int tid, PFILA2 fila);

void FinishThread(Thread_t *activeThread, void *arg);

#endif