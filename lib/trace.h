#ifndef GRPC_C_INTERNAL_TRACE_H
#define GRPC_C_INTERNAL_TRACE_H


void grpc_c_log(int level, const char *file, int line, const char * format, ...);

#define GRPC_C_DBG(format,args...) grpc_c_log(0,__FILE__,__LINE__, format, ##args)

#define GRPC_C_INF(format,args...) grpc_c_log(1,__FILE__,__LINE__, format, ##args)

#define GRPC_C_ERR(format,args...) grpc_c_log(2,__FILE__,__LINE__, format, ##args)


/*
 * Initialize tracing
 */
void grpc_c_trace_init(void);


#endif /* GRPC_C_INTERNAL_TRACE_H */
