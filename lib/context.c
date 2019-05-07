#include <grpc-c/grpc-c.h>

#include "context.h"
#include "trace.h"
#include "metadata_array.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*
 * Create and initialize context
 */
grpc_c_context_t * grpc_c_context_init(grpc_c_method_t *method, int is_client)
{
    grpc_c_context_t *context = gpr_malloc(sizeof(grpc_c_context_t));
    if (context == NULL) {
		return NULL;
    }
	
    memset(context, 0, sizeof(grpc_c_context_t));
    
    context->gcc_method = method;
    context->gcc_deadline = gpr_inf_future(GPR_CLOCK_REALTIME);
    context->gcc_client_cancel = 1;
    context->gcc_metadata = gpr_malloc(sizeof(grpc_metadata_array));
	
    if (context->gcc_metadata == NULL) {
		gpr_log(GPR_ERROR, "Failed to allocate memory in context for metadata");
		grpc_c_context_free(context);
		return NULL;
    }
    grpc_metadata_array_init(context->gcc_metadata);

    context->gcc_initial_metadata = gpr_malloc(sizeof(grpc_metadata_array));
    if (context->gcc_initial_metadata == NULL) {
		gpr_log(GPR_ERROR, "Failed to allocate memory in context for "
			"initial metadata");
		grpc_c_context_free(context);
		return NULL;
    }
    grpc_metadata_array_init(context->gcc_initial_metadata);

    context->gcc_trailing_metadata = gpr_malloc(sizeof(grpc_metadata_array));
    if (context->gcc_trailing_metadata == NULL) {
		gpr_log(GPR_ERROR, "Failed to allocate memory in context for "
			"trailing metadata");
		grpc_c_context_free(context);
		return NULL;
    }
    grpc_metadata_array_init(context->gcc_trailing_metadata);
    
    return context;
}



/*
 * Free the context object
 */
void grpc_c_context_free (grpc_c_context_t *context)
{
    
    if (context->gcc_payload) 
		grpc_byte_buffer_destroy(context->gcc_payload);

    gpr_free(context);
}

/*
 * Extracts the value for a key from the metadata array. Returns NULL if given
 * key is not present
 */
int grpc_c_get_metadata_by_key(grpc_c_context_t *context, const char *key, char *value, size_t len)
{
    return 0;
}


/*
 * Returns value for given key fro initial metadata array
 */
int grpc_c_get_initial_metadata_by_key(grpc_c_context_t *context, const char *key, char *value, size_t len)
{	
    return 0;
}

/*
 * Returns value for given key from trailing metadata array
 */
int grpc_c_get_trailing_metadata_by_key(grpc_c_context_t *context, const char *key, char *value, size_t len)
{
    return 0;
}

/*
 * Adds given key value pair to metadata array. Returns 0 on success and 1 on
 * failure
 */
int grpc_c_add_metadata (grpc_c_context_t *context, const char *key, const char *value)
{
    return grpc_c_add_metadata_by_array(context->gcc_metadata, &context->gcc_metadata_storage, key, value);
}

/*
 * Adds given key value pair to initial metadata array. Returns 0 on success
 * and 1 on failure
 */
int grpc_c_add_initial_metadata (grpc_c_context_t *context, const char *key, const char *value)
{
    return grpc_c_add_metadata_by_array(context->gcc_initial_metadata, &context->gcc_initial_metadata_storage, key, value);
}

/*
 * Adds given key value pair to trailing metadata array. Returns 0 on success
 * and 1 on failure
 */
int grpc_c_add_trailing_metadata (grpc_c_context_t *context, const char *key, const char *value)
{
    return grpc_c_add_metadata_by_array(context->gcc_trailing_metadata, &context->gcc_trailing_metadata_storage, key, value);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

				  
