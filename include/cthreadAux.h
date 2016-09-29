#ifndef cthreadAux
#define cthreadAux

#include "cdata.h"
#include "support.h"
#include <ucontext.h>
#include <string.h>


/*-------------------------------------------------------------------
Função:	Pega o próximo tid
Ret:	O novo tid
-------------------------------------------------------------------*/
int GetNewThreadTid();

/*-------------------------------------------------------------------
Função:	Cria uma nova thread
Ret:	A thread que foi criada
-------------------------------------------------------------------*/
Thread_t* CreateNewThread(BOOLEAN initStruct);

/*-------------------------------------------------------------------
Função:	Pesquisa uma thread por tid, em uma fila específica
Ret:	A thread com o tid pesquisado, ou NULL caso a mesma não
		tenha sido encontrada na lista pesquisada
-------------------------------------------------------------------*/
Thread_t* SearchThreadByTid(int tid, PFILA2 fila);

/*-------------------------------------------------------------------
Função:	Cria o contexto da thread em questão
Ret:	O contexto que foi criado
-------------------------------------------------------------------*/
//ucontext_t* HandleContext();

/*-------------------------------------------------------------------
Função:	Salva o contexto de execução atual
-------------------------------------------------------------------*/
void SetCheckpoint(ucontext_t* context);

/*-------------------------------------------------------------------
Função:	Finaliza a execução da thread
-------------------------------------------------------------------*/
void FinishThread(Thread_t *activeThread, void *arg);

/*-------------------------------------------------------------------
Função:	Escalona a próxima thread para execução
Ret:	== 0 se tudo ocorreu bem
		!= 0 caso contrário
-------------------------------------------------------------------*/
//void StartNextThread(Thread_t *activeThread, PFILA2 filaAble);

/*-------------------------------------------------------------------
Função:	Deleta a thread na fila desejada
Ret:	== 0 se tudo ocorreu bem
		!= 0 caso contrário
-------------------------------------------------------------------*/
int DeleteFromFila(int tid, PFILA2 fila);

#endif
