#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h> //para usar a função exp()
#include <windows.h>


//struct para cada Colonia (thread)
typedef struct {
    int id;
    int p0; 					//população inicial
    float r; 					//taxa de crescimento
    int tempo_total; 			//tempo total de simulação
    pthread_mutex_t *recursoA;
    pthread_mutex_t *recursoB;
} Colonia;

void *simular_crescimento(void *arg) {
    Colonia *colonia = (Colonia *)arg;
    
    for (int t = 0; t < colonia->tempo_total; t++) {
        if (colonia->id % 2 == 0) {
            //colônias pares tentam obter recursos A (Nutrientes) e depois o recurso B (Espaço)
            pthread_mutex_lock(colonia->recursoA);
            printf("Colonia %d obteve Nutrientes (Recurso A)\n", colonia->id);
            Sleep(2000); //simulando tempo para consumir recursos
            
            pthread_mutex_lock(colonia->recursoB);
            printf("Colonia %d obteve Espaco (Recurso B)\n", colonia->id);
        } else {
            //colônias ímpares tentam obter recurso B (Espaço) e depois recurso A (Nutrientes)
            pthread_mutex_lock(colonia->recursoB);
            printf("Colonia %d obteve Espaco (Recurso B)\n", colonia->id);
            Sleep(2000); //simulando tempo para consumir recursos
            
            pthread_mutex_lock(colonia->recursoA);
            printf("Colonia %d obteve Nutrientes (Recurso A)\n", colonia->id);
        }

        //calcula a população usando a fórmula de crescimento exponencial
        double populacao = colonia->p0 * exp(colonia->r * t);
        printf("Colonia %d - Tempo %d: Populacao = %.2f\n", colonia->id, t, populacao);
        
        //libera os recursos
        pthread_mutex_unlock(colonia->recursoA);
        pthread_mutex_unlock(colonia->recursoB);
        
        //espera antes do próximo ciclo de crescimento
        Sleep(2000);
    }
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    //argc indica os elementos que a função main() recebe na linha de comando
    //argv é um vetor ponteiro de char que lista todos os argumentos
	if (argc != 6) {
        printf("Uso: %s <Populacao inicial> <Taxa de crescimento> <Tempo total> <Numero de colonias> <Numero de recursos de cada tipo>\n", argv[0]);
        return -1;
    }
    
    //direcionamento de cada argumento vindo da compilação
    double p0 = atof(argv[1]);
    double r = atof(argv[2]);
    int tempo_total = atoi(argv[3]);
    int num_colonias = atoi(argv[4]);
    int num_recursos = atoi(argv[5]);

    pthread_t threads[num_colonias];
    pthread_mutex_t recursosA[num_recursos];
    pthread_mutex_t recursosB[num_recursos];
    Colonia colonias[num_colonias];

    //inicializa os mutexes
    for (int i = 0; i < num_recursos; i++) {
        pthread_mutex_init(&recursosA[i], NULL);
        pthread_mutex_init(&recursosB[i], NULL);
    }

    //criação das colônias (threads)
    for (int i = 0; i < num_colonias; i++) {
        colonias[i].id = i;
        colonias[i].p0 = p0;
        colonias[i].r = r;
        colonias[i].tempo_total = tempo_total;
        colonias[i].recursoA = &recursosA[i % num_recursos];
        colonias[i].recursoB = &recursosB[i % num_recursos];
        pthread_create(&threads[i], NULL, simular_crescimento, (void *)&colonias[i]);
    }

    //espera as threads terminarem
    for (int i = 0; i < num_colonias; i++) {
        pthread_join(threads[i], NULL);
    }

    //destrói os mutexes
    for (int i = 0; i < num_recursos; i++) {
        pthread_mutex_destroy(&recursosA[i]);
        pthread_mutex_destroy(&recursosB[i]);
    }

    return 0;
}

