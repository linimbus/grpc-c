/*
 * Copyright (c) 2016, Juniper Networks, Inc.
 * All rights reserved.
 */

#include <grpc-c/grpc-c.h>


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */






void * grpc_c_alloc()
{

}

void grpc_c_free()
{

}


/*
 * User provided allocate and free functions
 */
static grpc_c_memory_alloc_func_t grpc_c_alloc_func = NULL;
static grpc_c_memory_free_func_t  groc_c_free_func = NULL;

/*
 * Signatures for protobuf allocate and free function callbacks
 */
typedef void *(protobuf_c_alloc_func_t)(void *allocator_data, size_t size);
typedef void (protobuf_c_free_func_t)(void *allocator_data, void *data);

/*
 * Sets allocate function callback
 */
void grpc_c_set_memory_function( grpc_c_memory_alloc_func_t allocfunc, grpc_c_memory_free_func_t freefunc)
{
    grpc_c_alloc_func = allocfunc;
    groc_c_free_func  = freefunc;
}


/*
 * Takes pointer to allocator and fills alloc, free functions if available.
 * Else return NULL
 */
ProtobufCAllocator * grpc_c_get_protobuf_c_allocator(grpc_c_context_t *context, ProtobufCAllocator *allocator)
{
    if (grpc_c_alloc_func && groc_c_free_func && allocator) {

		allocator->alloc = (protobuf_c_alloc_func_t *)grpc_c_alloc_func;
		allocator->free  = (protobuf_c_free_func_t *)groc_c_free_func;

		if (context) {
			allocator->allocator_data = (void *)context;
		} else {
			allocator->allocator_data = NULL;
		}

		return allocator;
    }

    return NULL;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
