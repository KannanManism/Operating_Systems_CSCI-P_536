#ifndef _FUTURE_H_
#define _FUTURE_H_

typedef enum
{
    FUTURE_EMPTY, // inital future state after allocation "future_alloc()"
    FUTURE_WAITING, // future process waiting for a value to be assigned
    FUTURE_READY // future holds value assigned to it by "future_set()""
} future_state_t;

typedef enum
{
    FUTURE_EXCLUSIVE, //Assignment 5
    FUTURE_SHARED, // Assignment 6
    FUTURE_QUEUE    // Assignment 8
} future_mode_t;

typedef struct future_t
{
    char *data; // data pointer held by "future"
    uint size;
    future_state_t state;
    future_mode_t mode; // mode should be defined with future_alloc()
    qid16 get_queue; // Assignment_6: queue holding pid for multiple getter process
    pid32 pid; // thread process id waiting for value by calling "future_get()"
    qid16 set_queue; // Assignment 8: FUTURE_QUEUES, multiple setter operation
    uint16 max_elems; // Assignment 8: holds maximum number of elements that can be stored in a queue
    uint16 count; // Assignment 8: holds the current count of elements in a queue
                  // queue is full if count == max_elems;
                  // queue is empty if count == 0;
    uint16 head; // Assignment 8: index for consuming a new element
    uint16 tail; // Assignment 8: index for producing a new element
} future_t;

future_t* future_alloc(
    future_mode_t mode, // mode of the future
    uint size, // size of data in bytes
    uint nelem // number of elements (required in FUTURE_QUEUE)
    // return SYSERR or future pointer
    );

int future_fib(int nargs, char *args[]);
int future_free_test(int nargs, char *args[]);


syscall future_free(future_t* f); //frees allocated future
syscall future_get(future_t* f, char* out); //change state of future; get future value if set
syscall future_set(future_t* f, char* in); // sets the future value; change future state
void stream_consumer_future(int32 id, future_t *f);
int stream_proc_futures(int nargs, char* args[]);
#endif // _FUTURE_H