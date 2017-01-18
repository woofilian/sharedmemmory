#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sched.h>

typedef struct th_tcb_s {
    char name[16+1];
    void *(*func)(void *parm);
    void *parm;
}th_tcb_t;

static void *th_func (void *parm)
{
    void *ret = NULL;
    th_tcb_t *tcb = (th_tcb_t*)parm;
    
    prctl (PR_SET_NAME, (unsigned long)tcb->name);
    
    if(tcb->func)
    {
        ret = tcb->func(tcb->parm);
    }
    
    free(tcb);
    return ret;
}

int th_new (pthread_t *tid
    , char *name
    , pthread_attr_t *attr
    , void *(*func)(void *)
    , void *parm)
{
    th_tcb_t *tcb = (th_tcb_t*)calloc(1, sizeof(th_tcb_t));
    
    if(tcb == NULL) return -1;
        
    if(name != NULL) strncpy(tcb->name, name, 16);
    
    tcb->func = func;
    tcb->parm = parm;
    
    if(pthread_create(tid, attr, th_func, tcb) < 0)
    {
        free(tcb);
        return -1;
    };
    return 0;
}
