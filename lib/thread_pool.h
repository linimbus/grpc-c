/*
 * Copyright (c) 2016, Juniper Networks, Inc.
 * All rights reserved.
 */

#ifndef GRPC_C_INTERNAL_THREAD_POOL_H
#define GRPC_C_INTERNAL_THREAD_POOL_H

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/queue.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/sync.h>
#include <grpc/support/time.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


typedef uintptr_t gpr_thd_id;

/* Thread creation options. */
typedef struct {
  int flags; /* Opaque field. Get and set with accessors below. */
} gpr_thd_options;

typedef void (*grpc_c_callback_func_t)(void *);

/*
 * Structure definition for thread callbacks
 */
struct grpc_c_thread_callback_t {
    grpc_c_callback_func_t *gctc_func;	/* Pointer to the callback function
					   that needs to be called from this
					   thread */
    void *gctc_arg;			/* Pointer to data to be passed to the
					   callback */
    TAILQ_ENTRY(grpc_c_thread_callback_t) gctc_callbacks;
					/* Queue of callbacks */
};

/*
 * Structure to hold thread details
 */
struct grpc_c_thread_t {
    gpr_thd_id gct_thread;
    gpr_mu gct_lock;
    grpc_c_thread_pool_t *gct_pool;
    TAILQ_ENTRY(grpc_c_thread_t) gct_threads;/* Queue of threads */
};

/*
 * Structure holding details about pool of threads
 */
typedef struct grpc_c_thread_pool_s {
    int gctp_max_threads;	/* Maximum number of threads in this pool */
    int gctp_wait_threads;	/* Number of waiting threads */
    int gctp_nthreads;		/* Number of threads created */
    int gctp_shutdown;		/* Boolean to tell if we are shutting down */
    gpr_mu gctp_lock;		/* Lock for pool access */
    gpr_cv gctp_cv;		/* Condition variable to wait on */
    gpr_cv gctp_shutdown_cv;
				/* Condition variable to wait on for thread
				 * shutdown message */
    TAILQ_HEAD(grpc_c_thread_callback_queue_head, grpc_c_thread_callback_t)
	gctp_callbacks_head;	/* Head pointer for callback queue */
    TAILQ_HEAD(grpc_c_thread_queue_head, grpc_c_thread_t) gctp_dead_threads;
				/* List of finished threads to join */
} grpc_c_thread_pool_t;

/*
 * Creates and initializes a thread pool of size n
 */
grpc_c_thread_pool_t * grpc_c_thread_pool_create(int n);

/*
 * Adds a new job to pool of threads
 */
int grpc_c_thread_pool_add(grpc_c_thread_pool_t *pool, grpc_c_callback_func_t *func, void *arg);

/*
 * Shuts down and deletes pool of threads
 */
void grpc_c_thread_pool_shutdown(grpc_c_thread_pool_t *pool);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* GRPC_C_INTERNAL_THREAD_POOL_H */
