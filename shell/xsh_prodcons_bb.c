#include <xinu.h>
#include <prodcons_bb.h>

int arr_q[5];

sid32 consumed, produced, prodcons_calling, connecting_run_prodcons, mutex;
int front=-1, rear=-1;

shellcmd xsh_prodcons_bb(int nargs, char *args[])
{
    int no_of_producer_process, no_of_consumer_process, producer_iteration, consumer_iteration;
    
    
    if ( (nargs>5) || (nargs<5) )
    {
        printf("Syntax: run prodcons_bb <# of producer processes> <# of consumer processes> <# of iterations the producer runs> <# of iterations the consumer runs>\n");
        signal(connecting_run_prodcons);
        return 1;
    }
	else
    {
		if( args[1] && args[2] && args[3] && args[4] )
        {
			no_of_producer_process = atoi(args[1]);
            no_of_consumer_process = atoi(args[2]);
            producer_iteration = atoi(args[3]);
            consumer_iteration = atoi(args[4]);
            // printf("%d, %d, %d, %d", no_of_producer_process, no_of_consumer_process, producer_iteration, consumer_iteration);
            if( (no_of_producer_process * producer_iteration) == (no_of_consumer_process * consumer_iteration) )
            {
                consumed=semcreate(1);
                produced=semcreate(0);
                prodcons_calling=semcreate(0);
                mutex=semcreate(1);
                resume(create(producer_bb, 1024, 2, "producer_bb", 20, no_of_producer_process, producer_iteration));
                resume(create(consumer_bb, 1024, 2, "consumer_bb", 20, no_of_consumer_process, consumer_iteration));
                
                wait(prodcons_calling);
                signal(connecting_run_prodcons);        
            }
            else
            {
                printf("Iteration Mismatch Error: the number of producer(s) iteration does not match the consumer(s) iteration\n");
                signal(connecting_run_prodcons);
                return 1;
            }
		}
        
		// semdelete(consumed);
        // semdelete(produced);
        // semdelete(prodcons_calling);
        // semdelete(mutex);
	} 
    return 0; 
}
