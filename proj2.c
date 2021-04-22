/**
 * @brief Main process for creating child processes
 * @file proj2.c
 * @author Adam Zvara, xzvara01
 * @date 22.4.2021
 */

//forking
#include <sys/types.h>
#include <unistd.h>

//waiting
#include <sys/wait.h>

//shared variables
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  

//semaphores
#include <semaphore.h>

#include "proj2.h"


int main(int argc, char *argv[])
{
    args_t arguments;
    if (parse_arguments(argc, argv, &arguments))
    {
        usage();
        return 1;
    }
 
    sem_t *sem = sem_open("/semaphore", O_CREAT, 0644, 1);

    int fd = shm_open("/counter", O_RDWR|O_CREAT|O_TRUNC, 0644);
    ftruncate(fd, 4);

    u_int *counter = (u_int *)mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    //create one santa process
    pid_t pid = fork();

    if (pid == 0) //santa process
    {
        sem_wait(sem);
            (*counter)++;
            printf("%d : Hello i am santa\n", *counter);
            fflush(stdout);
        sem_post(sem);
        exit(1);
    }
    else if (pid == -1)
    {
        printf("Fork failed");
    }
    
    //main process continues
    

    //create elf processes
    for(int i = 0; i < arguments.NE; i++)
    {
        pid = fork();

        if (pid == 0)
        {
            sem_wait(sem);
                (*counter)++;
                printf("%d : Me elf\n", *counter); //elf process body
                fflush(stdout);
            sem_post(sem);
            exit(1);
        }
        else if (pid == -1)
        {
            printf("Fork failed");
        }
    }

    for (int i = 0; i < arguments.NR; i++)
    {
        pid = fork();

        if (pid == 0) //reindeer process body
        {
            sem_wait(sem);
                (*counter)++;
                printf("%d : *reindeer sound*\n", *counter);
                fflush(stdout);
            sem_post(sem);
            exit(1);
        }
        else if (pid == -1)
        {
            printf("Fork failed");
        }
    }

    while (wait(NULL) > 0);

    (*counter)++;
    printf("%d : Im a parent\n", *counter);
 
    sem_close(sem);
    sem_unlink("/semaphore");

    shm_unlink("/counter");
    return 0;
}
