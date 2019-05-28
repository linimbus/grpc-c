#ifndef GPRC_C_INTERNAL_METADATA_ARRAY_H
#define GRPC_C_INTERNAL_METADATA_ARRAY_H

#include <grpc-c/grpc-c.h>
#include "trace.h"

int grpc_c_metadata_array_get( grpc_c_metadata_array_t *mdarray, 
                               const char *key, char *value, size_t len );


#endif /* GRPC_C_INTERNAL_METADATA_ARRAY_H */
