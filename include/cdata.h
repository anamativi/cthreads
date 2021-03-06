/*
 * cdata.h: arquivo de inclus�o de uso apenas na gera��o da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida
 *
 */
#ifndef __cdata__
#define __cdata__

#include "ucontext.h"

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4

#define Function(func) void*(*func)(void*)

typedef int BOOLEAN;
#define TRUE 1
#define FALSE 0
//#define NULL 0

/* N�O ALTERAR ESSA struct */
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Cria��o; 1: Apto; 2: Execu��o; 3: Bloqueado e 4: T�rmino
	int 		ticket;		// "bilhete" de loteria da thread, para uso do escalonador
	ucontext_t 	context;	// contexto de execu��o da thread (SP, PC, GPRs e recursos) 
} TCB_t; 

typedef struct s_Thread {
	
	TCB_t data;	// Informa��es da thread

	BOOLEAN yield;
	BOOLEAN has_thread_waiting; // Tem alguma thread esperando pela nossa
	BOOLEAN is_waiting;			// Esta thread est� esperando por outra
	
	// A thread que est� esperando pela nossa � a seguinte:
	struct s_Thread* waitingThread; 

} Thread_t;

#endif





















