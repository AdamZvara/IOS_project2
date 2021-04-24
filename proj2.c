/**
 * @brief Main process for creating child processes
 * @file proj2.c
 * @author Adam Zvara, xzvara01
 * @date 22.4.2021
 */

#include "proj2.h"

#define MUTEX sem->p_num_mutex

//---------------SEMAPHORE FUNCTIONS-------------------

int init_semaphores(semaphores_t *sem)
{
    //TODO spravit ak sa nepodari alokovat semafor
    
    sem->p_num_mutex = sem_open("/p_num_mutex", O_CREAT, 0644, 1);
    sem->count_mutex = sem_open("/count_mutex", O_CREAT, 0644, 1);
    sem->santa = sem_open("/santa", O_CREAT, 0644, 0);
    sem->elf_mutex = sem_open("/elf_mutex", O_CREAT, 0644, 1);
    return 0;
}

void delete_semaphores(semaphores_t *sem)
{
    sem_close(sem->p_num_mutex);
    sem_close(sem->santa);
    sem_close(sem->count_mutex);
    sem_close(sem->elf_mutex);

    sem_unlink("/p_num_mutex");
    sem_unlink("/santa");
    sem_unlink("/count_mutex");
    sem_unlink("/elf_mutex");
}

//-------------- SHARED VARIABLE FUNCTIONS ------------------

int create_shared(const char *name, u_int **var)
{
    //TODO fix if var was not created
    int fd = shm_open(name, O_RDWR|O_CREAT|O_TRUNC, 0644);
    ftruncate(fd, 4);
    *var = (u_int *)mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    **var = 0;

    return 0;
}

int initialize_shared(shared_t *sh_vars)
{
    create_shared("/process_count", &(sh_vars->pcount));
    create_shared("/elves_count",&(sh_vars->elves));
    create_shared("/reindeer_count", &(sh_vars->reindeers));

    return 0;
}

void delete_shared()
{
    shm_unlink("/process_count");
    shm_unlink("/elves_count");
    shm_unlink("/reindeer_count");
}

//---------------- OTHER FUNCTIONS --------------------

int random_number(int min, int max)
{
    int number = rand() % (max-min+1);
    return number+min;
}

//--------------- PROCESS FUNCTIONS -------------------
void print_msg(FILE *fr, sem_t *mutex, shared_t *sh_vars, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    sem_wait(mutex);
        u_int *count = sh_vars->pcount;
        (*count)++;
        fprintf(fr, "%d: ", *count);
        vfprintf(fr, msg, args);
        fflush(fr);
    sem_post(mutex);

    va_end(args);

}

void santa_function(FILE *fr, semaphores_t *sem, shared_t *sh_vars)
{
    print_msg(fr, MUTEX, sh_vars, "Santa: going to sleep\n");
    
    int count = 0;

    /*
    while (1)
    {
    sem_wait(sem->santa);
    sem_wait(sem->count_mutex);
        if (*sh_vars->elves == 3)
        {
            print_msg(fr, MUTEX, sh_vars, "Santa: helping elves\n");
            count++;
        }
    sem_post(sem->count_mutex);

    if (count == 3)
    {
        break;
    }
    }*/
}

void elf_function(FILE *fr, semaphores_t *sem, shared_t *sh_vars, int my_id, int max_time)
{
    print_msg(fr, MUTEX, sh_vars, "Elf %d: started\n", my_id);
    
    //simulate work
    usleep(random_number(0,max_time)*1000);
    /*
    sem_wait(sem->elf_mutex);
        sem_wait(sem->count_mutex);
            (*sh_vars->elves)++;
            print_msg(fr, MUTEX, sh_vars, "Elf %d: need help\n", my_id);
            if ((*sh_vars->elves) == 3)
            {
                sem_post(sem->santa);
            } 
            else
            {
                sem_post(sem->elf_mutex);    
            }
        sem_post(sem->count_mutex);


        sem_wait(sem->count_mutex);
            (*sh_vars->elves)--;
            if ((*sh_vars->elves) == 0)
            {
                sem_post(sem->elf_mutex);
            }
        sem_post(sem->count_mutex);
    */
}

void reindeer_function(FILE *fr, semaphores_t *sem, shared_t *sh_vars, int my_id, int max_time)
{
    print_msg(fr, sem->p_num_mutex, sh_vars, "RD %d: rstarted\n", my_id);
    
    //simulate vacation
    usleep(random_number(max_time/2,max_time)*1000);
}

//---------------- MAIN FUNCTION ----------------------
int main(int argc, char *argv[])
{
    pid_t wpid;
    int status = 0;

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
    if (initialize_shared(&sh_vars))
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

    while ((wpid = wait(&status)) > 0);

    delete_semaphores(&sem);
    delete_shared();
    
    printf("Parent process\n");
    fflush(stdout);
    
    fclose(fr);
    return 0;
}
