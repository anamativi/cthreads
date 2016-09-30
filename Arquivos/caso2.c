
/*
	Condição de teste
		- ccreate
		- thread COM cyield
	Resultado de teste
		- Alterna entre as três!
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
		cyield();
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	int delay=10000;
	char	name[256];
	
	cidentify (name, 255);
	printf ("GRUPO: %s", name);	
	
	printf ("CONFIG: create, COM yield\n");
	printf ("RESULTADO: Alternar entre as 3 threads\n");
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


