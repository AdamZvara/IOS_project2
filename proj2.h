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
    sem_t *p_num_mutex;
    sem_t *count_mutex;
    sem_t *santa;
    sem_t *elf_mutex;
} semaphores_t;

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
 * @return 0 on success
 * @return 1 ff any error occured
 */
int init_semaphores(semaphores_t *sem);

/**
 * @brief Deletes all the semaphores
 *
 * @param Pointer to semaphores_t structure
 */
void delete_semaphores(semaphores_t *sem);


/**
 * @brief Create a single shared variable
 *
 * @param name Name of the shared variable
 * @param var Pointer to pointe to item in shared_t structure
 *
 * @return 0 on succcess
 * @return 1 if any error occured
 */
int create_shared(const char *name, u_int **var);

/**
 * @brief Initialize shared variables between processes
 *
 * @param sh_vars Pointer to shared variable structure
 *
 * @return 0 on succcess
 * @return 1 if any error occured
 */
int initialize_shared(shared_t *sh_vars);

/**
 * @brief Function to clear shared variables
 */
void delete_shared();

/**
 * @brief Function to generate random number from min to max
 *
 * @param min Lower bound of the interval
 * @param max Upper bound of the interval
 *
 * @return Random number within the interval
 */
int random_number(int min, int max);

/**
 * @brief Write message to a file (used by processes)
 *
 * @param fr Pointer to an opened file
 * @param mutex Mutex semaphore to shared counter of current number of process
 * @param sh_vars Pointer to shared variables structure
 * @param msg Last known parameter, format of the message to print out
 *
 * @details This function is variadic. Predefined format of this message is
 * "numOfProcess : *msg", so you don't need to include the first number 
 * in your msg string
 */
void print_msg(FILE *fr, sem_t *mutex, shared_t *sh_vars, const char *msg, ...);

#endif
