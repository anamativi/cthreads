
/*
	Condição de teste
		- ccreate
		- thread sem cyield
	Resultado de teste
		- Roda apenas uma das threads
*/
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void *th(void *param) {
	int n=(int)param;
	int cont=100;
	while(cont) {
		printf ("%d",n);
		--cont;
	}
	return NULL; //huebr
}

int main(int argc, char *argv[]) {
	int delay=10000;
	char	name[256];
	
	cidentify (name, 255);
	printf ("GRUPO: %s", name);	
	
	printf ("CONFIG: create, sem yield\n");
	printf ("RESULTADO: Roda apenas uma das threads de cada vez\n");
	getchar();
	
	ccreate(th, (void *)1);
	ccreate(th, (void *)2);
	ccreate(th, (void *)3);
	while(--delay) {
		cyield();
	}
	printf ("Fim do main\n");
	return 0;
}



