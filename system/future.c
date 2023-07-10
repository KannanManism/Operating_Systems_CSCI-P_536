#include <xinu.h>
#include <future.h>
#include <string.h>

future_t* future_alloc(future_mode_t mode, uint size, uint nelem)
{
    intmask mask;
    mask = disable();
    future_t *f;
    if(f == SYSERR)
    {
        restore(mask);
        return SYSERR;
    }
    f= (future_t *) getmem((size * nelem) + sizeof(future_t));
    // f->data = getmem(sizeof(future_t));
    f->data = sizeof(future_t) + (char *)f;
    f->size = size;
    f->mode = mode;
    f->state = FUTURE_EMPTY;
    f->max_elems = nelem;
    f->count = 0;
    f->tail = 0;
    f->head = 0;
    if (mode == FUTURE_SHARED || mode == FUTURE_QUEUE)
    {
        f->get_queue = newqueue();
        f->set_queue = newqueue();
    }
    restore(mask);
    return f;
}

syscall future_get(future_t* f, char* out)
{
    intmask mask;
    mask = disable();
    if (f->mode == FUTURE_SHARED)
    {
        if(f->state == FUTURE_EMPTY)
        {
            dequeue(f->get_queue);
            f->state = FUTURE_WAITING;
            memcpy(out, (char *)f->data, sizeof(out));   
        }
        else if ( f->state == FUTURE_WAITING)
        {
            f->pid = getpid();
            f->state = FUTURE_WAITING; 
            resume(f->pid);
            while(!isempty(f->get_queue))
                dequeue(f->get_queue);
             memcpy(out, (char *)f->data, sizeof(out)); 
        }
        else if( f->state == FUTURE_READY )
        {
            memcpy(out, (char *)f->data, sizeof(out));  
            f->state = FUTURE_READY; 
        }
        restore(mask);
        return OK;
    }
    else if (f->mode == FUTURE_EXCLUSIVE)
    {
        if (f->state == FUTURE_READY)
        {
            memcpy(out, (char *)f->data, sizeof(out));
            f->state = FUTURE_EMPTY;
        }
        else if(f->state == FUTURE_WAITING)
        {
            restore(mask);
            return SYSERR;
        }
        else if(f->state == FUTURE_EMPTY)
        {
            f->state=FUTURE_WAITING;
            f->pid=getpid();
            suspend(f->pid);
            memcpy(out, (char *)f->data, sizeof(out));
        } 
        restore(mask);
        return OK;  
    }
    else if (f->mode == FUTURE_QUEUE)
    {
        if(f->state == FUTURE_EMPTY)
        {
            // printf("Entered get_empty");
            f->state = FUTURE_WAITING;
            enqueue(getpid(), f->get_queue);
            suspend(getpid());
            // printf("Entered get_empty resumed");
            char *head_dp = f->data +(f->head * f->size);
            memcpy(out, head_dp, f->size);
            f->head = (f->head + 1) % f->max_elems;
            f->count = f->count - 1;
            resume(dequeue(f->set_queue)); 
            restore(mask);
            return OK;
        }
        else if(f->state == FUTURE_WAITING)
        {
            enqueue(getpid(), f->get_queue);
            suspend(getpid()); 
            // char *head_dp = f->data +(f->head * f->size);
            // memcpy(out, head_dp, f->size);
            // f->count = f->count - 1;
            // f->head = (f->head + 1) % f->max_elems;
            // resume(dequeue(f->set_queue)); 
            // restore(mask);
            // return OK; 
        }
        
        if(f->state == FUTURE_READY)
        {
            if(f->count == 0)
            {
                enqueue(getpid(), f->get_queue);
                suspend(getpid()); 
            }
            char *head_dp = f->data +(f->head * f->size);
            memcpy(out, head_dp, f->size);
            f->head = (f->head + 1) % f->max_elems;
            f->count = f->count - 1;
            resume(dequeue(f->set_queue));
            restore(mask);
            return OK;
        }
    }
}

syscall future_set(future_t* f, char* in)
{
    intmask mask;
    mask = disable();

    if( f->mode == FUTURE_SHARED )
    {
        if ( f->state == FUTURE_EMPTY)
        {
                enqueue(f->pid,f->get_queue);
                memcpy((char *)f->data, in, sizeof(in));
                f->state = FUTURE_READY;
        }
        else if(f->state == FUTURE_WAITING)
        { 
                enqueue(f->pid,f->get_queue);
                memcpy((char *)f->data, in, sizeof(in));
                f->state = FUTURE_READY;
                suspend(f->pid);
        }
        else if (f->state == FUTURE_READY)
        {
            restore(mask);
            return SYSERR;
        }
        restore(mask);
        return OK;
    }
    else if (f->mode == FUTURE_EXCLUSIVE)
    {
        if (f->state == FUTURE_EMPTY)
        {
            memcpy((char *)f->data, in, sizeof(in));
            f->state = FUTURE_READY;
        }
        else if (f->state == FUTURE_READY)
        {
            restore(mask);
            return SYSERR;
        }   
        else if (f->state == FUTURE_WAITING)
        {
            memcpy((char *)f->data, in, sizeof(in));
            f->state = FUTURE_EMPTY;
            resume(f->pid);
        }
        restore(mask);
        return OK;
    }
    else if (f->mode == FUTURE_QUEUE)
    {
        if(f->count == f->max_elems)
        {
            // printf("Entered set 0");
            enqueue(getpid(), f->set_queue);
            suspend(getpid());
            // printf("Entered set resume");
        }
        // printf("Entered set 1");
        f->state = FUTURE_READY;
        char *tail_dp = f->data + (f->tail * f->size);
        memcpy(tail_dp, in, f->size);
        f->tail = (f->tail + 1) % f->max_elems;
        f->count = f->count + 1;
        resume(dequeue(f->get_queue));
        restore(mask);
        return OK;
    }
}

syscall future_free(future_t* f)
{
    intmask mask;
    mask = disable();
    if (f->mode == FUTURE_EXCLUSIVE)
    {
        if (f->state == FUTURE_WAITING)
        {
            f->pid=getpid();
            freemem(f->data, f->size);
            freemem((char *)f, sizeof(future_t));
            kill(f->pid);

        }
        else if(f->state == FUTURE_READY || f == SYSERR)
        {
            restore(mask);
            return SYSERR;
        }
        else
        {    
            f->state = FUTURE_EMPTY;
            f->pid=getpid();
            freemem(f->data, f->size);
            freemem((char *)f, sizeof(future_t));
            restore(mask);
            return OK;
        }
    }
    else if(f->mode == FUTURE_SHARED)
    {
        f->state = FUTURE_EMPTY;
        f->pid=getpid();
        delqueue(f->get_queue);
        freemem(f->data, f->size);
        freemem((char *)f, sizeof(future_t));
    }
    else if(f->mode == FUTURE_QUEUE)
    {
        // freemem(f->data, sizeof(future_t));
        freemem(f, sizeof(future_t) + f->size);
    }
    restore(mask);
    return OK;
}