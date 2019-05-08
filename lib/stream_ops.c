#include "stream_ops.h"
#include "trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

gpr_timespec grpc_c_deadline_from_timeout (long timeout)
{
    gpr_timespec deadline;

    if (timeout < 0) {
        deadline = gpr_inf_future(GPR_CLOCK_MONOTONIC);
    } else if (timeout == 0) {
        deadline = gpr_time_0(GPR_CLOCK_MONOTONIC);
    } else {
        deadline = gpr_time_add(gpr_now(GPR_CLOCK_MONOTONIC), gpr_time_from_millis(timeout, GPR_CLOCK_MONOTONIC));
    }

    return deadline;
}

void grpc_c_stream_read_event_cb(grpc_c_event_t *event, int success) {

	grpc_c_stream_read_t *reader = (grpc_c_stream_read_t *)event->data;
	
	gpr_mu_lock(&reader->lock);
	reader->read_wait = 0;
	gpr_cv_signal(&reader->cv);
	gpr_mu_unlock(&reader->lock);
}

grpc_c_stream_read_t * grpc_c_stream_reader_init(int streaming)
{
	grpc_c_stream_read_t * reader;
	int ret;

	reader = (grpc_c_stream_read_t *)grpc_malloc(sizeof(grpc_c_stream_read_t));
	if ( NULL == reader ) {	
		GRPC_C_ERR("Nomemory.");
		return NULL;
	}

	memset(reader, 0, sizeof(grpc_c_stream_read_t));

	gpr_mu_init(&reader->lock);
	gpr_cv_init(&reader->cv);

	reader->event.type = GRPC_C_EVENT_READ;
	reader->event.data = (void *)reader;
	reader->event.callback = grpc_c_stream_read_event_cb;

	return reader;
}

void grpc_c_stream_reader_destory(grpc_c_stream_read_t * reader)
{
	if ( reader->payload ) {
		grpc_byte_buffer_destroy(reader->payload);
		reader->payload = NULL;
	}

	gpr_cv_destroy(&reader->cv);
	gpr_mu_destroy(&reader->lock);

	grpc_free(reader);
}


/*
 * Read handler. Returns data if already available or puts in a request for
 * data
 */
int grpc_c_stream_read (grpc_call *call,grpc_c_stream_read_t* reader, grpc_byte_buffer **output, uint32_t flags, long timeout)
{
	grpc_call_error error;
	grpc_op ops;
	int ret;

	gpr_mu_lock(&reader->lock);
	if ( !reader->stream && reader->count > 0 ) {
		gpr_mu_unlock(&reader->lock);
		
		GRPC_C_ERR("Nostream.");
		return GRPC_C_ERR_FAIL;
	}

	if ( reader->read_wait ) {
		ret = gpr_cv_wait(&reader->cv, &reader->lock, grpc_c_deadline_from_timeout(timeout));
		if ( ret )
		{
			gpr_mu_unlock(&reader->lock);
			
			GRPC_C_ERR("Timeout.");
			return GRPC_C_ERR_TMOUT;
		}
	}

	if ( reader->payload ) {
		*output = reader->payload;
		reader->payload = NULL;
		reader->count++;
	
		gpr_mu_unlock(&reader->lock);
		return GRPC_C_OK;
	}

	memset(&ops, 0, sizeof(grpc_op));

	ops.op = GRPC_OP_RECV_MESSAGE;
	ops.data.recv_message.recv_message = &reader->payload;

	reader->event.type = GRPC_C_EVENT_READ;
	reader->event.data = (void *)reader;
	reader->event.callback = grpc_c_stream_read_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&reader->event, NULL);
	if (error != GRPC_CALL_OK) {
		gpr_mu_unlock(&reader->lock);
	
		GRPC_C_ERR("Failed to finish read ops batch");
		return GRPC_C_ERR_FAIL;
    }

	reader->read_wait = 1;
	ret = gpr_cv_wait(&reader->cv, &reader->lock, grpc_c_deadline_from_timeout(timeout));
	if ( ret )
	{
		gpr_mu_unlock(&reader->lock);
		
		GRPC_C_ERR("Timeout.");
		return GRPC_C_ERR_TMOUT;
	}

	if ( reader->payload ) {
		*output = reader->payload;
		reader->payload = NULL;
		reader->count++;
		ret = GRPC_C_OK;
	}else {
		ret = GRPC_C_ERR_FAIL;
	}

	gpr_mu_unlock(&reader->lock);

	return ret;	
}


