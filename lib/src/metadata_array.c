#include "metadata_array.h"
#include <grpc-c.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*
 * Initializes metadata array
 */
void grpc_c_metadata_array_init(grpc_c_metadata_array_t *array) {
  grpc_metadata_array_init(array);
}

/*
 * Destroys metadata array after destroying metadata
 */
void grpc_c_metadata_array_destroy(grpc_c_metadata_array_t *array) {
  /*
   * Free metadata keyvalue pairs
   */
  while (array->count > 0) {
    grpc_slice_unref(array->metadata[array->count - 1].key);
    grpc_slice_unref(array->metadata[array->count - 1].value);
    array->count -= 1;
  }
  grpc_metadata_array_destroy(array);
}

int grpc_c_metadata_array_get(grpc_c_metadata_array_t *mdarray, const char *key,
                              char *value, size_t len) {
  int i;
  char *value_src;

  if (key == NULL || value == NULL || len == 0) {
    GRPC_C_ERR("Invalid key or value");
    return GRPC_C_ERR_FAIL;
  }

  for (i = 0; i < mdarray->count; i++) {

    if (grpc_slice_str_cmp(mdarray->metadata[i].key, key)) {
      continue;
    }

    value_src = grpc_slice_to_c_string(mdarray->metadata[i].value);
    strncpy(value, value_src, len);
    gpr_free(value_src);

    return GRPC_C_OK;
  }

  return GRPC_C_ERR_FAIL;
}

/*
 * Inserts given keyvalue pair into metadata array. Returns 0 on success and 1
 * on failure
 */
int grpc_c_metadata_array_add(grpc_c_metadata_array_t *mdarray, const char *key,
                              const char *value) {
  if (key == NULL || value == NULL) {
    GRPC_C_ERR("Invalid key or value");
    return GRPC_C_ERR_FAIL;
  }

  /*
   * Make space to hold metada
   */
  mdarray->capacity += 1;
  mdarray->count += 1;
  if (mdarray->metadata != NULL) {
    mdarray->metadata = gpr_realloc(mdarray->metadata,
                                    mdarray->capacity * sizeof(grpc_metadata));
  } else {
    mdarray->metadata = gpr_malloc(sizeof(grpc_metadata));
  }

  if (mdarray->metadata == NULL) {
    GRPC_C_ERR("Failed to (re)allocate memory for metadata");
    return GRPC_C_ERR_NOMEM;
  }

  mdarray->metadata[mdarray->count - 1].key =
      grpc_slice_from_copied_string(key);
  mdarray->metadata[mdarray->count - 1].value =
      grpc_slice_from_copied_string(value);

  return GRPC_C_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
