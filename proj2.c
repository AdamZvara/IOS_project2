/**
 * @brief Main process for creating child processes
 * @file proj2.c
 * @author Adam Zvara, xzvara01
 * @date 22.4.2021
 */

#include "proj2.h"

//---------------SEMAPHORE FUNCTIONS-------------------
/**
 * @brief Function for initializing semaphores in structure
 *
 * @param sem Pointer to semaphore structure
 *
 * @return 0 If semaphores are created
 */
int init_semaphores(semaphores_t *sem)
{
    //TODO spravit ak sa nepodari malokovat semafor
    sem->mutex = sem_open("/mutex", O_CREAT, 0644, 1);
    //sem->santa_sem = sem_open("/santa_sem", O_CREAT, 0644, 0);
    return 0;
}

/**
 * @brief Function for cleaning the semaphores
 *
 * @param sem Pointer to semaphore structure
 */
void delete_semaphores(semaphores_t *sem)
{
    //semaphore for shared variables
    sem_close(sem->mutex);
    sem_unlink("/mutex");
}

//-------------- SHARED VARIABLE FUNCTIONS ------------------
/**
 * @brief Function for initializing shared memory variables
 *
 * @param sh_vares Shared variables structure
 *
 * @return 0 If everything was successful
 */
int initialize_variables(shared_t *sh_vars)
{
    int fd = shm_open("/process_count", O_RDWR|O_CREAT|O_TRUNC, 0644);
    ftruncate(fd, 4);
    sh_vars->pcount = (u_int *)mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    fd = shm_open("/elves_count", O_RDWR|O_CREAT|O_TRUNC, 0644);
    ftruncate(fd, 4);
    sh_vars->elves = (u_int *)mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    fd = shm_open("/reindeer_count", O_RDWR|O_CREAT|O_TRUNC, 0644);
    ftruncate(fd, 4);
    sh_vars->reindeers = (u_int *)mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    
    return 0;
}

/**
 * @brief Function to clear shared variables
 */
void delete_shared()
{
    shm_unlink("/process_count");
    shm_unlink("/elves_count");
    shm_unlink("/reindeer_count");
}

//---------------- OTHER FUNCTIONS --------------------
/**
 * @brief Function to generate random number from min to max
 *
 * @param min Lower bound of the interval
 * @param max Upper bound of the interval
 *
 * @return Random number within the interval
 */
int random_number(int min, int max)
{
    int number = rand() % (max-min+1);
    return number+min;
}

//--------------- PROCESS FUNCTIONS -------------------
void santa_function(FILE *fr, semaphores_t *sem, shared_t *sh_vars)
{
    //TODO rewrite this critical section into variadic function 
    sem_wait(sem->mutex);
        u_int *count = sh_vars->pcount;
        (*count)++;
        fprintf(fr, "%d: Hello i am santa\n", *count);
        fflush(fr);
    sem_post(sem->mutex);
}

void elf_function(FILE *fr, semaphores_t *sem, shared_t *sh_vars, int my_id, int max_time)
{
    //simulate work
    usleep(random_number(0,max_time)*1000);

    sem_wait(sem->mutex);
        u_int *count = sh_vars->pcount;
        u_int *elf_count = sh_vars->elves;

        (*count)++;
        fprintf(fr, "%d: Elf %d: \n", *count, my_id);
        fflush(fr);

        (*elf_count)++;

    sem_post(sem->mutex);
}

void reindeer_function(FILE *fr, semaphores_t *sem, shared_t *sh_vars, int my_id, int max_time)
{

    //simulate vacation
    usleep(random_number(max_time/2,max_time)*1000);
    
    sem_wait(sem->mutex);
        u_int *count = sh_vars->pcount;
        (*count)++;
        fprintf(fr, "%d: RD %d: \n", *count, my_id);
        fflush(fr);
    sem_post(sem->mutex);
}

//---------------- MAIN FUNCTION ----------------------
int main(int argc, char *argv[])
{
    args_t args;
    if (parse_arguments(argc, argv, &args))
    {
        usage();
        return 1;
    }

    FILE *fr = fopen("proj2.out","w");
    if (fr == NULL) 
    {
        fprintf(stderr, "Subor sa nepodarilo otvorit\n");
        return 1;
    }

    //initialize semaphores
    //TODO fix failed creating
    semaphores_t sem;
    if (init_semaphores(&sem)) return 1;
    
    shared_t sh_vars;
    if (initialize_variables(&sh_vars))
    {
        delete_semaphores(&sem);
        fclose(fr);
        return 1;
    }

    //create one santa process
    pid_t pid = fork();

    if (pid == 0) //santa process
    {
        santa_function(fr , &sem, &sh_vars);
        exit(1);
    }
    else if (pid == -1)
    {
        printf("Fork failed");
    }
    

    //create elf processes
    for(int i = 0; i < args.NE; i++)
    {
        pid = fork();
        
        int elf_id = i+1;
        if (pid == 0) //elf process
        {   
            //generate new random seed for each process
            srand((int)time(0) % getpid());

            elf_function(fr, &sem, &sh_vars, elf_id, args.TE);
            exit(1);
        }
        else if (pid == -1)
        {
            printf("Fork failed");
        }
    }

    for (int i = 0; i < args.NR; i++)
    {
        pid = fork();
        
        int rd_id = i+1;
        if (pid == 0) //reindeer process
        {
            reindeer_function(fr, &sem, &sh_vars, rd_id, args.TR);
            exit(1);
        }
        else if (pid == -1)
        {
            printf("Fork failed");
        }
    }

    while (wait(NULL) > 0);

    delete_semaphores(&sem);
    delete_shared();
    fclose(fr);
    return 0;
}
