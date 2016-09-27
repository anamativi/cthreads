/*
 * cdata.h: arquivo de inclusão de uso apenas na geração da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida
 *
 */
#ifndef __cdata__
#define __cdata__

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4


typedef int BOOLEAN;
#define TRUE 1
#define FALSE 0
//#define NULL 0

/* NÃO ALTERAR ESSA struct */
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
	int 		ticket;		// "bilhete" de loteria da thread, para uso do escalonador
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
} TCB_t; 

typedef struct s_Thread {
	
	TCB_t data;	// Informações da thread

	BOOLEAN yield;
	BOOLEAN has_thread_waiting; // Tem alguma thread esperando pela nossa
	BOOLEAN is_waiting;			// Esta thread está esperando por outra
	
	// A thread que está esperando pela nossa é a seguinte:
	struct Thread_t* waitingThread; 

} Thread_t;

#endif





















