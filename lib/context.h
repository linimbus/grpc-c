#ifndef GPRC_C_INTERNAL_CONTEXT_H
#define GRPC_C_INTERNAL_CONTEXT_H

#include <grpc-c/grpc-c.h>

/*
 * Allocate and initialize context object
 */
grpc_c_context_t *grpc_c_context_init (grpc_c_method_t *method, int is_client);

/*
 * Destroy and free context
 */
void grpc_c_context_free (grpc_c_context_t *context);


#endif /* GRPC_C_INTERNAL_CONTEXT_H */
