#ifndef GRPC_C_INTERNAL_THREAD_POOL_H
#define GRPC_C_INTERNAL_THREAD_POOL_H

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/sync.h>
#include <grpc/support/time.h>
#include <grpc/support/cpu.h>

#include "list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef void (*grpc_c_callback_func_t)(void *);

typedef struct grpc_c_thread_callback_s {
    grpc_c_list_t list;
    grpc_c_callback_func_t func;
    void * data;
}grpc_c_thread_callback_t;

typedef struct grpc_c_thread_s {
    grpc_c_list_t list;
    pthread_t tid;
}grpc_c_thread_t;

typedef struct grpc_c_thread_pool_s {

    int max_threads;
    int stop_threads;
    int shutdown;
    
    gpr_mu lock;
    gpr_cv cv;

    grpc_c_list_t callbacks_head;
    grpc_c_list_t threads_head;
} grpc_c_thread_pool_t;


grpc_c_thread_pool_t * grpc_c_thread_pool_create(int n);

int grpc_c_thread_pool_add(grpc_c_thread_pool_t *pool, grpc_c_callback_func_t func, void *arg);

void grpc_c_thread_pool_shutdown(grpc_c_thread_pool_t *pool);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* GRPC_C_INTERNAL_THREAD_POOL_H */
