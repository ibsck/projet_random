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
#define NB_PROCESS 10 //nombre de processus

void createSharedMemory(key_t key, int size, int *shmid, int **shm);
void destroySharedMemory(int shmid);

int main(int argc, char * argv[])
{
    int i;
    int shmid;
    int *shm, *s;
    key_t key = ftok("fiche", 'a');
    
    //le pere cree NB_PROCESS processus
    for(i=0; i<NB_PROCESS; i++)
    {
        if(fork() == 0)
        {
           /*
            le processus fils doit creer une memoire partagee 
            et generer un tableau de 2 milliards d'entiers aleatoires
            et stocker les occurences de chaque entier dans la memoire partagee
           */   
            printf("creation processus %d\n",i+1);
            if(execl("./writer", "writer", NULL) == -1)
            {
                perror("execl");
                return 1;
            }
            exit(EXIT_SUCCESS);
        }
        
    }
    
    //le pere execute la suite
    printf("je suis le pere\n");
    printf("mon pid est %d\n", getpid());

    //le pere se branche sur le segment de memoire partagee
    createSharedMemory(key, 2000*sizeof(int), &shmid, &shm);
    
    //le pere attend la fin des fils
    for(int j=0; j < NB_PROCESS; j++)
    {
        wait(NULL);
        //affichage indicateur de progression
        printf("%d%%\n", (j+1)*10);
    }
    
    unsigned long  nbtotallancer = (unsigned long)214*10000000*NB_PROCESS; //nombre total de lancer
    int nbtailletableau = 2000; //nombre de case du tableau
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

    printf("nombre de case avec une erreur supérieur à 5%% : %d\n",equilibrage);
    printf("nombre de case avec une erreur inférieur à 5%% : %d\n",nbtailletableau - equilibrage);
    printf("pourcentage de case avec une erreur inférieur à 5%% : %f %%\n",((nbtailletableau - equilibrage)*100)/(float)nbtailletableau);
    printf("pourcentage de case avec une erreur supérieur à 5%% : %f %%\n",((equilibrage)*100)/(float)nbtailletableau);
    
    if(equilibrage > 0){
        printf("le tirage n'est pas équilibré\n");
    }
    else{
        printf("le tirage est équilibré\n");
    }
  
    //le pere detruit la memoire partagée
    destroySharedMemory(shmid);
    return EXIT_SUCCESS;
}
/*  fonction cree une memoire partagee de taille size 
    si elle n'a pas déjà é été créée 
    sinon elle retourne un pointeur sur la memoire partagee existante
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
  fonction detruit la memoire partagee
*/
void destroySharedMemory(int shmid)
{
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
}

















