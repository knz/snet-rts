#ifndef _TASK_H_
#define _TASK_H_

#include <stdio.h>
#include <pthread.h>
#include <pcl.h>    /* tasks are executed in user-space with help of
                       GNU Portable Coroutine Library  */

#include "scheduler.h"
#include "timing.h"
#include "atomic.h"


/**
 * If a stacksize attribute <= 0 is specified,
 * use the default stacksize
 */
#define TASK_STACKSIZE_DEFAULT  8192  /* 8k stacksize*/



#define TASK_ATTR_DEFAULT      (0)
#define TASK_ATTR_MONITOR   (1<<0)
#define TASK_ATTR_SYSTEM    (1<<8)


#define TASK_PRINT_TIMES    (1<<0)
#define TASK_PRINT_STREAMS  (1<<1)


/**
 * Check if a task is a waitany-task
 * @param t   pointer to task_t
 */
#define TASK_IS_WAITANY(t)  (BIT_IS_SET((t)->attr.flags, TASK_ATTR_WAITANY))


struct stream_desc;
struct stream;


typedef enum {
  TASK_INIT    = 'I',
  TASK_RUNNING = 'U',
  TASK_READY   = 'R',
  TASK_WAITING = 'W',
  TASK_ZOMBIE  = 'Z'
} taskstate_t;

typedef enum {
  WAIT_ON_READ  = 'r',
  WAIT_ON_WRITE = 'w',
  WAIT_ON_ANY   = 'a'
} taskstate_wait_t;


typedef struct task task_t;

typedef void (*taskfunc_t)(task_t *self, void *inarg);

typedef struct {
  int flags;
  int stacksize;
} taskattr_t;

/*
 * TASK CONTROL BLOCK
 */
struct task {
  unsigned long uid;
  taskstate_t state;
  /* queue handling: prev, next */
  //task_t *volatile prev;
  //task_t *volatile next;
  task_t *prev, *next;

  /* lock */
  pthread_mutex_t lock;
  
  /* attributes */
  taskattr_t attr;

  /* data to indicate on which event the task is waiting */
  taskstate_wait_t wait_on;

  /* poll token */
  atomic_t poll_token;
  struct stream_desc *wakeup_sd;

  /* reference counter */
  atomic_t refcnt;

  schedctx_t *sched_context;
  void *sched_info;  /* scheduling information for this task */

  /* ACCOUNTING INFORMATION */
  /* timestamps for creation, start/stop of last dispatch */
  struct {
    timing_t creat, start, stop;
  } times;
  /* dispatch counter */
  unsigned long cnt_dispatch;
  /* streams marked as dirty */
  struct stream_desc *dirty_list;

  /* CODE */
  coroutine_t ctx;
  taskfunc_t code;
  void *inarg;  /* input argument  */
  void *outarg; /* output argument */
};



extern task_t *TaskCreate( taskfunc_t, void *inarg, taskattr_t *attr);
extern int TaskDestroy(task_t *t);

extern void TaskCall(task_t *ct);
extern void TaskExit(task_t *ct, void *outarg);
extern void TaskYield(task_t *ct);


extern void TaskPrint( task_t *t, FILE *file, int flags);

#endif /* _TASK_H_ */