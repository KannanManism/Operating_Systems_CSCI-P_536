#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void producer(int count) {
  // TODO: implement the following:
  // - Iterates from 0 to count (count including)
  //   - setting the value of the global variable 'n' each time
  //   - print produced value (new value of 'n'), e.g.: "produced : 8"
  
  for(int i=0; i<=count; i++)
  {
    wait(can_read); // consumed_semaphore
    n=i;
    printf("produced : %d\n", n); 
    signal(can_write); // produced_semaphore
  }

  //semdelete(can_write);
  // kill(producer_id);
  // return INITRET;
}

void producer_bb(int id, int iterations)
{
  // int loop=iteration*id;
  //printf("Entered producer_bb: %d", loop);
  
   for(int i=0; i<(iterations*id); i++)
  {
    
    // for(int j=0; j<id; j++)
    // {    
      wait(consumed); 
      wait(mutex);     
      if(!is_full())
      {
        if(front == -1)
        {
          front = 0;
        }
        rear = (rear + 1) % 5;
        // int flag=1;
        // if((i) == (iterations))
        // {
        //   flag=0;
        // }
        // if(flag == 1)
        // {
        //   arr_q[rear] = i % iterations;
        // }
        arr_q[rear] = i % iterations;
      }
      printf("name : producer_%d, write : %d\n",(i%id), arr_q[rear]);
      signal(mutex);
      signal(produced);     
    // }
    
  } 
  
}

int is_full()
{
  if( (front == rear+1) || ((front == 0) && (rear == 5-1)) )
  {
    return 1;
  }
  return 0;
}