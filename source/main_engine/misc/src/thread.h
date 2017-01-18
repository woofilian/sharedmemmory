#ifndef __thread__
#define __thread__

#include <pthread.h>

extern int th_new (pthread_t *tid
        , char *name
        , pthread_attr_t *attr
        , void *(*func)(void *)
        , void *parm);

#define pthread_create(tid, name, attr, func, parm)  th_new(tid, name, attr, func, parm)


#endif //__thread__