void grpc_c_stream_write_event_cb(grpc_c_event_t *event, int success) {

	grpc_c_stream_write_t *writer = (grpc_c_stream_write_t *)event->data;
	
	gpr_mu_lock(&writer->lock);
	writer->write_wait = 0;
	writer->write_result = (success > 0) ? GRPC_C_OK : GRPC_C_ERR_FAIL;
	gpr_cv_signal(&writer->cv);
	gpr_mu_unlock(&writer->lock);
}

grpc_c_stream_write_t * grpc_c_stream_writer_init(int streaming)
{
	grpc_c_stream_write_t * writer;
	int ret;

	writer = (grpc_c_stream_write_t *)grpc_malloc(sizeof(grpc_c_stream_write_t));
	if ( NULL == writer ) {	
		GRPC_C_ERR("Nomemory.");
		return NULL;
	}

	memset(writer, 0, sizeof(grpc_c_stream_write_t));

	gpr_mu_init(&writer->lock);
	gpr_cv_init(&writer->cv);

	writer->event.type = GRPC_C_EVENT_WRITE;
	writer->event.data = (void *)writer;
	writer->event.callback = grpc_c_stream_write_event_cb;

	return writer;
}

void grpc_c_stream_writer_destory(grpc_c_stream_write_t * writer)
{
	if ( writer->payload ) {
		grpc_byte_buffer_destroy(writer->payload);
		writer->payload = NULL;
	}

	gpr_cv_destroy(&writer->cv);
	gpr_mu_destroy(&writer->lock);

	grpc_free(writer);
}

/*
 * Sends given data into the stream.
 */
int grpc_c_stream_write (grpc_call *call,grpc_c_stream_write_t *writer, grpc_byte_buffer *input, uint32_t flags, long timeout)
{
	grpc_call_error error;
	grpc_op ops;
	int ret;

	gpr_mu_lock(&writer->lock);
	if ( !writer->stream && writer->count > 0 ) {
		gpr_mu_unlock(&writer->lock);
		GRPC_C_ERR("Nostream.");
		return GRPC_C_ERR_FAIL;
	}

	if ( writer->write_done ) {
		gpr_mu_unlock(&writer->lock);
		GRPC_C_ERR("Write Done.");
		return GRPC_C_ERR_FAIL;
	}

	if ( writer->write_wait ) {
		ret = gpr_cv_wait(&writer->cv, &writer->lock, grpc_c_deadline_from_timeout(timeout));
		if ( ret )
		{
			gpr_mu_unlock(&writer->lock);
			GRPC_C_ERR("Timeout.");
			return GRPC_C_ERR_TMOUT;
		}
	}

	if ( writer->payload ) {
		grpc_byte_buffer_destroy(writer->payload);
		writer->payload = NULL;
	}

	memset(&ops, 0, sizeof(grpc_op));

	writer->payload = input;
	ops.op    = GRPC_OP_SEND_MESSAGE;
	ops.flags = flags;
	ops.data.send_message.send_message = input;

	writer->event.type = GRPC_C_EVENT_WRITE;
	writer->event.data = (void *)writer;
	writer->event.callback = grpc_c_stream_write_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&writer->event, NULL);
	if (error != GRPC_CALL_OK) {
		gpr_mu_unlock(&writer->lock);
		GRPC_C_ERR("Failed to finish read ops batch");
		return GRPC_C_ERR_FAIL;
	}

	writer->write_wait   = 1;
	writer->write_result = 0;
	ret = gpr_cv_wait(&writer->cv, &writer->lock, grpc_c_deadline_from_timeout(timeout));
	if ( ret )
	{
		gpr_mu_unlock(&writer->lock);
		GRPC_C_ERR("Timeout.");
		return GRPC_C_ERR_TMOUT;
	}

	if ( !writer->write_result ) {
		ret = GRPC_C_OK;
	}else {
		ret = GRPC_C_ERR_FAIL;
	}

	if ( writer->payload ) {
		grpc_byte_buffer_destroy(writer->payload);
		writer->payload = NULL;
	}

	gpr_mu_unlock(&writer->lock);

	return ret; 
}

