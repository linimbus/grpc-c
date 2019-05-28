
#include <stdio.h>
#include <stdlib.h>

#include <grpc-c/grpc-c.h>

#include "context.h"
#include "stream_ops.h"
#include "thread_pool.h"
#include "trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int grpc_c_register_method_ready(grpc_c_server_t *server, grpc_c_method_t *np);


void grpc_c_server_call_start(void *arg) {
    int ret;
    grpc_c_recv_close_t * recv_close;
    grpc_c_context_t *context = (grpc_c_context_t *)arg;

    context->state = GRPC_C_STATE_RUN;

    recv_close = grpc_c_server_recv_close_init();
    if ( recv_close == NULL ) {
        goto failed;
    }
    
    ret = grpc_c_server_recv_close(context->call, recv_close);
    if ( ret != GRPC_C_OK ) {
        goto failed;
    }

    context->type.server.callback(context);
    
    context->state == GRPC_C_STATE_DONE;
    grpc_c_server_recv_close_wait(recv_close);


failed:

    GRPC_C_INF("server call method %s exit!", context->method_url );

    grpc_c_context_free(context);

    if ( recv_close ) {
        grpc_c_server_recv_close_destory( recv_close );
    }
}

void grpc_c_server_register_event_cb(grpc_c_event_t *event, int success) {

    grpc_c_context_t *context = (grpc_c_context_t *)event->data;
    grpc_c_server_t  *server  = context->type.server.server_t;

    gpr_mu_lock(&server->lock);
    GRPC_LIST_REMOVE(&context->list);
    gpr_mu_unlock(&server->lock);

    grpc_c_register_method_ready(context->type.server.server_t, context->type.server.method);

    if ( !success ) {
        grpc_c_context_free(context);
    }else {
        grpc_c_thread_pool_add(server->thread_pool, grpc_c_server_call_start, (void *)context);
    }
}


int grpc_c_register_method_ready(grpc_c_server_t *server, grpc_c_method_t *np)
{
    grpc_call_error error;

    gpr_mu_lock(&server->lock);

    if ( server->shutdown ) {
        gpr_mu_unlock(&server->lock);
        return GRPC_C_OK;
    }

    grpc_c_context_t *context = grpc_c_context_init(np, 0);
    if (context == NULL) {
        gpr_mu_unlock(&server->lock);
        GRPC_C_ERR("Failed to create context before starting server");
        return GRPC_C_ERR_NOMEM;
    }

    context->type.server.method   = np;
    context->type.server.server_t = server;

    context->type.server.event.type     = GRPC_C_EVENT_SERVER_REGISTER;
    context->type.server.event.data     = context;
    context->type.server.event.callback = grpc_c_server_register_event_cb;

    error = grpc_server_request_registered_call(server->server,
                                                np->method_tag,
                                                &context->call,
                                                &context->deadline,
                                                &context->recv_init_metadata->metadata,
                                                NULL,
                                                server->queue,
                                                server->queue,
                                                &context->type.server.event);
    if (error != GRPC_CALL_OK) {
        gpr_mu_unlock(&server->lock);
        grpc_c_context_free(context);
        GRPC_C_ERR("Failed to register call: %d", error);
        return GRPC_C_ERR_FAIL;
    }

    GRPC_LIST_ADD_BEFORE(&context->list, &server->contexts_list_head);

    gpr_mu_unlock(&server->lock);

    return GRPC_C_OK;
}


/*
 * Register a method along with corresponding method functions
 */
int grpc_c_register_method( grpc_c_server_t *server, const char *method_url,
                            int client_streaming, int server_streaming,
                            grpc_c_service_callback_t handler,
                            grpc_c_method_funcs_t * funcs)
{
    void * method_tag;
    grpc_c_method_t * method;

    method_tag = grpc_server_register_method(server->server, 
                                             method_url, 
                                             server->hostname,
                                              GRPC_SRM_PAYLOAD_NONE, 0);
    if (method_tag == NULL) {
        GRPC_C_ERR("Failed to register method %s", method_url);
        return 1;
    }

    method = grpc_malloc(sizeof(grpc_c_method_t));
    if (method == NULL) {
        GRPC_C_ERR("Failed to allocate memory for method %s", method);
        return 1;
    }

    memset(method, 0, sizeof(grpc_c_method_t));

    method->method_url = gpr_strdup(method_url);
    method->method_tag = method_tag;
    method->funcs            = funcs;
    method->client_streaming = client_streaming;
    method->server_streaming = server_streaming;
    method->handler = (void *)handler;

    GRPC_LIST_ADD_BEFORE(&method->list, &server->method_list_head);

    return 0;
}

void grpc_c_method_destory(grpc_c_method_t * method)
{
    gpr_free(method->method_url);
    grpc_free(method);
}

/*
 * Adds insecure ip/port to grpc server
 */
int grpc_c_server_add_http2_port (grpc_c_server_t *server, const char* addr, grpc_server_credentials *creds)
{
    int ret;
    
    if (server == NULL) 
        return 0;

    if (creds == NULL) {
        ret = grpc_server_add_insecure_http2_port(server->server, addr);
    }else {
        ret = grpc_server_add_secure_http2_port(server->server, addr, creds);
    }

    return ret;
}

/*
 * Create a server object with given tcp/ip address
 */
