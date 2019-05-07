#include <grpc-c/grpc-c.h>
#include "thread_pool.h"
#include "trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


void * grpc_c_thread_body(void * arg)
{
	grpc_c_thread_callback_t * callback;
	grpc_c_thread_pool_t *pool = (grpc_c_thread_pool_t *)arg;

	for(;;)
	{
		gpr_mu_lock(&pool->lock);
		gpr_cv_wait(&pool->cv, &pool->lock, gpr_inf_future(GPR_CLOCK_REALTIME));
		if ( pool->shutdown  ) {
			pool->stop_threads++;
			gpr_mu_unlock(&pool->lock);
			break;
		}

		if ( GRPC_LIST_EMPTY(&pool->callbacks_head) ) {
			gpr_mu_unlock(&pool->lock);
			continue;
		}

		callback = (grpc_c_thread_callback_t *)pool->callbacks_head.next;
		GRPC_LIST_REMOVE(&callback->list);
		
		gpr_mu_unlock(&pool->lock);

		callback->func(callback->data);
		grpc_free(callback);
	}

	return NULL;
}

grpc_c_thread_t *grpc_c_thread_new(void * arg)
{
	int ret;
	grpc_c_thread_t * thread;
	pthread_attr_t attr;

	thread = (grpc_c_thread_t *)grpc_malloc(sizeof(grpc_c_thread_t));
	if ( NULL == thread ) {
		GRPC_C_ERR("Failed to allocate memory for thread!");
		return NULL;
	}

	(void)pthread_attr_init(&attr);
	(void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	ret = pthread_create(&thread->tid, &attr, &grpc_c_thread_body, arg);
	if ( ret != 0 ) {
		GRPC_C_ERR("Failed to create thread!");
		return NULL;
	}

	return thread;
}


/*
 * Create a structure to hold details about pool of threads. Takes the maximum
 * number of threads in pool as argument
 */
grpc_c_thread_pool_t * grpc_c_thread_pool_create(int n)
{
	int i;
	grpc_c_thread_t * thread;
    grpc_c_thread_pool_t *pool = gpr_malloc(sizeof(grpc_c_thread_pool_t));
    if (pool == NULL) {
		GRPC_C_ERR("Failed to allocate memory for thread pool");
		return NULL;
    } 
	
	memset(pool, 0, sizeof(grpc_c_thread_pool_t));
	
	pool->max_threads  = n;
	gpr_mu_init(&pool->lock);
	gpr_cv_init(&pool->cv);

	GRPC_LIST_INIT(&pool->callbacks_head);
	GRPC_LIST_INIT(&pool->threads_head);

	for( i = 0 ; i < n; i++ ) {
		thread = grpc_c_thread_new((void *)pool);
		if ( NULL == thread ) {
			return NULL;
		}
		GRPC_LIST_ADD(&thread->list, &pool->threads_head);
	}

    return pool;
}

/*
 * Adds a new job to the pool of threads. Creates one if necessary
 */
int grpc_c_thread_pool_add(grpc_c_thread_pool_t *pool, grpc_c_callback_func_t func, void *arg)
{
    grpc_c_thread_callback_t *callback;

    if (pool == NULL) {
		GRPC_C_ERR("Uninitialized pool");
		return 1;
    }

    callback = grpc_malloc(sizeof(grpc_c_thread_callback_t));
    if (callback == NULL) {
		GRPC_C_ERR("Failed to allocate memory for thread callback");
		return 1;
    }

    callback->func  = func;
    callback->data  = arg;

    /*
     * Add callback function and arguments to the queue
     */
    gpr_mu_lock(&pool->lock);
	GRPC_LIST_ADD_BEFORE(&callback->list, &pool->callbacks_head);
	gpr_cv_signal(&pool->cv);
    gpr_mu_unlock(&pool->lock);

    return 0;
}

/*
 * Shutdown thread pool
 */
void grpc_c_thread_pool_shutdown(grpc_c_thread_pool_t *pool)
{
	int i;
	grpc_c_list_t * item;
	
    if (!pool) {
		return;
    }

    gpr_mu_lock(&pool->lock);
    pool->shutdown  = 1;
    gpr_cv_broadcast(&pool->cv);
	gpr_mu_unlock(&pool->lock);

	GRPC_LIST_TRAVERSAL(item, &pool->threads_head )
	{
		grpc_c_thread_t * thread;
		thread = GRPC_LIST_OFFSET(item, grpc_c_thread_t, list);
		pthread_join(thread->tid, NULL);
		grpc_free(thread);
	}

	GRPC_LIST_TRAVERSAL(item, &pool->callbacks_head )
	{
		grpc_c_thread_callback_t * callback;
		callback = GRPC_LIST_OFFSET(item, grpc_c_thread_callback_t, list);
		grpc_free(callback);
	}

	grpc_free(pool);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */



