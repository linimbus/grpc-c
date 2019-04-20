
#ifndef __GRPC_C_H__
#define __GRPC_C_H__

#include <string.h>
#include <grpc/grpc.h>
#include <grpc/grpc_security.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/string_util.h>
#include <protobuf-c/protobuf-c.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define GRPC_C_DAEMON_SOCK_PATH "unix:/var/run/grpc_c_"

#define GRPC_C_BUFSIZ            1024

/*
 * Tracing levels and related functions
 */
#define GRPC_C_TRACE_TCP                   (1 << 0)
#define GRPC_C_TRACE_CHANNEL               (1 << 1)
#define GRPC_C_TRACE_SURFACE               (1 << 2)
#define GRPC_C_TRACE_HTTP                  (1 << 3)
#define GRPC_C_TRACE_FLOWCTL               (1 << 4)
#define GRPC_C_TRACE_BATCH                 (1 << 5)
#define GRPC_C_TRACE_CONNECTIVITY_STATE    (1 << 6)
#define GRPC_C_TRACE_SECURE_ENDPOINT       (1 << 7)
#define GRPC_C_TRACE_TRANSPORT_SECURITY    (1 << 8)
#define GRPC_C_TRACE_ROUND_ROBIN           (1 << 9)
#define GRPC_C_TRACE_HTTP_WRITE_STATE      (1 << 10)
#define GRPC_C_TRACE_API                   (1 << 11)
#define GRPC_C_TRACE_CHANNEL_STACK_BUILDER (1 << 12)
#define GRPC_C_TRACE_HTTP1                 (1 << 13)
#define GRPC_C_TRACE_COMPRESSION           (1 << 14)
#define GRPC_C_TRACE_QUEUE_PLUCK           (1 << 15)
#define GRPC_C_TRACE_QUEUE_TIMEOUT         (1 << 16)
#define GRPC_C_TRACE_OP_FAILURE            (1 << 17)
#define GRPC_C_TRACE_CORE                  (1 << 18)
#define GRPC_C_TRACE_ALL                   (~0)

void grpc_c_trace_enable(int flags, int severity);
void grpc_c_trace_disable(int flags);


/*
 * Write return status. This should eventually become an enum and
 * writer->write should return that type instead of int
 */
#define GRPC_C_WRITE_OK        0
#define GRPC_C_WRITE_FAIL      1
#define GRPC_C_WRITE_PENDING   2

/*
 * GRPC-C return status codes
 */
#define GRPC_C_OK             0
#define GRPC_C_ERR_FAIL       1
#define GRPC_C_ERR_TMOUT      2

/*
 * List of all the possible states for grpc_c client and server
 */
typedef enum grpc_c_state_s {
    GRPC_C_STATE_NEW,       /* Waiting to call */
    GRPC_C_STATE_RUN,       /* Called RPC handler */
    GRPC_C_STATE_DONE,      /* Finished */
    GRPC_C_STATE_FREE,      /* Context is free */
} grpc_c_state_t;


/*
 * Types of events that we use with tag when batching gRPC operations
 */
typedef enum grpc_c_event_type_s {
    GRPC_C_EVENT_SEND_METADATA,
    GRPC_C_EVENT_RECV_METADATA,
    GRPC_C_EVENT_READ,
    GRPC_C_EVENT_WRITE,
    GRPC_C_EVENT_WRITE_DONE,
    GRPC_C_EVENT_FINISH,
    GRPC_C_EVENT_RECV_CLOSE,
    GRPC_C_EVENT_SERVER_SHUTDOWN,
} grpc_c_event_type_t;

/*
 * Event structure to be used as tag when batching gRPC operations
 */
typedef struct grpc_c_event_s {
    grpc_c_event_type_t type;   /* Type of this event */
    void              * data;	/* Data associated with this event */
} grpc_c_event_t;

typedef struct grpc_c_list_s {
	struct grpc_c_list_s * next;
	struct grpc_c_list_s * prev;
}grpc_c_list_t;


/*
 * Structure definition for return status of RPC
 */
typedef struct grpc_c_status_s {
    int  code;
    char message[GRPC_C_BUFSIZ];
} grpc_c_status_t;


/*
 * Forward declarations
 */
typedef struct grpc_c_server_s grpc_c_server_t;
typedef struct grpc_c_client_s grpc_c_client_t;
typedef struct grpc_c_context_s grpc_c_context_t;
typedef struct grpc_c_method_funcs_s grpc_c_method_funcs_t;
typedef grpc_metadata_array grpc_c_metadata_array_t;


