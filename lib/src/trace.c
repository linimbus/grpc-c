#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>

#include <grpc-c.h>
#include <grpc/support/log.h>

#include "trace.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static int grpc_c_output_level = 2;

void grpc_c_log(int level, const char *file, int line, const char *format,
                ...) {
  va_list args;
  const char *fname;
  char *rslash;
  const char level_name[3] = {'D', 'I', 'E'};
  char buffer[1024];
  int cnt;

  if (level < grpc_c_output_level) {
    return;
  }

  rslash = strrchr(file, '/');
  if (rslash == NULL) {
    fname = file;
  } else {
    fname = rslash + 1;
  }

  cnt = snprintf(buffer, sizeof(buffer) - 1, "[grpc-c]%c %s:%d ",
                 level_name[level], fname, line);
  if (cnt == -1) {
    return;
  }

  va_start(args, format);
  vsnprintf(&buffer[cnt], sizeof(buffer) - cnt - 1, format, args);
  va_end(args);

  fprintf(stderr, "%s\n", buffer);
}

void grpc_c_log_output_level(int level) { grpc_c_output_level = level; }

/*
 * Internal grpc-c trace callback that gets registered into grpc context
 */
void grpc_c_gpr_log(gpr_log_func_args *args) {
  int priority = 0;
  const char *fname;
  char *rslash;

  rslash = strrchr(args->file, '/');
  if (rslash == NULL) {
    fname = args->file;
  } else {
    fname = rslash + 1;
  }

  fprintf(stderr, "[grpc-core]%s %s:%d %s\n",
          gpr_log_severity_string(args->severity), fname, args->line,
          args->message);
}

/*
 * Enable/disable tracing by given flags
 */
void grpc_c_trace_enable_by_flag(int flags, int enabled) {
  if (flags & GRPC_C_TRACE_ALL) {
    grpc_tracer_set_enabled("all", enabled);
    return;
  }

  if (flags & GRPC_C_TRACE_TCP) {
    grpc_tracer_set_enabled("tcp", enabled);
  }

  if (flags & GRPC_C_TRACE_CHANNEL) {
    grpc_tracer_set_enabled("channel", enabled);
  }

  if (flags & GRPC_C_TRACE_SURFACE) {
    grpc_tracer_set_enabled("surface", enabled);
  }

  if (flags & GRPC_C_TRACE_HTTP) {
    grpc_tracer_set_enabled("http", enabled);
  }

  if (flags & GRPC_C_TRACE_FLOWCTL) {
    grpc_tracer_set_enabled("flowctl", enabled);
  }

  if (flags & GRPC_C_TRACE_BATCH) {
    grpc_tracer_set_enabled("batch", enabled);
  }

  if (flags & GRPC_C_TRACE_CONNECTIVITY_STATE) {
    grpc_tracer_set_enabled("connectivity_state", enabled);
  }

  if (flags & GRPC_C_TRACE_SECURE_ENDPOINT) {
    grpc_tracer_set_enabled("secure_endpoint", enabled);
  }

  if (flags & GRPC_C_TRACE_TRANSPORT_SECURITY) {
    grpc_tracer_set_enabled("transport_security", enabled);
  }

  if (flags & GRPC_C_TRACE_ROUND_ROBIN) {
    grpc_tracer_set_enabled("round_robin", enabled);
  }

  if (flags & GRPC_C_TRACE_HTTP_WRITE_STATE) {
    grpc_tracer_set_enabled("http_write_state", enabled);
  }

  if (flags & GRPC_C_TRACE_API) {
    grpc_tracer_set_enabled("api", enabled);
  }

  if (flags & GRPC_C_TRACE_CHANNEL_STACK_BUILDER) {
    grpc_tracer_set_enabled("channel_stack_builder", enabled);
  }

  if (flags & GRPC_C_TRACE_HTTP1) {
    grpc_tracer_set_enabled("http1", enabled);
  }

  if (flags & GRPC_C_TRACE_COMPRESSION) {
    grpc_tracer_set_enabled("compression", enabled);
  }

  if (flags & GRPC_C_TRACE_QUEUE_PLUCK) {
    grpc_tracer_set_enabled("queue_pluck", enabled);
  }

  if (flags & GRPC_C_TRACE_QUEUE_TIMEOUT) {
    grpc_tracer_set_enabled("queue_timeout", enabled);
  }

  if (flags & GRPC_C_TRACE_OP_FAILURE) {
    grpc_tracer_set_enabled("op_failure", enabled);
  }
}

/*
 * Enable tracing by flags
 */
void grpc_c_trace_enable(int flags, int severity) {
  grpc_c_trace_enable_by_flag(flags, 1);
  gpr_set_log_verbosity(severity);
}

/*
 * Disables tracing by flag
 */
void grpc_c_trace_disable(int flags) { grpc_c_trace_enable_by_flag(flags, 0); }

/*
 * Initializes tracing. Sets grpc-c trace function into grpc context
 */
void grpc_c_trace_init() { gpr_set_log_function(grpc_c_gpr_log); }

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