/*
 * Finishes write from client
 */
int grpc_c_stream_write_done (grpc_call *call,grpc_c_stream_write_t *writer, uint32_t flags, long timeout)
{
	grpc_call_error error;
	grpc_op ops;
	int ret;

	gpr_mu_lock(&writer->lock);
	
	if ( writer->write_done ) {
		gpr_mu_unlock(&writer->lock);
		return GRPC_C_OK;
	}

	memset(&ops, 0, sizeof(grpc_op));

	ops.op = GRPC_OP_SEND_CLOSE_FROM_CLIENT;

	writer->event.type = GRPC_C_EVENT_WRITE_DONE;
	writer->event.data = (void *)writer;
	writer->event.callback = grpc_c_stream_write_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&writer->event, NULL);
	if (error != GRPC_CALL_OK) {
		gpr_mu_unlock(&writer->lock);
		GRPC_C_ERR("Failed to finish read ops batch");
		return GRPC_C_ERR_FAIL;
	}

	writer->write_done   = 1;
	writer->write_result = 0;
	ret = gpr_cv_wait(&writer->cv, &writer->lock, grpc_c_deadline_from_timeout(timeout));
	if ( ret )
	{
		gpr_mu_unlock(&writer->lock);
		GRPC_C_ERR("Timeout.");
		return GRPC_C_ERR_TMOUT;
	}

	if ( !writer->write_result ) {
		ret = GRPC_C_OK;
	}else {
		ret = GRPC_C_ERR_FAIL;
	}

	gpr_mu_unlock(&writer->lock);

	return ret; 
}

void grpc_c_status_send_event_cb(grpc_c_event_t *event, int success)
{
	grpc_c_stream_status_t * status = (grpc_c_stream_status_t *)event->data;

	gpr_mu_lock(&status->lock);
	status->result  = (success > 0) ? GRPC_C_OK : GRPC_C_ERR_FAIL;
	gpr_cv_signal(&status->cv);
	gpr_mu_unlock(&status->lock);
}

grpc_c_stream_status_t * grpc_c_status_init( int is_client ) {
	grpc_c_stream_status_t * status;
	int ret;

	status = (grpc_c_stream_status_t *)grpc_malloc(sizeof(grpc_c_stream_status_t));
	if ( NULL == status ) {	
		GRPC_C_ERR("Nomemory.");
		return NULL;
	}

	memset(status, 0, sizeof(grpc_c_stream_status_t));

	gpr_mu_init(&status->lock);
	gpr_cv_init(&status->cv);

	status->event.data = (void *)status;
	status->is_client = is_client;

	grpc_metadata_array_init(&status->trailing_metadata);

	return status;
}

void grpc_c_status_destory( grpc_c_stream_status_t * status ) {

	if (!status->is_client) {
		int i;
		for( i = 0 ; i < status->trailing_metadata.count ; i++ ) {
			grpc_slice_unref(status->trailing_metadata.metadata[i].key);
			grpc_slice_unref(status->trailing_metadata.metadata[i].value);
		}
	}

	gpr_mu_destroy(&status->lock);
	gpr_cv_destroy(&status->cv);

	grpc_metadata_array_destroy(&status->trailing_metadata);

	grpc_free(status);
}

int grpc_c_status_trailing_metadata_set( grpc_c_stream_status_t * status, const char * key, char * value ) {

	return GRPC_C_OK;
}

