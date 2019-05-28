
#ifndef GPRC_C_INTERNAL_STREAM_OPS_H
#define GPRC_C_INTERNAL_STREAM_OPS_H


#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <grpc/grpc.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/sync.h>
#include <grpc/support/time.h>

#include <grpc-c/grpc-c.h>

#include "context.h"
#include "metadata_array.h"

grpc_c_stream_read_t * grpc_c_stream_reader_init(int streaming);

void grpc_c_stream_reader_destory(grpc_c_stream_read_t * reader);

grpc_c_stream_write_t * grpc_c_stream_writer_init(int streaming);

void grpc_c_stream_writer_destory(grpc_c_stream_write_t * writer);


/*
 * Read handler. Returns data if already available or puts in a request for
 * data
 */
int grpc_c_stream_read (grpc_call *call, grpc_c_stream_read_t *reader, grpc_byte_buffer **output, uint32_t flags, long timeout);

/*
 * Sends given data into the stream. If previous write is still pending,
 * return GRPC_C_WRITE_PENDING
 */
int grpc_c_stream_write (grpc_call *call, grpc_c_stream_write_t * writer, grpc_byte_buffer *input, uint32_t flags, long timeout);

/*
 * Finishes write from client
 */
int grpc_c_stream_write_done (grpc_call *call, grpc_c_stream_write_t * writer, uint32_t flags, long timeout);

grpc_c_stream_status_t * grpc_c_status_init( int is_client );

void grpc_c_status_destory( grpc_c_stream_status_t * status );


/*
 * Finishes stream from client
 */
int grpc_c_status_send (grpc_call *call, grpc_c_stream_status_t * status, grpc_c_status_t *status_input, uint32_t flags);


/*
 * Finishes stream from server
 */
int grpc_c_status_recv (grpc_call *call, grpc_c_stream_status_t * status, grpc_c_status_t *status_output, uint32_t flags);


int grpc_c_status_trailing_metadata_set( grpc_c_stream_status_t * status, const char * key, const char * value );

int grpc_c_status_trailing_metadata_get( grpc_c_stream_status_t * status, const char * key, char * value, size_t len );


int grpc_c_initial_metadata_set( grpc_c_initial_metadata_t * init_metadata, const char * key, const char * value );

int grpc_c_initial_metadata_get( grpc_c_initial_metadata_t * init_metadata, const char * key, char * value, size_t len );


grpc_c_initial_metadata_t * grpc_c_initial_metadata_init( int is_send );

void grpc_c_initial_metadata_destory( grpc_c_initial_metadata_t * init_metadata );

int grpc_c_send_initial_metadata (grpc_call *call, grpc_c_initial_metadata_t * init_metadata, long timeout);

int grpc_c_recv_initial_metadata (grpc_call *call, grpc_c_initial_metadata_t * init_metadata, long timeout);




grpc_c_recv_close_t * grpc_c_server_recv_close_init();

void grpc_c_server_recv_close_destory( grpc_c_recv_close_t * recv_close );

int grpc_c_server_recv_close (grpc_call *call, grpc_c_recv_close_t * recv_close);

int grpc_c_server_recv_close_wait (grpc_c_recv_close_t * recv_close);



/*
 * Calculate timeout spec from millisecs
 */
gpr_timespec grpc_c_deadline_from_timeout (long timeout); 

#endif

