#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cdata.h"
#include "cthread.h"


////////////////
// TESTE BASE //
////////////////

/*

int canContinue = 0;
csem_t* globalSem;
int contadorGlobal = 0;

void Teste(void* _arg)
{
    int* val = (int*)_arg;
    int i;

    for(i=0; i<10; i++)
    {
        printf("Valor global eh: %d\n", RetornaContadorGlobal(*(int*)_arg));
        cyield();
    }

    canContinue = 1;
}

int RetornaContadorGlobal(int _tid)
{
    int tid = _tid;
    cwait(globalSem);

    contadorGlobal++;
    int valorARetornar = contadorGlobal-1;

    cyield();

    csignal(globalSem);

    return valorARetornar;
}

int main()
{
    printf("Teste\n\n");

    globalSem = (csem_t*)malloc(sizeof(csem_t));
    csem_init(globalSem);

    int val1 = 0;
    int val2 = 1;

    int pid1 = ccreate(&Teste, &val1);
    int pid2 = ccreate(&Teste, &val2);

    cjoin(pid2);
    cjoin(pid1);

    printf("Hello world!\n");
    return 0;
}
*/

///////////////
// EXEMPLO 1 //
///////////////

void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
}

int main(int argc, char *argv[]) {

	int	id0, id1;
	int i;

	id0 = ccreate(func0, (void *)&i);
	//id1 = ccreate(func1, (void *)&i);

	//printf("Eu sou a main após a criação de ID0 e ID1\n");

	//cjoin(id0);
	//cjoin(id1);

	//printf("Eu sou a main voltando para terminar o programa\n");
}

///////////////
// EXEMPLO 2 //
///////////////

/*
struct work{
    double c0_r;
    double c0_i;
    double c1_r;
    double c1_i;
    int    lim;
    int    n;
    int    t;
    int    tid;
    int*  region;
};


double  distance(double a, double b) {
    return (a*a + b*b);
}

void*   mandel(void *arg) {
    double dx, dy, z_r, z_i, c_r, c_i, c0_r, c0_i, c1_r, c1_i, aux;
    int i, j, k, lim, n, t, tid, yi, yf;
    int *plan;



    c0_r = ((struct work*)(arg))->c0_r;
    c0_i = ((struct work*)arg)->c0_i;
    c1_r = ((struct work*)arg)->c1_r;
    c1_i = ((struct work*)arg)->c1_i;
    lim  = ((struct work*)arg)->lim;
    n = ((struct work*)arg)->n;
    t = ((struct work*)arg)->t;
    tid = ((struct work*)arg)->tid;

    plan = (int *)malloc( n/t * n * sizeof(int));
    ((struct work*)arg)->region = plan;


    dx = (c1_r - c0_r)/n;
    dy = (c0_i - c1_i)/n;


    yi = tid * n/t;
    yf = (tid+1) * n/t - 1;

    printf("thread calculando da linha %d a linha %d...\n", yi, yf);

    for (i=yi; i<=yf; i++ ) {
        for ( j = 0 ; j < n ; j++ ) {
            z_r = 0;
            z_i = 0;
            c_r = c0_r + j*dx;
            c_i = c0_i - i*dy;

            k = 0;
            while ( distance(z_r, z_i) < 4 && k < lim) {
                aux = z_r*z_r - z_i*z_i + c_r;
                z_i = 2*z_r*z_i + c_i;
                z_r = aux;
                k++;
            }

            plan[(i-yi)*n+j] = (int)(k*((double)255/lim)+ 0.5i);
          }
      }
    return(NULL);
}

void usage(void) {
    fprintf(stderr, "usage: mandel [-t thread] [-n size] [-c c0_r c0_i c1_r c1_i] \n");
    fprintf(stderr, "       [-l limit] [-p]\n");
    exit(1);
}

void error(char *code){
    fprintf(stderr, "%s\n", code);
    exit(1);
}

int main(int argc, char **argv) {
    int i, r, print=0, t=1, n=256, lim=100;
    double c0_r=-2.0, c0_i=1.25, c1_r=0.5, c1_i=-1.25;
    struct work *workload;
    int *workers;
    FILE *fp;


    for (i = 1; i < argc; i++){
        if (argv[i][0] != '-') usage();
        switch (argv[i][1]) {
            case 't': i++; t = atoi(argv[i]);
                      break;
            case 'n': i++; n = atoi(argv[i]);
                      break;
            case 'c': i++; c0_r = atof(argv[i]);
                      i++; c0_i = atof(argv[i]);
                      i++; c1_r = atof(argv[i]);
                      i++; c1_i = atof(argv[i]);
                      break;
            case 'l': i++; lim = atoi(argv[i]);
                      break;
            case 'p': print = 1;
                      break;
            default : usage();
                      break;
         }
    }

    print = 1;
    t = 4;

    workload = (struct work *) malloc(t * sizeof(struct work));
    if (workload == NULL)
       error("ERRO: Alocacao de memória para workload");

    workers = (int *)malloc(t * sizeof(int));
    if (workers == NULL)
       error("ERRO: Alocacao de memória para workers");

    for (i=0; i< t; i++) {
        workload[i].c0_r = c0_r;
        workload[i].c0_i = c0_i;
        workload[i].c1_r = c1_r;
        workload[i].c1_i = c1_i;
        workload[i].lim = lim;
        workload[i].n = n;
        workload[i].t = t;
        workload[i].tid = i;
        workload[i].region = NULL;
    }

    for (i=0; i<t; i++) {
        workers[i] = ccreate(mandel, (void *)(&workload[i]));
        if (workers[i] == -1)
           error("ERRO: Problema na criacao de thread worker\n");
    }

    for (i=0; i<t; i++)
        cjoin(workers[i]);

    if (print) {
       fp = fopen("mandel.ppm", "w");
       printf("Gerando arquivo mandel.ppm...\n");
       fprintf(fp, "P3 %d %d 3\n", n, n);
       for (i=0; i< t; i++) {
           for( r= 0; r < n*n/t; r++)
             fprintf(fp, "%d %d %d \n", (workload[i].region[r]>>5) & 6,
                                        (workload[i].region[r]>>3)& 7,
                                        workload[i].region[r] & 7  );
           free(workload[i].region);
       }
    }

    printf("Final do Mandelbrot...\n");
    exit(0);
}

*/

///////////////
// EXEMPLO 3 //
///////////////

/*

#define		MAX_SIZE	250
#define		MAX_THR		10

int vetor[MAX_SIZE];
int  inc = 0;

void *func(void *arg){

   while ( inc < MAX_SIZE ) {
       vetor[inc] = (int)arg;
       inc++;
       if ( (inc % 20) == 0 )
           cyield();
       else
           continue;
   }

   return (NULL);
}


int main(int argc, char *argv[]) {
    int i, pid[MAX_THR];


    for (i = 0; i < MAX_THR; i++) {
        pid[i] = ccreate(func, (void *)('A'+i));
       if ( pid[i] == -1) {
          printf("ERRO: criação de thread!\n");
          exit(-1);
       }
     }

    for (i = 0; i < MAX_THR; i++)
         cjoin(pid[i]);

    for (i = 0; i < MAX_SIZE; i++) {
        if ( (i % 20) == 0 )
           printf("\n");
        printf("%c", (char)vetor[i]);
    }

    printf("\nConcluido vetor de letras...\n");
    exit(0);
}

*/
