/**
 * @brief Header file for proj2.c
 * @file proj2.h
 * @author Adam Zvara
 * @date 22.4.2021
 */

#ifndef PROJ2_H
#define PROJ2_H

//forking
#include <sys/types.h>
#include <unistd.h>

//waiting
#include <sys/wait.h>

//shared variables
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>  

//standard libs
#include <stdio.h>
#include <stdlib.h>

//semaphores management
#include <semaphore.h>

//variadic functions
#include <stdarg.h>

//random number seed
#include <time.h>

//parsing arguments
#include "parse_args.h"

//define max values for elves and reindeers
#define MAX_ELVES 1000
#define MAX_RAIND 20
#define MAX_ELF_T 1000
#define MAX_REIND_T 1000

typedef struct semaphores
{
    sem_t *mutex;
    sem_t *santa_sem;
} semaphores_t;

//TODO is this necessary?
typedef struct shared
{
    u_int *pcount;
    u_int *elves;
    u_int *reindeers;
} shared_t;

/**
 * @brief Initializes all the semaphores in sem. structure
 * 
 * @param Pointer to semaphores_t structure
 *
 * @return 0 On success
 * @return 1 If any error occured
 */
int init_semaphores(semaphores_t *sem);

/**
 * @brief Deletes all the semaphores
 *
 * @param Pointer to semaphores_t structure
 */
void delete_semaphores(semaphores_t *sem);

#endif
