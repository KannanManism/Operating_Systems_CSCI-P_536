
// #include <xinu.h>
// #include <stream.h>
// #include "tscdf.h"
// #include "tscdf_input.h"

// /**
//  * Code Referred from
//  * https://github.com/devanshjain14/xinu-operating-system/blob/a7/apps/stream_proc.c
//  */

// int num_streams, work_queue_depth, time_window, output_time, port_id;
// // extern const char *stream_input[];

// int32 stream_proc(int nargs, char *args[])
// {
    
//     // TODO: Parse arguments
//     char usage[] = "Usage: run tscdf -s <num_streams> -w <work_queue_depth> -t <time_window> -o <output_time>\n";
//     int i, v, st, ts;
//     char *ch, c, *a;
//     str **s;
//     ulong secs=clktime, msecs=clkticks;
//     if (nargs != 9)
//     {
//         printf("%s", usage);
//         return SYSERR;
//     } else 
//     {
//         i = nargs - 1;
//         while (i > 0) 
//         {
//             ch = args[i - 1];
//             c = *(++ch);

//             switch (c) 
//             {
//                 case 's':
//                     num_streams = atoi(args[i]);
//                     break;

//                 case 'w':
//                     work_queue_depth = atoi(args[i]);
//                     break;

//                 case 't':
//                     time_window = atoi(args[i]);
//                     break;

//                 case 'o':
//                     output_time = atoi(args[i]);
//                     break;

//                 default:
//                     printf("%s", usage);
//                     return SYSERR;
//             }

//             i -= 2;
//         }
//     }

//     port_id = ptcreate(num_streams);
//     s = getmem(sizeof(str *) * num_streams);
//     for (int i = 0; i < num_streams; i++)
//     {
//         s[i] = getmem(sizeof(str) + (sizeof(de) * work_queue_depth));
//         s[i]->head = 0;
//         s[i]->tail = 0;
//         s[i]->mutex = semcreate(1);
//         s[i]->spaces = semcreate(0);
//         s[i]->items = semcreate(work_queue_depth);
//         s[i]->queue = getmem(sizeof(de) * work_queue_depth);
//     }
//     for (int j = 0; j < num_streams; j++)
//     {
//         resume(create((void *)stream_consumer, 4096, 20,\
//          "stream_consumer", 2, j, s[j]));
//     }
//     for (int i = 0; i < number_inputs; i++)
//     {
//         a = (char *)stream_input[i];
//         st = atoi(a);
//         while (*a++ != '\t');
//         ts = atoi(a);
//         while (*a++ != '\t');
//         v = atoi(a);
//         wait(s[st]->items);
//         wait(s[st]->mutex);
//         s[st]->queue[s[st]->tail].time = ts;
//         s[st]->queue[s[st]->tail].value = v;
//         s[st]->tail += 1;
//         s[st]->tail %= work_queue_depth;
//         signal(s[st]->mutex);
//         signal(s[st]->spaces);
//     }

//     for (int i = 0; i < num_streams; i++)
//         printf("process %d exited\n", ptrecv(port_id));

//     ptdelete(port_id, 0);
//     ulong time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
//     kprintf("time in ms: %u\n", time);
//     return 0;
// }

// void stream_consumer(int32 id, struct stream *str)
// {
//     int temp = output_time;
//     kprintf("stream_consumer id:%d (pid:%d)\n", id, getpid());
//     struct tscdf *tc = tscdf_init(time_window);
//     for(;;)
//     {
//         wait(str->spaces);
//         wait(str->mutex);
//         if (str->queue[str->head].time == 0 && str->queue[str->head].value == 0)
//         {
//             kprintf("stream_consumer exiting\n");
//             break;
//         }
//         tscdf_update(tc, str->queue[str->head].time, str->queue[str->head].value);

//         if (temp-- == 1)
//         {
//             temp = output_time;
//             char output[25];
//             int *qarray = tscdf_quartiles(tc);

//             if (qarray == NULL) {
//             kprintf("tscdf_quartiles returned NULL\n");
//             continue;
//             }

//             sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
//             kprintf("%s\n", output);
//             freemem((char *) qarray, (6*sizeof(int32)));
//         }
//         str->head += 1;
//         str->head %= work_queue_depth;
//         signal(str->mutex);
//         signal(str->items);
//     }
//     tscdf_free(tc);
//     ptsend(port_id, getpid());
// }