#include <xinu.h>
#include <shprototypes.h>
#include <prodcons.h>
#include <prodcons_bb.h>
#include <future.h>
#include <future_prodcons.h>
#include <stream.h>

// make clean && make && make qemu

extern sid32 connecting_run_prodcons;
extern sid32 connecting_run_hello, connecting_run_futest;

int future_fib(int nargs, char *args[]);
int future_free_test(int nargs, char *args[]);

shellcmd xsh_run(int nargs, char *args[]){
    // Print list of available functions
	if((nargs==1)||(strncmp(args[1], "list", 4) == 0)){
        printf("futest\n");
        printf("hello\n");
        printf("list\n");
        printf("prodcons\n");
        printf("prodcons_bb\n");
        printf("tscdf\n");
        return -1;
	}

    /* This will go past "run" and pass the function/process name and its arguments */
    args++;
    nargs--;

    // In case of running functions with its arguments
    if(strncmp(args[0], "hello", 5) == 0){
        
        connecting_run_hello=semcreate(0);
        resume(create((void *)xsh_hello, 4096, 20, "hello", 2, nargs, args ));
        wait(connecting_run_hello);
        //INITRET is a constant variable defined in "process.h", which helps to remove the top-level-function from stack
        // but do not "#include <process.h>", which produces errors on process.h file.
        //INITRET should be returned here because process gets removed from stack when the top-level-function return INITRET - related to usernet.c
        // It kills the process
        // return INITRET;
    }
    else if(strncmp(args[0], "prodcons_bb", 11) == 0)
    {
        connecting_run_prodcons=semcreate(0);
        resume(create((void*)xsh_prodcons_bb,4096,20,"prodcons_bb", 5, nargs, args));
        wait(connecting_run_prodcons);
    }
    else if(strncmp(args[0], "prodcons", 8) == 0)
    {
        
        connecting_run_prodcons=semcreate(0);
        resume(create((void *)xsh_prodcons, 4096, 20, "prodcons", 2, nargs, args));
        wait(connecting_run_prodcons);
        //INITRET is a constant variable defined in "process.h", which helps to remove the top-level-function from stack
        // return INITRET;
    }
    else if(strncmp(args[0],"futest", 6)==0){
        if( nargs > 3 && (strcmp(args[1],"-pcq") == 0) && atoi(args[2]))
        {
            resume(create((void *) xsh_futest, 4096, 20, "future_q_prodcons", 2,nargs, args));
            wait(connecting_run_futest);
        }
        else if(strcmp(args[1],"-pc")==0 && !atoi(args[2]))
        {
            resume(create((void *) xsh_futest, 4096, 20, "future_prodcons", 2,nargs, args));
            wait(connecting_run_futest);
        }
        else if( nargs == 2 && strncmp(args[1],"--free", 6) == 0)
            future_free_test(nargs,args);
        else if(nargs == 3 && strncmp(args[1],"-f", 2) == 0 && atoi(args[2]) )
            future_fib(nargs,args);
        else
        {
            printf("Syntax: run futest [-pc [g ...] [s VALUE ...]] | [-pcq LENGTH [g ...] [s VALUE ...]] | [-f NUMBER] | [--free]\n");
            return -1; 
        } 
        
    }
    // else if(strcmp(args[0],"tscdf")==0)
    //     stream_proc(nargs,args);
    // else if(strcmp(args[0],"tscdf_fq")==0)
    //     stream_proc_futures(nargs,args);
    else if(strcmp(args[0], "fstest")==0)
        fstest(nargs,args);
    else{
        printf("futest\n");
        printf("hello\n");
        printf("list\n");
        printf("prodcons\n");
        printf("prodcons_bb\n");
        printf("tscdf\n");
        printf("tscdf_fq\n");
        return -1;
    }

    return 0;
}
