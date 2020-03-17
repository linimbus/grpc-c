
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
#define GRPC_C_ERR_NOMEM      3
#define GRPC_C_ERR_NORECV     4

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
    GRPC_C_EVENT_SERVER_REGISTER,
    GRPC_C_EVENT_CLIENT_CONNECT,
} grpc_c_event_type_t;


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
typedef struct grpc_c_thread_pool_s grpc_c_thread_pool_t;

typedef struct grpc_c_event_s grpc_c_event_t;
typedef struct grpc_c_server_s grpc_c_server_t;
typedef struct grpc_c_client_s grpc_c_client_t;
typedef struct grpc_c_context_s grpc_c_context_t;
typedef struct grpc_c_method_funcs_s grpc_c_method_funcs_t;
typedef struct grpc_c_method_s grpc_c_method_t;

typedef grpc_metadata_array grpc_c_metadata_array_t;

typedef void (*grpc_c_event_callback_t)(grpc_c_event_t *event, int success);

typedef size_t (*grpc_c_method_data_pack_t)(void *input, grpc_byte_buffer **buffer);

typedef void *(*grpc_c_method_data_unpack_t)(grpc_c_context_t *context, grpc_byte_buffer *input);

/*
 * Structure definition for method functions
 */
struct grpc_c_method_funcs_s {
    grpc_c_method_data_pack_t   input_packer;        /* Input packer */
    grpc_c_method_data_unpack_t input_unpacker;    /* Input unpacker */
    grpc_c_method_data_pack_t   output_packer;        /* Output packer */
    grpc_c_method_data_unpack_t output_unpacker;    /* Output unpacker */
};

/*
 * Event structure to be used as tag when batching gRPC operations
 */
struct grpc_c_event_s {
    grpc_c_event_type_t type;   /* Type of this event */
    void              * data;    /* Data associated with this event */
    grpc_c_event_callback_t callback;
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
 * Definition for RPC method structure
 */
struct grpc_c_method_s {
    grpc_c_list_t list;
    void * method_tag;            /* Tag returned by grpc_server_register_method() */
    char * method_url;            /* URL for this RPC */
    int client_streaming;        /* Flag to indicate if client is streaming */
    int server_streaming;        /* Flag to indicate if server is streaming */
    void * handler;
    grpc_c_method_funcs_t * funcs;
};

typedef struct grpc_c_stream_write_s {
    grpc_c_event_t event;
    int stream;
    int count;
    int write_done;
    int write_wait;
    gpr_mu lock;
    gpr_cv cv;
    int write_result;
    grpc_byte_buffer * payload;
}grpc_c_stream_write_t;


typedef struct grpc_c_stream_read_s {
    grpc_c_event_t event;
    int stream;
    int count;
    int read_wait;
    gpr_mu lock;
    gpr_cv cv;
    uint32_t flags;
    grpc_byte_buffer * payload;
}grpc_c_stream_read_t;


typedef struct grpc_c_stream_status_s {
    int              is_client;
    grpc_c_event_t   event;
    int              result;
    gpr_cv           cv;
    gpr_mu           lock;
    grpc_c_metadata_array_t trailing_metadata;
    grpc_status_code status_code;
    grpc_slice       status_details;
}grpc_c_stream_status_t;

typedef struct grpc_c_initial_metadata_s {
    int                     is_send;
    int                     done_once;
    grpc_c_metadata_array_t metadata;
    grpc_c_event_t   event;
    int              result;
    gpr_mu           lock;
}grpc_c_initial_metadata_t;


typedef struct grpc_c_recv_close_s {
    int              client_cancel; /* Boolean indicating if client has cancelled the call */
    grpc_c_event_t   event;         /* Recv close grpc-c event in case of server context */
    gpr_mu           lock;
    gpr_cv           cv;
    int              result;
}grpc_c_recv_close_t;


/*
 * Structure definition for grpc_c client
 */
struct grpc_c_client_s {
    grpc_channel * channel;        /* Underlying grpc channel to host */
    grpc_completion_queue *queue;  /* Completion queue associated with this client */
    
    gpr_slice host;                /* Hostname of remote providing RPC service */
    grpc_c_state_t state;        /* Channel connectivity state */
    grpc_c_thread_pool_t  * thread_pool;
    
    int connect_status;            /* Connection status */
    grpc_c_event_t connect_event;
    int timeout;                /* Connection timeout flag */

    gpr_mu lock;                /* Mutex lock */
    gpr_cv shutdown_cv;            /* Shutdown condition variable */
    int shutdown;                /* Client shutdown flag */
};

/*
 * Structure definition for grpc-c context
 */
struct grpc_c_context_s {
    char       * method_url;
    gpr_timespec deadline;            /* Deadline for operations in this context */

