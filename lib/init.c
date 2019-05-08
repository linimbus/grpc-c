
#include <stdio.h>
#include <stdlib.h>

#include <grpc-c/grpc-c.h>
#include <grpc/support/alloc.h>
#include "trace.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*
 * Initializes japi library
 */
int grpc_c_init () 
{
	gpr_allocation_functions func = {grpc_malloc,NULL,grpc_realloc,grpc_free};

	gpr_set_allocation_functions(func);

	/*
	 * Initialize trace functions
	 */
	grpc_c_trace_init();

	/*
	 * Initialize grpc
	 */
	grpc_init();

	return 0;
}

/*
 * Cleans up and shutsdown japi library
 */
int grpc_c_shutdown ()
{
	grpc_shutdown();

	return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