grpc_c_server_t * grpc_c_server_create(const char *addr, grpc_server_credentials *creds, grpc_channel_args *args)
{
    int ret;
    grpc_c_server_t *server;
    
    server = (grpc_c_server_t *)grpc_malloc(sizeof(grpc_c_server_t));
    if (server == NULL) {
        return NULL;
    }
    
    memset(server, 0, sizeof(grpc_c_server_t));

    server->queue    = grpc_completion_queue_create_for_next(NULL);
    server->server   = grpc_server_create(args, NULL);
    server->hostname = gpr_strdup(addr);

    GRPC_LIST_INIT(&server->contexts_list_head);
    GRPC_LIST_INIT(&server->method_list_head);

    gpr_mu_init(&server->lock);
    gpr_cv_init(&server->shutdown_cv);

    /*
     * If we have credentials, we create a secure server
     */
    if (creds) {
        ret = grpc_server_add_secure_http2_port(server->server, addr, creds);
    } else {
        ret = grpc_server_add_insecure_http2_port(server->server, addr);
    }

    if (ret == 0) {
        grpc_c_server_destroy(server);
        return NULL;
    }

    grpc_server_register_completion_queue(server->server, server->queue, NULL);

    server->thread_pool = grpc_c_thread_pool_create(gpr_cpu_num_cores());
    
    return server;
}


void grpc_c_server_master_start(void *arg) {
    grpc_event       event;
    grpc_c_server_t *server = (grpc_c_server_t *)arg;
    
    for(;;) {
        event = grpc_completion_queue_next(server->queue, gpr_inf_future(GPR_CLOCK_MONOTONIC), NULL);
        if ( event.type == GRPC_OP_COMPLETE ) {
            grpc_c_event_t * gc_event = (grpc_c_event_t *)event.tag;
            if ( gc_event->type == GRPC_C_EVENT_SERVER_SHUTDOWN) {
                gpr_mu_lock(&server->lock);
                grpc_completion_queue_shutdown(server->queue);
                gpr_cv_signal(&server->shutdown_cv);
                gpr_mu_unlock(&server->lock);
            }else {
                gc_event->callback(gc_event, event.success);
            }
        }else {
            break;
        }
    }

    GRPC_C_INF("server master task exit!");
}


/*
 * Start server
 */
int grpc_c_server_start(grpc_c_server_t *server)
{
    struct grpc_c_list_s *item;
    grpc_c_method_t *np;
    int ret;

    if (server == NULL) {
        GRPC_C_ERR("Invalid server");
        return 1;
    }

    grpc_server_start(server->server);

    GRPC_LIST_TRAVERSAL(item, &server->method_list_head)
    {
        np = GRPC_LIST_OFFSET(item, grpc_c_method_t, list); 
        ret = grpc_c_register_method_ready(server, np);
        if (ret != 0) {
            GRPC_C_ERR("Failed to reregister method %s", np->method_url);
            return ret;
        }
    }

    /*
     * Schedule a callback if we are threaded
     */
    grpc_c_thread_pool_add(server->thread_pool, grpc_c_server_master_start, server);

    return GRPC_C_OK;
}

/*
 * Makes a threaded server block
 */
void grpc_c_server_wait(grpc_c_server_t *server)
{
    gpr_mu_lock(&server->lock);
    gpr_cv_wait(&server->shutdown_cv, &server->lock, gpr_inf_future(GPR_CLOCK_MONOTONIC));
    gpr_mu_unlock(&server->lock);

    grpc_c_thread_pool_shutdown(server->thread_pool);
}


/*
 * stop server
 */
int grpc_c_server_stop(grpc_c_server_t *server)
{
    gpr_mu_lock(&server->lock);
    
    if ( server->shutdown ) {
        gpr_mu_unlock(&server->lock);
        return GRPC_C_OK;
    }

    server->shutdown_event.type = GRPC_C_EVENT_SERVER_SHUTDOWN;
    server->shutdown_event.data = (grpc_c_server_t *)server;
    server->shutdown_event.callback = NULL;

    grpc_server_shutdown_and_notify(server->server, server->queue, &server->shutdown_event);

    server->shutdown = 1;    
    gpr_mu_unlock(&server->lock);

    return GRPC_C_OK;
}

/*
 * free grpc-c server
 */
void grpc_c_server_destroy(grpc_c_server_t *server)
{
    grpc_c_list_t *item;
    grpc_c_list_t *temp;
    
    gpr_mu_lock(&server->lock);

    GRPC_LIST_TRAVERSAL_REMOVE(item, temp, &server->contexts_list_head) 
    {
        grpc_c_context_t *context = GRPC_LIST_OFFSET(item, grpc_c_context_t, list);
        GRPC_LIST_REMOVE(&context->list);
        grpc_c_context_free(context);
    }

    GRPC_LIST_TRAVERSAL_REMOVE(item, temp, &server->method_list_head) 
    {
        grpc_c_method_t *method = GRPC_LIST_OFFSET(item, grpc_c_method_t, list);
        GRPC_LIST_REMOVE(&method->list);
        grpc_c_method_destory(method);
    }
    
    if (server->queue)
    {
        grpc_completion_queue_destroy(server->queue);
        server->queue = NULL;
    }
    
    if (server->server)
    {
        grpc_server_cancel_all_calls(server->server);
        grpc_server_destroy(server->server);
        server->server = NULL;
    }

    if (server->hostname)
    {
        gpr_free(server->hostname);
    }

    gpr_mu_unlock(&server->lock);

    gpr_mu_destroy(&server->lock);
    gpr_cv_destroy(&server->shutdown_cv);

    grpc_free(server);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

