#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void consumer(int count) {
  // TODO: implement the following:
  // - Iterates from 0 to count (count including)
  //   - reading the value of the global variable 'n' each time
  //   - print consumed value (the value of 'n'), e.g. "consumed : 8"

	for(int i=0;i<=count;i++)
	{
		wait(can_write); // produced_semaphore
		//n=i;
		printf("consumed : %d\n", n);
		signal(can_read); // consumed_semaphore
	}
	// Semaphores don't need to be deleted here
	// [wrong] Both semaphores are deleted here because semdelete() also send the signal() for wait() process
	//semdelete(can_read);
	// Not deleting "can_write" semaphore changed the order into consuming and producing
	// semdelete(can_write);
	//kill(consumer_id);
  	// return INITRET;
	//   kill(producer_id);
	//   kill(consumer_id);
	signal(prodcons_calling);
	  //signal(connecting_run_prodcons_hello);

}

void consumer_bb(int id, int iterations)
{
	// int loop=iterations*id;
	//printf("Entered consumer_bb: %d", loop);
	
	for(int i=0; i<(iterations*id); i++)
	{
		
		// for(int j=0;j<id; j++)
		// {
			wait(produced);
			wait(mutex);
			
			if(!is_empty())
			{
				// int flag=1;
				// if((i) == (iterations))
				// {
				// 	flag=0;
				// }
				// if(flag == 1)
				// {
					printf("name : consumer_%d, read : %d\n",(i%id), arr_q[front]);
					if(front == rear)
					{
						front = -1;
						rear = -1;
					}
					else
					{
						front = (front + 1) % 5;
					}
				// }
				
				
			}		
			signal(mutex);
			signal(consumed);
		// }
			
	}	
	
  signal(prodcons_calling);
}

int is_empty()
{
  if(front==-1)
  {
    return 1;
  }
  return 0;
}