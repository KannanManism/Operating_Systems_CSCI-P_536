#include<xinu.h>
/* Global variable for producer consumer */
extern int n; /* this is just declaration */

// Creation of semaphores
extern sid32 can_read; // consumer semaphore
extern sid32 can_write; // producer semaphore
extern sid32 prodcons_calling; //to close "Enter" issue
// extern sid32 connecting_run_prodcons_hello; 
extern int producer_id, consumer_id;
/* Function Prototype */
void consumer(int count);
void producer(int count);
