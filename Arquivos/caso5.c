
/*
	Condição de teste
		- Várias threads com semáforo iniciando com contador em 2
	Resultado de teste
		- Entra: 0,1,2; tranca:2 -> 0101010101
		- Sai: 0,1; entra 3,4; tranca:4 -> 23232323232323
		- Sai: 2,3; entre 5; -> 454545454545
*/
#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

csem_t sem;

void *th(void *param) {
	int n=(int)param;
	int delay=100;
	
	printf ("Entry th %d\n", n);
	
	cwait(&sem);
	
	if (n<5)
		ccreate(th, (void *)(n+1));
	
	while(--delay) {
		printf ("%d",n);
		cyield();
	}
	
	csignal(&sem);
	
	printf ("\nExit  th %d\n", n);
	return NULL;
}

int main(int argc, char *argv[]) {
	int delay=10000;
	char	name[256];
	
	cidentify (name, 255);
	printf ("GRUPO: %s", name);	
	
	csem_init(&sem, 2);	
	
	printf ("CONFIG: Teste de SEMAFORO inicializado com 2\n");
	printf ("RESULTADO: Roda sempre apenas duas: 01010..., 23232... e 45454...\n");
	getchar();
	
	ccreate(th, (void *)0);
	while(--delay) {
		cyield();
	}
	printf ("Fim do main\n");
	return 0;
}


