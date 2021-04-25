/**
 * @brief Main process for creating child processes
 * @file proj2.c
 * @author Adam Zvara, xzvara01
 * @date 22.4.2021
 */

#include "proj2.h"


//--------------- SEMAPHORES AND  FUNCTIONS -------------------
sem_t* p_num_mutex, *count_mutex, *santa, *elf_mutex, *elf_barrier,\
        *reindeer_mutex, *all_hitched, *santa_test;

int init_semaphores()
{
    p_num_mutex = sem_open("/p_num_mutex", O_CREAT, 0644, 1);
    count_mutex = sem_open("/count_mutex", O_CREAT, 0644, 1);
    santa = sem_open("/santa", O_CREAT, 0644, 0);
    elf_mutex = sem_open("/elf_mutex", O_CREAT, 0644, 1);
    elf_barrier = sem_open("/elf_barrier", O_CREAT, 0644, 0);
    reindeer_mutex = sem_open("/reindeer_mutex", O_CREAT, 0644, 0);
    all_hitched = sem_open("/all_hitched", O_CREAT, 0644, 0);
    santa_test = sem_open("/santa_test", O_CREAT, 0644, 0);

    return 0;
}

void delete_semaphores()
{
    sem_close(p_num_mutex);
    sem_close(santa);
    sem_close(count_mutex);
    sem_close(elf_mutex);
    sem_close(elf_barrier);
    sem_close(reindeer_mutex);
    sem_close(all_hitched);
    sem_close(santa_test);

    sem_unlink("/p_num_mutex");
    sem_unlink("/santa");
    sem_unlink("/count_mutex");
    sem_unlink("/elf_mutex");
    sem_unlink("/elf_barrier");
    sem_unlink("/reindeer_mutex");
    sem_unlink("/all_hitched");
    sem_unlink("/santa_test");
}

//-------------- SHARED VARIABLE FUNCTIONS ------------------

int create_shared(const char *name, u_int **var)
{
    //TODO fix if var was not created
    int fd = shm_open(name, O_RDWR|O_CREAT|O_TRUNC, 0644);
    ftruncate(fd, 4);
    *var = (u_int *)mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    return 0;
}

int initialize_shared(shared_t *sh_vars)
{
    create_shared("/process_count", &(sh_vars->pcount));
    create_shared("/elves_count",&(sh_vars->elves));
    create_shared("/reindeer_count", &(sh_vars->reindeers));
    create_shared("/workshop_closed", &(sh_vars->workshop_closed));

    return 0;
}

void delete_shared()
{
    shm_unlink("/process_count");
    shm_unlink("/elves_count");
    shm_unlink("/reindeer_count");
    shm_unlink("/workshop_closed");
}

//---------------- OTHER FUNCTIONS --------------------
int random_number(int min, int max)
{
    int number = rand() % (max-min+1);
    return number+min;
}

//--------------- PROCESS FUNCTIONS -------------------
void print_msg(FILE *fr, shared_t *sh_vars, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    sem_wait(p_num_mutex);
        u_int *count = sh_vars->pcount;
        (*count)++;
        fprintf(fr, "%d: ", *count);
        vfprintf(fr, msg, args);
        fflush(fr);
    sem_post(p_num_mutex);

    va_end(args);

}

void santa_function(FILE *fr, shared_t *sh_vars, args_t args)
{
    print_msg(fr, sh_vars, "Santa: going to sleep\n");
    sem_wait(count_mutex);
        (*sh_vars->workshop_closed) = 0;
    sem_post(count_mutex);

    while (1)
    {

        sem_wait(santa);
        sem_wait(count_mutex);
            if (*sh_vars->reindeers == args.NR)
            {
                print_msg(fr, sh_vars, "Santa: closing workshop\n");
                sem_post(elf_barrier);
                (*sh_vars->workshop_closed) = 1;
                (*sh_vars->reindeers) = 0;
                for (int i = 0; i < args.NR; i++)
                {
                    sem_post(reindeer_mutex);
                }
                sem_post(count_mutex);
                break;
            }
            else if (*sh_vars->elves == 3)
            {
                print_msg(fr, sh_vars, "Santa: helping elves\n");
                sem_post(elf_barrier);
                sem_post(elf_barrier);
                sem_post(elf_barrier);
                
                sem_post(count_mutex);
                
                sem_wait(santa_test);
                print_msg(fr, sh_vars, "Santa: going to sleep\n");

            }
    }

    sem_wait(all_hitched);
    print_msg(fr, sh_vars, "Santa: Christmas started\n");

}

