
/*
	Condição de teste
		- Thread com semaforo para garantir a ordem de execucao
	Resultado de teste
		- Gera uma sequencia 0-1-2-3-4 ...(não pode mudar a ordem)
*/
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

int shared=0;
csem_t sem;

void *th(void *param) {
	int n=(int)param;
	int flag;
	
	while(1) {
		cwait(&sem);
		if (shared>=100) flag=1;
		if (n==1) cyield();
		csignal(&sem);
		
		if (flag) break;
		
		cwait(&sem);
		if (shared%5==0) printf ("\n");
		printf ("(%d)%d-", n, shared);
		if (n==2) cyield();
		++shared;
		if (n==3) cyield();
		csignal(&sem);
		
		cyield();
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	
	int delay=10000;
	char	name[256];
	
	cidentify (name, 255);
	printf ("GRUPO: %s", name);	

	csem_init(&sem, 1);
	
	printf ("CONFIG: Teste de SEMAFORO\n");
	printf ("RESULTADO: Gera uma sequencia 0-1-2-3-4 ...(não pode mudar a ordem)\n");
	getchar();
	
	ccreate(th, (void *)0);
	ccreate(th, (void *)1);
	ccreate(th, (void *)2);
	ccreate(th, (void *)3);
	ccreate(th, (void *)4);
	while(--delay) {
		cyield();
	}
	printf ("Fim do main\n");
	return 0;
}


