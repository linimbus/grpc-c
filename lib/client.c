#include <stdio.h>

#include <grpc-c/grpc-c.h>
#include <grpc/support/alloc.h>
#include <grpc/support/sync.h>

#include "context.h"
#include "thread_pool.h"
#include "stream_ops.h"
#include "trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int grpc_c_client_connect_status_sub(grpc_c_client_t *client);


/*
 * Main function for sync nostreaming RPC call from client
 */
int grpc_c_client_request_sync( grpc_c_client_t *client,
                               grpc_c_metadata_array_t *array, uint32_t flags,
                               const char *method_url,
                               void *input, void **output,
                               grpc_c_status_t *status,
                               grpc_c_method_funcs_t * funcs,
                               long timeout)
{
    int ret;
    grpc_c_method_t method;
    grpc_c_context_t *context;

    method.method_url = (char *)method_url;
    method.client_streaming = 0;
    method.server_streaming = 0;
    method.funcs   = funcs;
    method.handler = NULL;

    context = grpc_c_context_init(&method, 1);
    if (context == NULL) {
        return GRPC_C_ERR_NOMEM;
    }

    context->state = GRPC_C_STATE_RUN;

    context->call = grpc_channel_create_call(client->channel, NULL, 0, 
                                             client->queue, 
                                             grpc_slice_from_static_string(method_url), 
                                             &client->host, 
                                             gpr_inf_future(GPR_CLOCK_MONOTONIC), NULL);
    if (context->call == NULL) {
        grpc_c_context_free(context);
        return GRPC_C_ERR_NOMEM;
    }

    ret = grpc_c_send_initial_metadata(context->call, context->send_init_metadata, timeout);
    if (ret != GRPC_C_OK) {
        goto failed;
    }

    ret = grpc_c_write(context, input, 0, timeout);
    if (ret != GRPC_C_OK) {
        goto failed;
    }

    ret = grpc_c_write_done(context, 0, timeout);
    if (ret != GRPC_C_OK) {
        goto failed;
    }

    ret = grpc_c_recv_initial_metadata(context->call, context->recv_init_metadata, timeout);
    if (ret != GRPC_C_OK) {
        goto failed;
    }

    ret = grpc_c_read(context, output, 0, timeout);
    if (ret != GRPC_C_OK) {
        goto failed;
    }

    return grpc_c_finish(context, status, 0);

failed:
    grpc_c_context_free(context);
    return ret;
}

/*
 * Main function for async nostreaming RPC call from client
 */
int grpc_c_client_request_async( grpc_c_client_t *client,
                                 grpc_c_metadata_array_t *mdarray, uint32_t flags,
                                 const char *method_url,
                                 void *input,
                                 grpc_c_client_callback_t *cb, void *tag,
                                 grpc_c_method_funcs_t * funcs,
                                 long timeout)
{
    return 0;
}

/*
 * Main function for streaming RPC call from client
 */
int grpc_c_client_request_stream( grpc_c_client_t *client,
                                    grpc_c_metadata_array_t *mdarray, uint32_t flags,
                                    const char *method_url,
                                    grpc_c_context_t **pcontext,
                                    int client_streaming, int server_streaming,
                                    grpc_c_method_funcs_t * funcs,
                                    long timeout)
{
    int ret;
    grpc_c_method_t method;
    grpc_c_context_t *context;

    method.method_url = (char *)method_url;
    method.client_streaming = client_streaming;
    method.server_streaming = server_streaming;
    method.funcs   = funcs;
    method.handler = NULL;

    context = grpc_c_context_init(&method, 1);
    if (context == NULL) {
        return GRPC_C_ERR_NOMEM;
    }

    context->state = GRPC_C_STATE_RUN;
    context->call = grpc_channel_create_call(client->channel, NULL, 0, 
                                             client->queue, 
                                             grpc_slice_from_static_string(method_url), 
                                             &client->host, 
                                             gpr_inf_future(GPR_CLOCK_MONOTONIC), NULL);
    if (context->call == NULL) {
        grpc_c_context_free(context);
        return GRPC_C_ERR_NOMEM;
    }

    ret = grpc_c_send_initial_metadata(context->call, context->send_init_metadata, timeout);
    if (ret != GRPC_C_OK) {
        goto failed;
    }

    ret = grpc_c_recv_initial_metadata(context->call, context->recv_init_metadata, timeout);
    if (ret != GRPC_C_OK) {
        goto failed;
    }

    *pcontext = context;

    return GRPC_C_OK;

failed:
    grpc_c_context_free(context);
    return ret;    
}


void grpc_c_client_connect_status_event_cb(grpc_c_event_t *event, int success) {
    grpc_c_client_t *client = (grpc_c_client_t *)event->data;

    grpc_c_client_connect_status_sub(client);
}

