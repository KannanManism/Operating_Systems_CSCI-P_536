#include <xinu.h>
// including <stdlib.h> for atoi()
#include <stdlib.h>
#include <future_prodcons.h>

sid32 print_sem, semaphore, connecting_run_futest, future_prodcons_calling,consumed, produced, mutex;

shellcmd xsh_futest(int nargs, char *args[])
{
    print_sem = semcreate(1);future_prodcons_calling=semcreate(1);
    future_t* f;
    char *val; 
    int max_elems; // Length of future_data_queue

    // i=0 is futest
    // i=1 is -pc or -pcq
    // i=2 is max_elems
    int i = 2;

    // checking argument syntax and counting arguments
    while (i < nargs)
    {
        if(strncmp(args[i],"g", 1) == 0)
        {
            i++;continue;
        } 
        else if(strncmp(args[i],"s", 1) == 0)
        {
            i++;continue;
        } 
        else if (atoi(args[i]))
        {
            i++;continue;
        } 
        else
        {
            printf("Syntax: run futest [-pc [g ...] [s VALUE ...]] | [-pcq LENGTH [g ...] [s VALUE ...]] | [-f NUMBER] | [--free]\n");
            signal(connecting_run_futest);
            return -1;
        }
        i++;
    }
    int num_args = i;
    i = 2;
    val = (char *) getmem(num_args);
    if(strncmp(args[1],"-pcq",4) == 0 && atoi(args[2]))
    {
        max_elems = atoi(args[2]);
        // printf("Max elements: %d\n",max_elems);
        f = future_alloc(FUTURE_QUEUE, sizeof(int), max_elems);
    }
    else if(strncmp(args[1],"-pc",3) == 0)
    {
        f = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);
    }

    while (i < nargs)
    {
        if (strcmp(args[i], "g")==0)
        {
            char id[10];
            sprintf(id, "fcons%d",i);
            resume(create(future_cons, 2048, 20, id, 1, f));
        }
        if (strcmp(args[i], "s") == 0)
        {
            char id[10];
            sprintf(id, "fprod%d", i++);
            uint8 number = atoi(args[i]);
            val[i] = number;
            resume(create(future_prod, 2048, 20, id, 2, f, &val[i]));
            sleepms(5);
        }
        i++;
    } 
    signal(connecting_run_futest);
    sleepms(100);
    future_free(f);
}