void elf_function(FILE *fr, shared_t *sh_vars, int my_id, int max_time)
{
    print_msg(fr, sh_vars, "Elf %d: started\n", my_id);
    bool need_help = false;

    while (1)
    {
        //simulate work
        usleep(random_number(0,max_time)*1000);
        
        sem_wait(count_mutex);
        if (*sh_vars->workshop_closed == 1)
        {
            sem_post(count_mutex);
            break;
        }
        sem_post(count_mutex);

        sem_wait(elf_mutex);
        sem_wait(count_mutex);
            (*sh_vars->elves)++;
            print_msg(fr, sh_vars, "Elf %d: need help\n", my_id);
            need_help = true;
            if (*sh_vars->elves == 3)
            {
                sem_post(santa);
            }
            else
            {
                sem_post(elf_mutex);
            }
        sem_post(count_mutex);

        sem_wait(elf_barrier);

    
        sem_wait(count_mutex);
        if (*sh_vars->workshop_closed == 1)
        {
            sem_post(count_mutex);
            sem_post(elf_mutex);
            break;
        }
        sem_post(count_mutex);

        print_msg(fr, sh_vars, "Elf %d: get help\n", my_id);
        need_help = false;
    
        sem_wait(count_mutex);
            (*sh_vars->elves)--;
            if (*sh_vars->elves == 0)
            {
                sem_post(santa_test);
                sem_post(elf_mutex);
            }
        sem_post(count_mutex);

    }
    sem_post(count_mutex);

    if (need_help == false)
    {
        print_msg(fr, sh_vars, "Elf %d: need help\n", my_id);
    }
    
    print_msg(fr, sh_vars, "Elf %d: taking holidays\n", my_id);
    sem_post(elf_barrier);
}

void reindeer_function(FILE *fr, shared_t *sh_vars, int my_id, args_t args)
{
    print_msg(fr, sh_vars, "RD %d: rstarted\n", my_id);
    
    //simulate vacation
    usleep(random_number(args.TR/2,args.TR)*1000);
    
    sem_wait(count_mutex);
        (*sh_vars->reindeers)++;
        print_msg(fr, sh_vars, "RD %d: return home\n", my_id);
        if ((*sh_vars->reindeers) == args.NR)
        {
            sem_post(santa);
        }
    sem_post(count_mutex);

    sem_wait(reindeer_mutex);

    print_msg(fr, sh_vars, "RD %d: get hitched\n", my_id);    
    sem_wait(count_mutex);
        (*sh_vars->reindeers)++;
        if (*sh_vars->reindeers == args.NR)
        {
            sem_post(all_hitched);   
        }
    sem_post(count_mutex);
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
    
    //FILE *fr = stdout;

    //initialize semaphores
    //TODO fix failed creating
    init_semaphores();
    
    shared_t sh_vars;
    if (initialize_shared(&sh_vars))
    {
        delete_semaphores();
        fclose(fr);
        return 1;
    }


    //create one santa process
    pid_t pid = fork();

    if (pid == 0) //santa process
    {
        santa_function(fr , &sh_vars, args);
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

            elf_function(fr, &sh_vars, elf_id, args.TE);
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
            srand((int)time(0) % getpid());
            
            reindeer_function(fr, &sh_vars, rd_id, args);
            exit(1);
        }
        else if (pid == -1)
        {
            printf("Fork failed");
        }
    }

    while ((wpid = wait(&status)) > 0);

    delete_semaphores();
    delete_shared();
    
    fclose(fr);
    return 0;
}