    grpc_c_state_t state;            /* Current state of client/server */
    grpc_call *    call;            /* grpc_call for this RPC */
    gpr_mu         lock;            /* Mutex for access to this cq */
    gpr_cv         shutdown;
    grpc_c_method_funcs_t *method_funcs;    /* Pointer to method functions like input/output packer,unpacker, free and method callbacks */

    grpc_c_stream_status_t *status;
    grpc_c_stream_read_t   *reader;
    grpc_c_stream_write_t  *writer;
    
    grpc_c_initial_metadata_t * send_init_metadata;
    grpc_c_initial_metadata_t * recv_init_metadata;

    int is_client;
    
    union grpc_c_ctx_data {
        struct grpc_c_context_client_s {
            grpc_c_client_t           *client_t;
            void                      *tag;
            grpc_c_client_callback_t   callback;     /* Client callback */
        }client;
        struct grpc_c_context_server_s {
            grpc_c_method_t *method;
            grpc_c_event_t   event;            /* grpc-c event this context belongs to */
            grpc_c_server_t *server_t;
            grpc_c_service_callback_t callback;     /* RPC handler */
        }server;
    } type;
    
    grpc_c_list_t list; /* List of context objects */
};


/*
 * User provided memory alloc and free functions
 */
typedef void *(* grpc_c_memory_alloc_func_t)(grpc_c_context_t *context, size_t size);

typedef void (* grpc_c_memory_free_func_t)(grpc_c_context_t *context, void *data);

void grpc_c_set_memory_function(grpc_c_memory_alloc_func_t , grpc_c_memory_free_func_t );

void * grpc_malloc(size_t size);

void grpc_free(void *data);

void * grpc_realloc(void * ptr,size_t size);

ProtobufCAllocator * grpc_c_get_protobuf_c_allocator (grpc_c_context_t *context, ProtobufCAllocator *allocator);

/*
 * Server structure definition
 */
struct grpc_c_server_s {
    char * hostname;                      /* Server hostname */
    grpc_server * server;                  /* Grpc server */
    
    grpc_completion_queue * queue;          /* Server completion queue */
    grpc_c_thread_pool_t  * thread_pool;

    grpc_c_list_t method_list_head;
    grpc_c_list_t contexts_list_head;

    gpr_mu lock;                          /* Mutex lock */

    gpr_cv shutdown_cv;                      /* Shutdown condition variable */
    int    shutdown;                      /* Server shutting down */

    grpc_c_event_t shutdown_event;          /* Event signalling server shutdown */
};


/*
 * Initialize libgrpc-c to be used with given underlying libgrpc. Second
 * parameter is used to pass data to underlying library if it needs any
 */
int grpc_c_init(void);

/*
 * Shutsdown initialized grpc-c library. To be called towards end of program
 */
int grpc_c_shutdown(void);

/*
 * Control log output level.
 */
void grpc_c_log_output_level(int level);

/*
 * User using Interface
 */
int grpc_c_read(grpc_c_context_t *context, void **content, uint32_t flags, long timeout);

int grpc_c_write(grpc_c_context_t *context, void *output, uint32_t flags, long timeout);

int grpc_c_write_done(grpc_c_context_t *context, uint32_t flags, long timeout);

int grpc_c_finish(grpc_c_context_t *context, grpc_c_status_t *status, uint32_t flags);

/*
 * Initialize a client with client_id and server address
 */
grpc_c_client_t * grpc_c_client_init( const char *address, 
                                      grpc_channel_credentials *channel_creds,
                                      grpc_channel_args *channel_args,
                                      int thread_nums);

/*
 * Stop client.
 */
int grpc_c_client_stop(grpc_c_client_t *client);

/*
 * Waits for all callbacks to get done in a threaded client
 */
void grpc_c_client_wait (grpc_c_client_t *client);

/*
 * Destroy and free client object
 */
void grpc_c_client_free (grpc_c_client_t *client);

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
                                    const char *method_url,
                                    grpc_c_context_t **context,
                                    int client_streaming, int server_streaming,
                                    grpc_c_method_funcs_t * funcs,
                                    long timeout);

/*
 * Create a server object with given tcp/ip address
 */
grpc_c_server_t * grpc_c_server_create( const char *addr, grpc_server_credentials *creds, grpc_channel_args *args);


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
                            grpc_c_service_callback_t handler,
                            grpc_c_method_funcs_t * funcs);

/*
 * Initialize a metadata array
 */
void grpc_c_metadata_array_init(grpc_c_metadata_array_t *array); 

/*
 * Destroy a metadata array
 */
void grpc_c_metadata_array_destroy(grpc_c_metadata_array_t *array);

/*
 * Insert provided key value pair to given metadata array and storage list
 */
int grpc_c_metadata_array_add(grpc_c_metadata_array_t *mdarray, 
                               const char *key, const char *value);


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


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* GRPC_C_GRPC_C_H */