typedef size_t (*grpc_c_method_data_pack_t)(void *input, grpc_byte_buffer **buffer);

typedef void *(*grpc_c_method_data_unpack_t)(grpc_c_context_t *context, grpc_byte_buffer *input);

typedef void (*grpc_c_method_data_free_t)(grpc_c_context_t *context, void *buf);

/*
 * Structure definition for method functions
 */
struct grpc_c_method_funcs_s {
    grpc_c_method_data_pack_t   *input_packer;	    /* Input packer */
    grpc_c_method_data_unpack_t *input_unpacker;	/* Input unpacker */
    grpc_c_method_data_free_t   *input_free;		/* Input free function */
    grpc_c_method_data_pack_t   *output_packer;	    /* Output packer */
    grpc_c_method_data_unpack_t *output_unpacker;	/* Output unpacker */
    grpc_c_method_data_free_t   *output_free;	    /* Output free function */
};

/*
 * Definition for RPC method structure
 */
struct grpc_c_method_t {
    grpc_c_list_t list;
    void * method_tag;			/* Tag returned by grpc_server_register_method() */
    char * method_url;			/* URL for this RPC */
    int client_streaming;		/* Flag to indicate if client is streaming */
    int server_streaming;		/* Flag to indicate if server is streaming */
    grpc_c_method_funcs_s * funcs;
};

/*
 * Signature for client callback
 */
typedef void (* grpc_c_client_callback_t)(grpc_c_context_t *context, void * tag, int success);

/*
 * Service implementation
 */
typedef void (* grpc_c_service_callback_t)(grpc_c_context_t *context);



/*
 * Client structure related definitions
 */

/*
 * Structure definition for grpc_c client
 */
struct grpc_c_client_s {
    grpc_channel *gcc_channel;	    /* Underlying grpc channel to host */
    grpc_completion_queue *gcc_cq;  /* Completion queue associated with this client */
    grpc_completion_queue *gcc_channel_connectivity_cq;  /* Completion queue to receive channel connectivity change events */
    gpr_slice gcc_host;		    /* Hostname of remote providing RPC service */
    char *gcc_id;		        /* Client identification string */
    int gcc_channel_state;	    /* Channel connectivity state */
    int gcc_connected;		    /* Connection status */
    int gcc_conn_timeout;	    /* Connection timeout flag */
    void *gcc_retry_tag;	    /* Retry tag to be used when stopping reconnection attempts */
    gpr_mu gcc_lock;		    /* Mutex lock */
    gpr_cv gcc_callback_cv;	    /* Callback condition variable */
    gpr_cv gcc_shutdown_cv;	    /* Shutdown condition variable */
    int gcc_running_cb;		    /* Current running callbacks */
    int gcc_shutdown;		    /* Client shutdown flag */
    int gcc_wait;		        /* Waiting flag */
    grpc_c_list_t context_list; /* List of active context objects */
};

/*
 * Structure definition for grpc-c context
 */
struct grpc_c_context_s {
    struct grpc_c_method_t *gcc_method;		/* Corresponding method */
    grpc_byte_buffer *gcc_payload;		/* Payload holder */
    grpc_op *gcc_ops;				/* Array of grpc operations */
    grpc_byte_buffer **gcc_ops_payload;		/* Payload per operation */
    grpc_completion_queue *gcc_cq;		/* Completion queue associated
						   with this context */
    gpr_timespec gcc_deadline;			/* Deadline for operations in
						   this context */
    grpc_c_metadata_array_t *gcc_metadata;	/* Metadata array to send
						   metadata with each call */
    char **gcc_metadata_storage;		/* Array pointing to key value
						   pair storage */
    grpc_c_metadata_array_t *gcc_initial_metadata;  /* Initial metadata array */
    char **gcc_initial_metadata_storage;	/* Array containing intitial
						   metadata key value pairs */
    grpc_c_metadata_array_t *gcc_trailing_metadata; /* Trailing metadata array */
    char **gcc_trailing_metadata_storage;	/* Array containing trailing
						   metadata key value pairs */
    void *gcc_tag;				/* User provided tag to
						   identify context in
						   callbacks */
    int gcc_meta_sent;				/* Flag to mark that initial
						   metadata is sent */
    int gcc_is_client;				/* Flag to mark if context
						   belongs to client */
    int gcc_op_count;				/* Number of pending grpc
						   operations */
    int gcc_op_capacity;			/* Capacity of gcc_ops array */
    grpc_c_state_t gcc_state;			/* Current state of
						   client/server */
    grpc_call *gcc_call;			/* grpc_call for this RPC */
    gpr_mu *gcc_lock;				/* Mutex for access to this cq */
    grpc_status_code gcc_status;		/* Result of RPC execution */
    grpc_slice gcc_status_details;		/* Status details from RPC
						   execution */
    grpc_c_method_funcs_t *gcc_method_funcs;	/* Pointer to method functions
						   like input/output packer,
						   unpacker, free and method
						   callbacks */

