#include "stream_ops.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*
 * Figure out deadline to finish the operation. A timeout of -1 will
 * block till we get event back or the operation fails before that
 */
gpr_timespec gc_deadline_from_timeout (long timeout)
{
    gpr_timespec deadline;

    if (timeout < 0) {
        deadline = gpr_inf_future(GPR_CLOCK_REALTIME);
    } else if (timeout == 0) {
        deadline = gpr_time_0(GPR_CLOCK_REALTIME);
    } else {
        deadline = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_millis(timeout, GPR_CLOCK_REALTIME));
    }

    return deadline;
}

/*
 * Sends available initial metadata. Returns 0 on success and 1 on failure.
 * This function will block caller
 */
int grpc_c_send_initial_metadata (grpc_c_context_t *context, long timeout)
{
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

