#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h> //Para usar a função exp()
#include <windows.h>

#define NUM_RECURSOS 1

//struct para cada Colonia (thread)
typedef struct {
    int id;
    double p0; 					//população inicial
    double r; 					//taxa de crescimento
    int tempo_total; 			//tempo total de simulação
    pthread_mutex_t *recursoA;
    pthread_mutex_t *recursoB;
    int *status;				//array para monitorar o status da thread
} Colonia;

pthread_mutex_t recursoA[NUM_RECURSOS];
pthread_mutex_t recursoB[NUM_RECURSOS];
int thread_status[100]; //máximo de 100 threads

void *simular_crescimento(void *arg) {
    Colonia *colonia = (Colonia *)arg;
    
    for (int t = 0; t < colonia->tempo_total; t++) {
        if (colonia->id % 2 == 0) {
            //colônias pares tentam obter recursos A (Nutrientes) e depois o recurso B (Espaço)
            pthread_mutex_lock(colonia->recursoA);
            printf("Colonia %d obteve Nutrientes (Recurso A)\n", colonia->id);
            Sleep(2000); //simulando tempo para consumir recursos
            
            //indica que está esperando o próximo recurso
            colonia->status[colonia->id] = 1;
            
            pthread_mutex_lock(colonia->recursoB);
            printf("Colonia %d obteve Espaco (Recurso B)\n", colonia->id);
        } else {
            //colônias ímpares tentam obter recurso B (Espaço) e depois recurso A (Nutrientes)
            pthread_mutex_lock(colonia->recursoB);
            printf("Colonia %d obteve Espaco (Recurso B)\n", colonia->id);
            Sleep(2000); //simulando tempo para consumir recursos
            
            //indica que está esperando o próximo recurso
            colonia->status[colonia->id] = 1;
            
            pthread_mutex_lock(colonia->recursoA);
            printf("Colonia %d obteve Nutrientes (Recurso A)\n", colonia->id);
        }

        //calcular a população usando a fórmula de crescimento exponencial
        double populacao = colonia->p0 * exp(colonia->r * t);
        printf("Colonia %d - Tempo %d: Populacao = %.2f\n", colonia->id, t, populacao);
        
        // Libera os recursos
        pthread_mutex_unlock(colonia->recursoA);
        pthread_mutex_unlock(colonia->recursoB);
        
        // Marca a thread como ativa novamente
        colonia->status[colonia->id] = 0;
        
        // Espera antes do próximo ciclo de crescimento
        Sleep(2000);
    }
    
    pthread_exit(NULL);
}

void *monitor_deadlock(void *arg) {
	// esperando qualquer argumento void que será convertido para o cast (int)
	//status será 1 se a thread estiver esperando
	int *status = (int *)arg;
	int prev_status[100] = {0};
	int wait_count[100] = {0}; //contador para quantas vezes a thread está "esperando"
	
	// 1 por causa das threads que estão em espera
	while(1) {
		Sleep(5000); // Verifica a cada 5 segundos o status da thread
		for(int i = 0; i < 100; i++) {
			if(status[i] == 1 && prev_status[i] == 1) {
				wait_count[i]++;
				if(wait_count[i] > 3) {
					// Se a thread ainda estiver esperando, pode ter acontecido um deadlock
					printf("Deadlock detectado: Colonia %d esta parada esperando.\n", i);
				}
			} else {
				wait_count[i] = 0; //reseta se não estiver mais preso
			}
			prev_status[i] = status[i];
		}
	}
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Uso: %s <Populacao inicial> <Taxa de crescimento> <Tempo total> <Numero de colonias> <Numero de recursos de cada tipo>\n", argv[0]);
        return -1;
    }
    
    double p0 = atof(argv[1]);
    double r = atof(argv[2]);
    int tempo_total = atoi(argv[3]);
    int num_colonias = atoi(argv[4]);
    int num_recursos = atoi(argv[5]);

    // Inicializa os mutexes
    for (int i = 0; i < num_recursos; i++) {
        pthread_mutex_init(&recursoA[i], NULL);
        pthread_mutex_init(&recursoB[i], NULL);
    }
    
    pthread_t threads[num_colonias];
    pthread_t monitor_thread;
    Colonia colonias[num_colonias];

    // Cria e inicializa as colônias (threads)
    for (int i = 0; i < num_colonias; i++) {
        colonias[i].id = i;
        colonias[i].p0 = p0;
        colonias[i].r = r;
        colonias[i].tempo_total = tempo_total;
        colonias[i].recursoA = &recursoA[i % num_recursos];
        colonias[i].recursoB = &recursoB[i % num_recursos];
        colonias[i].status = thread_status;
        pthread_create(&threads[i], NULL, simular_crescimento, (void *)&colonias[i]);
    }
    
    // cria uma thread de monitoramento para detectar deadlock
    pthread_create(&monitor_thread, NULL, monitor_deadlock, (void *)thread_status);

    // Espera as threads terminarem
    for (int i = 0; i < num_colonias; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // cancela a thread de monitoramento
    pthread_cancel(monitor_thread);
    
    //destrói os mutexes
    for (int i = 0; i < num_recursos; i++) {
        pthread_mutex_destroy(&recursoA[i]);
        pthread_mutex_destroy(&recursoB[i]);
    }

    return 0;
}