int grpc_c_status_trailing_metadata_get( grpc_c_stream_status_t * status, const char * key, char * value, size_t len ) {

	return GRPC_C_OK;
}


/*
 * Finishes stream from client
 */
int grpc_c_status_send (grpc_call *call, grpc_c_stream_status_t * status, grpc_c_status_t *status_input, uint32_t flags)
{
	grpc_call_error error;
	grpc_op ops;
	int ret;

	gpr_mu_lock(&status->lock);

	status->status_code = status_input->code;
	if (status_input->message[0] == '\0') {
		status->status_details = grpc_empty_slice();
	}else {
		status->status_details = grpc_slice_from_static_string(status_input->message);
	}

	memset(&ops, 0, sizeof(grpc_op));

	ops.op = GRPC_OP_SEND_STATUS_FROM_SERVER;
	ops.data.send_status_from_server.status = status->status_code;
	ops.data.send_status_from_server.status_details = &status->status_details;
	ops.data.send_status_from_server.trailing_metadata_count = status->trailing_metadata.count;
	ops.data.send_status_from_server.trailing_metadata = status->trailing_metadata.metadata;
	
	status->event.type     = GRPC_C_EVENT_FINISH;
	status->event.data     = status;
	status->event.callback = grpc_c_status_send_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&status->event, NULL);
	if (error != GRPC_CALL_OK) {
		gpr_mu_unlock(&status->lock);
		GRPC_C_ERR("Failed to finish send status ops batch");
		return GRPC_C_ERR_FAIL;
	}

	(void)gpr_cv_wait(&status->cv, &status->lock, grpc_c_deadline_from_timeout(-1));

	if ( !status->result ) {
		ret = GRPC_C_OK;
	}else {
		ret = GRPC_C_ERR_FAIL;
	}

	gpr_mu_unlock(&status->lock);

	return GRPC_C_OK;
}

void grpc_c_status_recv_event_cb(grpc_c_event_t *event, int success)
{
	grpc_c_stream_status_t * status = (grpc_c_stream_status_t *)event->data;

	gpr_mu_lock(&status->lock);
	status->result  = (success > 0) ? GRPC_C_OK : GRPC_C_ERR_FAIL;
	gpr_cv_signal(&status->cv);
	gpr_mu_unlock(&status->lock);
}

/*
 * Finishes stream from server
 */
int grpc_c_status_recv (grpc_call *call, grpc_c_stream_status_t * status, grpc_c_status_t *status_output, uint32_t flags)
{
	grpc_call_error error;
	grpc_op ops;
	int ret;

	gpr_mu_lock(&status->lock);

	memset(&ops, 0, sizeof(grpc_op));

	ops.op = GRPC_OP_RECV_STATUS_ON_CLIENT;
	ops.data.recv_status_on_client.status         = &status->status_code;
	ops.data.recv_status_on_client.status_details = &status->status_details;
	ops.data.recv_status_on_client.trailing_metadata = &status->trailing_metadata;

	status->event.type     = GRPC_C_EVENT_FINISH;
	status->event.data     = status;
	status->event.callback = grpc_c_status_recv_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&status->event, NULL);
	if (error != GRPC_CALL_OK) {
		gpr_mu_unlock(&status->lock);
		GRPC_C_ERR("Failed to finish send status ops batch");
		return GRPC_C_ERR_FAIL;
	}

	(void)gpr_cv_wait(&status->cv, &status->lock, grpc_c_deadline_from_timeout(-1));

	if ( !status->result ) {
		char * temp;

		status_output->code = status->status_code;
		temp = grpc_slice_to_c_string(status->status_details);

		strncpy(status_output->message, temp, sizeof(status_output->message) - 1);
		status_output->message[sizeof(status_output->message) - 1] = '\0';

		gpr_free(temp);

		ret = GRPC_C_OK;
	}else {
		ret = GRPC_C_ERR_FAIL;
	}

	gpr_mu_unlock(&status->lock);

	return ret;
}