    grpc_c_service_callback_t *gcmfh_server;    /* RPC handler */
    grpc_c_client_callback_t  *gcmfh_client;     /* Client callback */

    void *gcc_read_resolve_arg;			/* Data that can be passed to
						   when calling read resolve
						   cb */
    void *gcc_write_resolve_arg;		/* Identifying data that can
						   be passed to user provided
						   callback when a write is
						   finished */
    int gcc_call_cancelled;			/* Boolean indicating that call has been cancelled */
    int gcc_client_cancel;			/* Boolean indicating if client has cancelled the call */
    grpc_c_event_t gcc_event;			/* grpc-c event this context belongs to */
    grpc_c_event_t gcc_read_event;		/* Event tag for read ops */
    grpc_c_event_t gcc_write_event;		/* Event tag for write ops */
    grpc_c_event_t gcc_write_done_event;	/* Event tag for write done from client */
    grpc_c_event_t gcc_recv_close_event;	/* Recv close grpc-c event in case of server context */

	grpc_c_server_t *gccd_server;
	grpc_c_client_t *gccd_client;

	grpc_c_list_t list; /* List of context objects */
};


/*
 * User provided memory alloc and free functions
 */
typedef void *(* grpc_c_memory_alloc_func_t)(grpc_c_context_t *context, size_t size);

typedef void (* grpc_c_memory_free_func_t)(grpc_c_context_t *context, void *data);

void grpc_c_set_memory_function(grpc_c_memory_alloc_func_t , grpc_c_memory_free_func_t );

ProtobufCAllocator *
grpc_c_get_protobuf_c_allocator (grpc_c_context_t *context,
				                 ProtobufCAllocator *allocator);

/*
 * Server structure definition
 */
struct grpc_c_server_s {
    char *gcs_host;			                 /* Server hostname */
    grpc_server *gcs_server;		         /* Grpc server */
    grpc_completion_queue *gcs_cq;	         /* Server completion queue */
    grpc_c_list_t method_list_head;
    int gcs_method_count;		          /* Number of registered methods */
    grpc_c_context_t **gcs_contexts;      /* List of context objects waiting on methods */
    int gcs_running_cb;			          /* Number of currently running callbacks */
    gpr_mu gcs_lock;			          /* Mutex lock */
    gpr_cv *gcs_callback_cv;		      /* Callback condition variable */
    gpr_cv gcs_shutdown_cv;		          /* Shutdown condition variable */
    gpr_cv gcs_cq_destroy_cv;		      /* Completion queue destroy cv */
    int gcs_shutdown;			          /* Server shutting down */
    int *gcs_callback_shutdown;		      /* Shadow value so grpc_c_server_wait() can consume */
    int *gcs_callback_running_cb;	      /* Shadow running callback count */
    int gcs_cq_shutdown;		          /* Boolean to indicate that server completion queue has shutdown */
    grpc_c_event_t gcs_shutdown_event;	  /* Event signalling server shutdown */
};


/*
 * Initialize libgrpc-c to be used with given underlying libgrpc. Second
 * parameter is used to pass data to underlying library if it needs any
 */
void grpc_c_init(void);

/*
 * Shutsdown initialized grpc-c library. To be called towards end of program
 */
void grpc_c_shutdown(void);


int grpc_c_read(grpc_c_context_t *context, void **content, uint32_t flags, long timeout);

int grpc_c_write(grpc_c_context_t *context, void *output, uint32_t flags, long timeout);

int grpc_c_client_finish(grpc_c_context_t *context, grpc_c_status_t *status, uint32_t flags);

int grpc_c_server_finish(grpc_c_context_t *context, grpc_c_status_t *status, uint32_t flags);


/*
 * Initialize a client with client_id to server_name. We build unix domain
 * socket path to server from server_name. When channel_creds is given, we
 * create a secure channel. Otherwise it'll be an insecure one.
 */
grpc_c_client_t *
grpc_c_client_init( const char *server_name, const char *client_id,
        		    grpc_channel_credentials *channel_creds,
        		    grpc_channel_args *channel_args);

