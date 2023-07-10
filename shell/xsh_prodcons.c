#include <xinu.h>
#include <prodcons.h>

//Assignment 2
int n=0;
//Assignment 3
sid32 can_read, can_write, prodcons_calling;
sid32 connecting_run_prodcons;
// sid32 can_write;
int producer_id, consumer_id;

shellcmd xsh_prodcons(int nargs, char *args[]){
	//For Assignment 2, count was set to 2000.

	//For Assignment 3, count was modified to 200.
	int count = 200;
	can_write=semcreate(0); // produced_semaphore
	can_read=semcreate(1); // consumed_semaphore
	prodcons_calling=semcreate(0);
    //Assignment 2
	// if(nargs> 2)
	// 	printf("Too many arguments!!\n");
	// else if(nargs<2)
	// 	printf("Need atleast one argument!!\n");
	// else if(nargs==2){
	// //count=(int)(args[1]);
	// count = atoi(args[1]);
	// }
	//printf("Value of count is %s\n", count);
	
	
	if (nargs>2){
		printf("Syntax: run prodcons [counter]\n");
	}
	else{
		if(args[1]){
			count = atoi(args[1]);
		}
		// assigning high number to priority mean thats given more preferencee
		
		producer_id=create(producer, 1024, 2, "producer", 20, count);
		consumer_id=create(consumer, 1024, 2, "consumer", 20, count);
		
		resume(producer_id);
		resume(consumer_id);
		
		wait(prodcons_calling);
		signal(connecting_run_prodcons);
	}
	
	
	// signal(can_write);
	// If semdelete done here, then order is changed into producing fully and then to consuming
	// semdelete(can_read);
	// semdelete(can_write);
	
	return 0;
}
