
#include <stdio.h>
#include <stdlib.h>

#include "trace.h"
#include <grpc-c.h>
#include <grpc/support/alloc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*
 * Initializes japi library
 */
int grpc_c_init() {
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
int grpc_c_shutdown() {
  grpc_shutdown();

  return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