/*
 * Initialize a client with client_id and server address
 */
grpc_c_client_t *
grpc_c_client_init_by_host( const char *address, const char *client_id,
            			    grpc_channel_credentials *channel_creds,
            			    grpc_channel_args *channel_args);

/*
 * Waits for all callbacks to get done in a threaded client
 */
void grpc_c_client_wait (grpc_c_client_t *client);

/*
 * Destroy and free client object
 */
void grpc_c_client_free (grpc_c_client_t *client);


/*
 * Create a server object with given daemon name. We build unix domain socket
 * path from this name
 */
grpc_c_server_t * grpc_c_server_create( const char *name, grpc_server_credentials *creds, grpc_channel_args *args);

/*
 * Create a server object with given tcp/ip address
 */
grpc_c_server_t * grpc_c_server_create_by_host(const char *addr, grpc_server_credentials *creds, grpc_channel_args *args);

/*
 * Start server
 */
int grpc_c_server_start(grpc_c_server_t *server);

/*
 * Makes a threaded server block
 */
void grpc_c_server_wait(grpc_c_server_t *server);

/*
 * stop server
 */
int grpc_c_server_stop(grpc_c_server_t *server);

/*
 * free grpc-c server
 */
void grpc_c_server_destroy(grpc_c_server_t *server);


/*
 * Register a method along with corresponding method functions
 */
int grpc_c_register_method( grpc_c_server_t *server, const char *method_url,
            			    int client_streaming, int server_streaming,
            			    grpc_c_service_callback_t *handler,
            			    grpc_c_method_funcs_t * funcs);

/*
 * Metadata array
 */
void grpc_c_metadata_array_init(grpc_c_metadata_array_t *array);

void grpc_c_metadata_array_destroy(grpc_c_metadata_array_t *array);

/*
 * Extract the value from metadata by key. Return NULL if not found
 */
int grpc_c_get_metadata_by_key(grpc_c_context_t *context, const char *key, char *value, size_t len);

/*
 * Extract the value from initial metadata by key. Return NULL if not found
 */
int grpc_c_get_initial_metadata_by_key(grpc_c_context_t *context, const char *key, char *value, size_t len);

/*
 * Extract the value from trailing metadata by key. Return NULL if not found
 */
int grpc_c_get_trailing_metadata_by_key(grpc_c_context_t *context, const char *key, char *value, size_t len);

/*
 * Adds given key value pair to initial metadata array of given context.
 * Returns 0 on success and 1 on failure
 */
int grpc_c_add_initial_metadata(grpc_c_context_t *context, const char *key, const char *value);

/*
 * Adds given key value pair to trailing metadata array of given context.
 * Returns 0 on success and 1 on failure
 */
int grpc_c_add_trailing_metadata(grpc_c_context_t *context, const char *key, const char *value);

/*
 * sends immediately the available initial metadata from server.
 * NOTE: This will block the caller till the initial metadata is sent to the
 * receiver. If this is not called, all the added initial metadata will be
 * sent upon first write from server
 */
int grpc_c_send_initial_metadata(grpc_c_context_t *context, long timeout);

/*
 * Get client-id from context
 */
const char *grpc_c_get_client_id(grpc_c_context_t *context);

/*
 * Main function for sync nostreaming RPC call from client
 */
int grpc_c_client_request_sync( grpc_c_client_t *client,
                			    grpc_c_metadata_array_t *array, uint32_t flags,
                			    const char *method_url,
                			    void *input, void **output,
                				grpc_c_status_t *status,
                				grpc_c_method_funcs_t * funcs,
                				long timeout);

/*
 * Main function for async nostreaming RPC call from client
 */
int grpc_c_client_request_async( grpc_c_client_t *client,
                				 grpc_c_metadata_array_t *mdarray, uint32_t flags,
                				 const char *method_url,
                				 void *input,
                				 grpc_c_client_callback_t *cb, void *tag,
                				 grpc_c_method_funcs_t * funcs,
                				 long timeout);

/*
 * Main function for streaming RPC call from client
 */
int grpc_c_client_request_stream( grpc_c_client_t *client,
                  				  grpc_c_metadata_array_t *mdarray, uint32_t flags,
                  				  const char *method_url, grpc_c_context_t **context,
                  				  int client_streaming, int server_streaming,
                  				  grpc_c_method_funcs_t * funcs,
                  				  long timeout);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* GRPC_C_GRPC_C_H */
