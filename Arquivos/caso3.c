
/*
	Condição de teste
		- main: create(th1)
		- th1: create(th2); join(th2)
		- th2: COM cyield
	Resultado de teste
		- Gera toda a th2 e depois a th1.
*/
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void *th2(void *param) {
	int n=(int)param;
	int cont=10;
	while(cont) {
		printf ("%d",n);
		--cont;
		cyield();
	}
	return NULL;
}

void *th1(void *param) {
	int n=(int)param;
	int cont=10;
	int id2 = ccreate(th2, (void *)2);
	cjoin(id2);
	while(cont) {
		printf ("%d",n);
		--cont;
		cyield();
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	int delay=10000;
	char	name[256];
	
	cidentify (name, 255);
	printf ("GRUPO: %s", name);	
	
	printf ("CONFIG: Teste de JOIN\n");
	printf ("RESULTADO: Roda toda a thread 2, depois a 1\n");
	getchar();
	
	ccreate(th1, (void *)1);
	while(--delay) {
		cyield();
	}
	printf ("Fim do main\n");
	return 0;
}


