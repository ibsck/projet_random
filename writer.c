#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>

#define TAILLE_TAB 10000000
#define TAILLE_TAB_FINAL 2000 // La taille mémoire partagée
#define NB_LOOP 214 // RANDMAX / 10 millions, le tout en entier

//prototypes des fonctions
void getRandom(int *tab);
int * createSharedMemory(key_t key, int size); 

typedef union semun {
    int val;
    struct semid_ds * buffer;
    unsigned short int * table;
} semun_t;

int main(int argc, char *argv[] )
{
    //variable memoire partagee
    int shmid;
    int *memory;
    
    //variable semaphore
    semun_t u_semun;
    unsigned short table[1];
    int sem;
    struct sembuf sembuf;
     
    //creation d'une cle pour la memoire partagee
    key_t key = ftok("fiche", 'a');
    memory = createSharedMemory(key, TAILLE_TAB_FINAL * sizeof(int));
    //creation du semaphore
    if ((sem = semget(key, 1, 0)) == -1) 
    {
        if ((sem = semget(key, 1, IPC_CREAT | IPC_EXCL | 0600)) == -1) 
        {
            perror("semget");
            exit(EXIT_FAILURE);
        }
        table[0] = 1;
        u_semun.table = table;
        if (semctl(sem, 0, SETALL, u_semun) < 0)
            perror("semctl");
    } 
    srand((unsigned)time(NULL)^getpid());

    for(int i = 0; i < NB_LOOP; i++){
        //tableau de 10 millions d'entiers local
        int * array =  malloc(TAILLE_TAB * sizeof(int));
        getRandom(array); //remplissage du tableau local
        printf("%d\n",i);

        //le semaphore bloque la memoire partagee
        sembuf.sem_num = 0;
        sembuf.sem_op = -1;
        sembuf.sem_flg = 0;
        if (semop(sem, &sembuf, 1) == -1) 
        {
            perror("semop");
            exit(EXIT_FAILURE);
        }
        //ecriture dans la memoire partagee

        for(int j=0; j < TAILLE_TAB; j++){
            memory[array[j] % TAILLE_TAB_FINAL]++;
        } 

        //le semaphore debloque la memoire partagee 
        sembuf.sem_num = 0;
        sembuf.sem_op = 1;
        sembuf.sem_flg = 0;
        if (semop(sem, &sembuf, 1) == -1) 
        {
            perror("semop");
            exit(EXIT_FAILURE);
        }
        free(array); //liberation de la memoire
    }
    //le fils se detache de la memoire partagee
    if (shmdt(memory) == -1) 
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    printf("fils fini\n");
    return EXIT_SUCCESS;
}

/*  fonction genere 10 Millions de nombres aleatoires compris entre 0
    et RAND_MAX et les stocker dans le tableau passé en parametre   */

void getRandom(int *tab) 
{
    for(int i = 0; i < TAILLE_TAB; i++){ tab[i] = rand(); }
}

/*  fonction cree une memoire partagee de taille size et retourne un
    pointeur sur la memoire partagee  */

int * createSharedMemory(key_t key, int size)
{
    int shmid;
    int *shm;
    if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((shm = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }
    return shm;
}

