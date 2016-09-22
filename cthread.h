/*
 * cthread.h: arquivo de inclusão com os protótipos das funções a serem
 *            implementadas na realização do trabalho.
 *
 * NÃO MODIFIQUE ESTE ARQUIVO.
 *
 * VERSÃO 1 - 04/04/2016
 */
#ifndef __cthread__
#define __cthread__

typedef struct _sFila2 sFila2;

typedef struct s_sem {
	int	count;	// indica se recurso está ocupado ou não (livre > 0, ocupado = 0)
	sFila2*	fila; 	// ponteiro para uma fila de threads bloqueadas no semáforo
} csem_t;

int ccreate (void* (*start)(void*), void *arg);
int cyield(void);
int cjoin(int tid);
int csem_init(csem_t *sem);
int cwait(csem_t *sem);
int csignal(csem_t *sem);

#endif
