#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <time.h>
#include <math.h>

#define NB_PROCESS 10 // Nombre de processus
#define NB_LOOP 214 // RANDMAX / 1 Milliard le tout en entier
#define NB_RANDOM_GENERE 10000000
#define TAILLE_TAB_FINAL 2000 // La taille mémoire partagée

void createSharedMemory(key_t key, int size, int *shmid, int **shm);
void destroySharedMemory(int shmid);

int main(int argc, char * argv[])
{
    int i;
    int shmid;
    int *shm, *s;
    key_t key = ftok("fiche", 'a');
    
    // Le pere cree NB_PROCESS processus
    for(i=0; i<NB_PROCESS; i++)
    {
        if(fork() == 0)
        {
           /*
            Le processus fils doit créer une mémoire partagée
            et générer un tableau de 2 milliards d'entiers aléatoires
            et stocker les occurences de chaque entier dans la mémoire partagée
           */   

            if(execl("./writer", "writer", NULL) == -1)
            {
                perror("execl");
                return 1;
            }
            exit(EXIT_SUCCESS);
        } else{
            printf("Création du processus n%d\n",i+1);
        }
        
    }

    //le pere se branche sur le segment de memoire partagee
    createSharedMemory(key, TAILLE_TAB_FINAL * sizeof(int), &shmid, &shm);
    
    //le pere attend la fin des fils
    for(int j=0; j < NB_PROCESS; j++)
    {
        wait(NULL);
        //affichage indicateur de progression
        printf("%d%%\n", (j+1) * NB_PROCESS);
    }
    
    unsigned long  nbtotallancer = (unsigned long)NB_LOOP * NB_RANDOM_GENERE * NB_PROCESS; //nombre total de lancer
    int nbtailletableau = TAILLE_TAB_FINAL; //nombre de case du tableau
    float nbtotal = nbtotallancer / nbtailletableau; //nombre de lancer par case du tableau
    float erreur = 0;
    float erreurpourcent = 0;
    printf("nb total lancer : %ld\n",nbtotallancer);
    printf("nb total par case : %f\n",nbtotal);

    s = shm; //pointeur sur la memoire partagee
    int equilibrage = 0;
    for(int i=0;i<nbtailletableau;i++){
        erreur = abs(nbtotal - *s);
        erreurpourcent = (erreur/nbtotal)*100;
        if(erreurpourcent > 5){
            equilibrage++;
        }
        s++;
    }

    printf("Nombre de case avec une erreur supérieur à 5%% : %d\n",equilibrage);
    printf("Nombre de case avec une erreur inférieur à 5%% : %d\n",nbtailletableau - equilibrage);
    printf("Pourcentage de case avec une erreur inférieur à 5%% : %f %%\n",((nbtailletableau - equilibrage)*100)/(float)nbtailletableau);
    printf("Pourcentage de case avec une erreur supérieur à 5%% : %f %%\n",((equilibrage)*100)/(float)nbtailletableau);
    
    if(equilibrage > 0){
        printf("Le tirage n'est pas équilibré\n");
    }
    else{
        printf("Le tirage est équilibré\n");
    }
  
    // Le père détruit la mémoire partagée
    destroySharedMemory(shmid);
    return EXIT_SUCCESS;
}
/*  La fonction crée une mémoire partagée de taille size
    si elle n'a pas déjà été créée
    sinon elle retourne un pointeur sur la mémoire partagée existante
*/
void createSharedMemory(key_t key, int size, int *shmid, int **shm)
{
    if ((*shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    if ((*shm = shmat(*shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
}
/* 
  La fonction détruit la mémoire partagée
*/
void destroySharedMemory(int shmid)
{
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
}

