int grpc_c_initial_metadata_set( grpc_c_initial_metadata_t * init_metadata, const char * key, char * value ) {

	return GRPC_C_OK;
}

int grpc_c_initial_metadata_get( grpc_c_initial_metadata_t * init_metadata, const char * key, char * value, size_t len ) {

	return GRPC_C_OK;
}


grpc_c_initial_metadata_t * grpc_c_initial_metadata_init( int is_send ) {
	grpc_c_initial_metadata_t * init_metadata;
	int ret;

	init_metadata = (grpc_c_initial_metadata_t *)grpc_malloc(sizeof(grpc_c_initial_metadata_t));
	if ( NULL == init_metadata ) {	
		GRPC_C_ERR("Nomemory.");
		return NULL;
	}

	memset(init_metadata, 0, sizeof(grpc_c_initial_metadata_t));

	gpr_mu_init(&init_metadata->lock);

	init_metadata->result     = GRPC_C_ERR_NORECV;
	init_metadata->event.data = (void *)init_metadata;
	init_metadata->is_send    = is_send;

	grpc_metadata_array_init(&init_metadata->metadata);

	return init_metadata;
}


void grpc_c_initial_metadata_destory( grpc_c_initial_metadata_t * init_metadata ) {

	if ( init_metadata->is_send ) {
		int i;
		for( i = 0 ; i < init_metadata->metadata.count ; i++ ) {
			grpc_slice_unref(init_metadata->metadata.metadata[i].key);
			grpc_slice_unref(init_metadata->metadata.metadata[i].value);
		}
	}

	gpr_mu_destroy(&init_metadata->lock);

	grpc_metadata_array_destroy(&init_metadata->metadata);

	grpc_free(init_metadata);
}


void grpc_c_send_initial_metadata_event_cb(grpc_c_event_t *event, int success)
{
	grpc_c_initial_metadata_t * init_metadata = (grpc_c_initial_metadata_t *)event->data;

	gpr_mu_lock(&init_metadata->lock);
	init_metadata->result  = (success > 0) ? GRPC_C_OK : GRPC_C_ERR_FAIL;
	gpr_mu_unlock(&init_metadata->lock);
	
}

int grpc_c_send_initial_metadata (grpc_call *call, grpc_c_initial_metadata_t * init_metadata, long timeout)
{
	grpc_call_error error;
	grpc_op ops;

	gpr_mu_lock(&init_metadata->lock);

	if ( init_metadata->done_once ) {
		gpr_mu_unlock(&init_metadata->lock);
		return GRPC_C_OK;
	}

	memset(&ops, 0, sizeof(grpc_op));

	ops.op = GRPC_OP_SEND_INITIAL_METADATA;
	ops.data.send_initial_metadata.metadata = init_metadata->metadata.metadata;
	ops.data.send_initial_metadata.count    = init_metadata->metadata.count;

	init_metadata->event.type	   = GRPC_C_EVENT_SEND_METADATA;
	init_metadata->event.data	   = init_metadata;
	init_metadata->event.callback  = grpc_c_send_initial_metadata_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&init_metadata->event, NULL);
	if (error != GRPC_CALL_OK) {
		gpr_mu_unlock(&init_metadata->lock);
		GRPC_C_ERR("Failed to finish send status ops batch");
		return GRPC_C_ERR_FAIL;
	}

	init_metadata->done_once = 1;

	gpr_mu_unlock(&init_metadata->lock);

	return GRPC_C_OK;
}


void grpc_c_recv_initial_metadata_event_cb(grpc_c_event_t *event, int success)
{
	grpc_c_initial_metadata_t * init_metadata = (grpc_c_initial_metadata_t *)event->data;

	gpr_mu_lock(&init_metadata->lock);
	init_metadata->result  = (success > 0) ? GRPC_C_OK : GRPC_C_ERR_FAIL;
	gpr_mu_unlock(&init_metadata->lock);
}


