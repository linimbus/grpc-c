
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


int grpc_c_register_method_ready(grpc_c_server_t *server, grpc_c_method_t *np)
{
    grpc_call_error e;

    /*
     * Create a context that gets returned when this is method is called
     */
    grpc_c_context_t *context = grpc_c_context_init(np, 0);
    if (context == NULL) {
		GRPC_C_ERR("Failed to create context before starting server");
		return 1;
    }

    context->gcc_event.type = GRPC_C_EVENT_SERVER_REGISTER;
    context->gcc_event.data = context;

    context->gcc_cq    = grpc_completion_queue_create_for_next(NULL);
    context->gcc_state = GRPC_C_STATE_NEW;

    if (!server->gcs_shutdown) {
		e = grpc_server_request_registered_call(server->gcs_server,
												np->method_tag,
												&context->gcc_call,
												&context->gcc_deadline,
												context->gcc_metadata,
												NULL,
												context->gcc_cq,
												server->gcs_cq,
												&context->gcc_event);

		if (e != GRPC_CALL_OK) {
		    grpc_c_context_free(context);
		    GRPC_C_ERR("Failed to register call: %d", e);
		    return 1;
		}
    }

    return 0;
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

	method_tag = grpc_server_register_method(server->gcs_server, 
											 method_url, 
											 server->gcs_host,
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

	method->method_url = gpr_strdup(method_url);
	method->method_tag = method_tag;
	method->funcs = funcs;
	method->client_streaming = client_streaming;
	method->server_streaming = server_streaming;
	method->handler = handler;

	GRPC_LIST_ADD_BEFORE(&method->list, &server->method_list_head);

	return 0;
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
		ret = grpc_server_add_insecure_http2_port(server->gcs_server, addr);
	}else {
		ret = grpc_server_add_secure_http2_port(server->gcs_server, addr, creds);
	}

    return ret;
}


/*
 * Create a server object with given daemon name. We build unix domain socket
 * path from this name
 */
grpc_c_server_t * grpc_c_server_create( const char *name, grpc_server_credentials *creds, grpc_channel_args *args)
{

    return NULL;
}

/*
 * Create a server object with given tcp/ip address
 */
grpc_c_server_t * grpc_c_server_create_by_host(const char *addr, grpc_server_credentials *creds, grpc_channel_args *args)
{
	int ret;
	grpc_c_server_t *server;
	
	server = gpr_malloc(sizeof(grpc_c_server_t));
	if (server == NULL) {
		return NULL;
	}
	
	memset(server, 0, sizeof(grpc_c_server_t));

	server->gcs_cq = grpc_completion_queue_create_for_next(NULL);
	server->gcs_server = grpc_server_create(args, NULL);
	server->gcs_host = strdup(addr);

	gpr_mu_init(&server->gcs_lock);
	gpr_cv_init(&server->gcs_cq_destroy_cv);
	gpr_cv_init(&server->gcs_shutdown_cv);

	/*
	 * If we have credentials, we create a secure server
	 */
	if (creds) {
		ret = grpc_server_add_secure_http2_port(server->gcs_server, addr, creds);
	} else {
		ret = grpc_server_add_insecure_http2_port(server->gcs_server, addr);
	}

	if (ret == 0) {
		grpc_c_server_destroy(server);
		return NULL;
	}

	grpc_server_register_completion_queue(server->gcs_server, server->gcs_cq, NULL);

	return 0;
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

	grpc_server_start(server->gcs_server);

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


	return 0;
}

/*
 * Makes a threaded server block
 */
void grpc_c_server_wait(grpc_c_server_t *server)
{

}

/*
 * stop server
 */
int grpc_c_server_stop(grpc_c_server_t *server)
{

}

/*
 * free grpc-c server
 */
void grpc_c_server_destroy(grpc_c_server_t *server)
{


}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

