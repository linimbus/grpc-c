#include <stdio.h>

#include <grpc-c/grpc-c.h>
#include <grpc/support/alloc.h>

#include "context.h"
#include "thread_pool.h"
#include "stream_ops.h"
#include "trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


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

	return 0;
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
                  				  grpc_c_context_t **context,
                  				  int client_streaming, int server_streaming,
                  				  grpc_c_method_funcs_t * funcs,
                  				  long timeout)
{
	return 0;
}


grpc_c_client_t * grpc_c_client_init( const char *server_name, const char *client_id,
				        		    grpc_channel_credentials *channel_creds,
				        		    grpc_channel_args *channel_args)
{
	grpc_c_client_t *client;

	if (server_name == NULL || client_id == NULL) {
		GRPC_C_ERR("Invalid hostname or client-id");
		return NULL;
	}

	client = grpc_malloc(sizeof(grpc_c_client_t));
	if (client == NULL) {
		GRPC_C_ERR("Failed to allocate memory for client");
		return NULL;
	}

	memset(client, 0, sizeof(grpc_c_client_t));

	client->gcc_id   = strdup(client_id);
	client->gcc_host = grpc_slice_from_copied_string(server_name);

	/*
	 * Create a channel using given hostname. If channel credentials are
	 * provided, create a secure channel. Otherwise go for an insecure one
	 */
	if (channel_creds) {
		client->gcc_channel = grpc_secure_channel_create(channel_creds, server_name, channel_args, NULL);
	} else {
		client->gcc_channel = grpc_insecure_channel_create(server_name, channel_args, NULL);
	}

	if (client->gcc_channel == NULL) {
		GRPC_C_ERR("Failed to create a channel");
		grpc_c_client_free(client);
		return NULL;
	}

	/*
	 * Initialize mutex and condition variables
	 */
	gpr_mu_init(&client->gcc_lock);
	gpr_cv_init(&client->gcc_shutdown_cv);

	/*
	 * Register server connect and disconnect callbacks
	 */
	client->gcc_channel_connectivity_cq = grpc_completion_queue_create_for_next(NULL);
	if (client->gcc_channel_connectivity_cq == NULL) {
		GRPC_C_ERR( "Failed to create completion queue for server connect/disconnect notifications");
		grpc_c_client_free(client);
		return NULL;
	}

	client->gcc_channel_state = grpc_channel_check_connectivity_state(client->gcc_channel, 0);

	/*
	 * Watch for change in channel connectivity
	 */
	grpc_channel_watch_connectivity_state(client->gcc_channel,
										  client->gcc_channel_state,
										  gpr_inf_future(GPR_CLOCK_REALTIME),
										  client->gcc_channel_connectivity_cq,
										  (void *) client);
	
	return client;
}

/*
 * Initialize a client with client_id and server address
 */
grpc_c_client_t *
grpc_c_client_init_by_host( const char *address, const char *client_id,
            			    grpc_channel_credentials *channel_creds,
            			    grpc_channel_args *channel_args)
{
	return NULL;
}


/*
 * Waits for all callbacks to get done in a threaded client
 */
void grpc_c_client_wait (grpc_c_client_t *client)
{

}

/*
 * Destroy and free client object
 */
void grpc_c_client_free (grpc_c_client_t *client)
{

}




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