int grpc_c_client_connect_status_sub(grpc_c_client_t *client) {

    gpr_mu_lock(&client->lock);

    if ( client->channel ) {
        
        client->connect_status = grpc_channel_check_connectivity_state(client->channel, 0);

        client->connect_event.type     = GRPC_C_EVENT_CLIENT_CONNECT;
        client->connect_event.data     = client;
        client->connect_event.callback = grpc_c_client_connect_status_event_cb;

        /*
         * Watch for change in channel connectivity
         */
        grpc_channel_watch_connectivity_state(client->channel,
                                              client->connect_status,
                                              gpr_inf_future(GPR_CLOCK_REALTIME),
                                              client->queue,
                                              (void *)&client->connect_event);

        GRPC_C_INF("client connect status is %d", client->connect_status );
    }

    gpr_mu_unlock(&client->lock);

    return GRPC_C_OK;
}



void grpc_c_client_master_task(void *arg) {
    grpc_event event;
    grpc_c_client_t *client = (grpc_c_client_t *)arg;

    for(;;) {
        event = grpc_completion_queue_next(client->queue, gpr_inf_future(GPR_CLOCK_MONOTONIC), NULL);
        if (event.type == GRPC_OP_COMPLETE ) {
            grpc_c_event_t * grpc_event = (grpc_c_event_t *)event.tag;
            grpc_event->callback(grpc_event, event.success);
        }else if ( event.type == GRPC_QUEUE_SHUTDOWN ) {

            gpr_mu_lock(&client->lock);
            gpr_cv_signal(&client->shutdown_cv);
            gpr_mu_unlock(&client->lock);

            break;
        }
    }

    GRPC_C_INF("client master task exit!");
}


grpc_c_client_t * grpc_c_client_init( const char *server_name, 
                                    grpc_channel_credentials *channel_creds,
                                    grpc_channel_args *channel_args,
                                    int thread_nums)
{
    grpc_c_client_t *client;
    int n;
    if (thread_nums <= 0)
    {
        n = gpr_cpu_num_cores();
    }
    else
    {
    	n = thread_nums;
    }

    if (server_name == NULL) {
        GRPC_C_ERR("Invalid hostname or client-id");
        return NULL;
    }

    client = (grpc_c_client_t *)grpc_malloc(sizeof(grpc_c_client_t));
    if (client == NULL) {
        GRPC_C_ERR("Failed to allocate memory for client");
        return NULL;
    }

    memset(client, 0, sizeof(grpc_c_client_t));

    client->host  = grpc_slice_from_copied_string(server_name);

    client->thread_pool = grpc_c_thread_pool_create(n);

    /*
     * Initialize mutex and condition variables
     */
    gpr_mu_init(&client->lock);
    gpr_cv_init(&client->shutdown_cv);

    /*
     * Create a channel using given hostname. If channel credentials are
     * provided, create a secure channel. Otherwise go for an insecure one
     */
    if (channel_creds) {
        client->channel = grpc_secure_channel_create(channel_creds, server_name, channel_args, NULL);
    } else {
        client->channel = grpc_insecure_channel_create(server_name, channel_args, NULL);
    }

    if (client->channel == NULL) {
        GRPC_C_ERR("Failed to create a channel");
        grpc_c_client_free(client);
        return NULL;
    }

    /*
     * Register server connect and disconnect callbacks
     */
    client->queue = grpc_completion_queue_create_for_next(NULL);
    if (client->queue == NULL) {
        GRPC_C_ERR( "Failed to create completion queue for server connect/disconnect notifications");
        grpc_c_client_free(client);
        return NULL;
    }

    grpc_c_client_connect_status_sub(client);

    grpc_c_thread_pool_add(client->thread_pool, grpc_c_client_master_task, client);
    
    return client;
}

int grpc_c_client_stop (grpc_c_client_t *client) { 
    grpc_event event;
    gpr_mu_lock(&client->lock);
    client->shutdown = 1;
    grpc_channel_destroy(client->channel);
    client->channel = NULL;
    grpc_completion_queue_shutdown(client->queue);
    do 
    {
        event = grpc_completion_queue_next(client->queue, gpr_inf_future(GPR_CLOCK_REALTIME), NULL);
    } while (event.type != GRPC_QUEUE_SHUTDOWN);       

    gpr_mu_unlock(&client->lock);

    return GRPC_C_OK;
}


/*
 * Waits for all callbacks to get done in a threaded client
 */
void grpc_c_client_wait (grpc_c_client_t *client)
{
    gpr_mu_lock(&client->lock);
    if ( !client->shutdown ) {
        (void)gpr_cv_wait(&client->shutdown_cv, &client->lock, grpc_c_deadline_from_timeout(-1));
    }
    gpr_mu_unlock(&client->lock);
}

/*
 * Destroy and free client object
 */
void grpc_c_client_free (grpc_c_client_t *client)
{
    grpc_c_thread_pool_shutdown(client->thread_pool);

    gpr_mu_destroy(&client->lock);
    gpr_cv_destroy(&client->shutdown_cv);

    grpc_completion_queue_destroy(client->queue);

    gpr_slice_unref(client->host);

    grpc_free(client);
}




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