int grpc_c_recv_initial_metadata (grpc_call *call, grpc_c_initial_metadata_t * init_metadata, long timeout)
{
	grpc_call_error error;
	grpc_op ops;

	gpr_mu_lock(&init_metadata->lock);

	if ( init_metadata->done_once ) {
		gpr_mu_unlock(&init_metadata->lock);
		return GRPC_C_OK;
	}

	memset(&ops, 0, sizeof(grpc_op));

	ops.op = GRPC_OP_RECV_INITIAL_METADATA;
	ops.data.recv_initial_metadata.recv_initial_metadata = &init_metadata->metadata;

	init_metadata->event.type	   = GRPC_C_EVENT_RECV_METADATA;
	init_metadata->event.data	   = init_metadata;
	init_metadata->event.callback  = grpc_c_recv_initial_metadata_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&init_metadata->event, NULL);
	if (error != GRPC_CALL_OK) {
		gpr_mu_unlock(&init_metadata->lock);
		GRPC_C_ERR("Failed to finish send status ops batch");
		return GRPC_C_ERR_FAIL;
	}

	init_metadata->done_once = 1;

	gpr_mu_unlock(&init_metadata->lock);

	return GRPC_C_OK;
}

grpc_c_recv_close_t * grpc_c_server_recv_close_init() {
	grpc_c_recv_close_t * recv_close;
	int ret;

	recv_close = (grpc_c_recv_close_t *)grpc_malloc(sizeof(grpc_c_recv_close_t));
	if ( NULL == recv_close ) {	
		GRPC_C_ERR("Nomemory.");
		return NULL;
	}

	memset(recv_close, 0, sizeof(grpc_c_recv_close_t));

	gpr_mu_init(&recv_close->lock);
	gpr_cv_init(&recv_close->cv);

	recv_close->event.data = recv_close;
	recv_close->result = GRPC_C_ERR_NORECV;

	return recv_close;
}

void grpc_c_server_recv_close_destory( grpc_c_recv_close_t * recv_close ) {
	gpr_mu_destroy(&recv_close->lock);
	gpr_cv_destroy(&recv_close->cv);
	grpc_free(recv_close);
}

void grpc_c_server_recv_close_event_cb(grpc_c_event_t *event, int success) {
	grpc_c_recv_close_t *recv_close = (grpc_c_recv_close_t *)event->data;

	gpr_mu_lock(&recv_close->lock);
	gpr_cv_signal(&recv_close->cv);
	recv_close->result = (success > 0) ? GRPC_C_OK : GRPC_C_ERR_FAIL;
	gpr_mu_unlock(&recv_close->lock);
}


int grpc_c_server_recv_close (grpc_call *call, grpc_c_recv_close_t * recv_close)
{
	grpc_call_error error;
	grpc_op ops;

	memset(&ops, 0, sizeof(grpc_op));

	ops.op = GRPC_OP_RECV_CLOSE_ON_SERVER;
	ops.data.recv_close_on_server.cancelled = &recv_close->client_cancel;

	recv_close->event.type     = GRPC_C_EVENT_RECV_CLOSE;
	recv_close->event.data     = recv_close;
	recv_close->event.callback = grpc_c_server_recv_close_event_cb;

	error = grpc_call_start_batch(call, &ops, 1, (void *)&recv_close->event, NULL);
	if (error != GRPC_CALL_OK) {

		GRPC_C_ERR("Failed to recv close ops batch");
		return GRPC_C_ERR_FAIL;
	}

	return GRPC_C_OK;
}

int grpc_c_server_recv_close_wait (grpc_c_recv_close_t * recv_close)
{
	gpr_mu_lock(&recv_close->lock);

	if (recv_close->result == GRPC_C_ERR_NORECV) {
		(void)gpr_cv_wait(&recv_close->cv, &recv_close->lock, grpc_c_deadline_from_timeout(-1));
	}

	gpr_mu_unlock(&recv_close->lock);

	return GRPC_C_OK;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

