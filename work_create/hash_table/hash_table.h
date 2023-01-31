#ifndef __N_ELFHASH_TABLE_H__
#define __N_ELFHASH_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>

#define MAX_KEY_SIZE (512)
#define MAX_VAL_SIZE (4096)

typedef void *helfhash_table_t;

helfhash_table_t elfhash_table_create();
void elfhash_table_destroy(helfhash_table_t table);

int elfhash_insert(helfhash_table_t table, const char *key, uint32_t key_len, const void *value, uint32_t value_size);
int elfhash_find_and_delete(helfhash_table_t htable, const char *key, uint32_t key_len, void *out_value, uint32_t *out_value_size, bool is_ptr, bool is_delete);
int elfhash_find(helfhash_table_t table, const char *key, uint32_t key_len, void *out_value, uint32_t *out_value_size);
int elfhash_findx(helfhash_table_t htable, const char *key, uint32_t key_len, void *out_value_ptr, uint32_t *out_value_size);
int elfhash_delete(helfhash_table_t htable, const char *key, uint32_t key_len);
/*
void elfhash_dump(helfhash_table_t htable);
*/

#ifdef __cplusplus
}
#endif   /* end of __cplusplus */

#endif //__N_ELFHASH_TABLE_H__
