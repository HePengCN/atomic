#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "hash_table.h"
#include "com_log.h"

#define MAX_BUCKETS (1117)//(2903)//(1033)

typedef struct hash_node
{
    char key[MAX_KEY_SIZE];
    uint32_t key_size;
    uint8_t value[MAX_VAL_SIZE];
    uint32_t value_size;
    struct hash_node *next;
} hash_node_t;

typedef struct hash_table
{
    hash_node_t *node[MAX_BUCKETS];
    pthread_mutex_t mutex;
} hash_table_t;


static void free_node_list(hash_node_t *node)
{
    if (node == NULL) return;
    hash_node_t *tmp = node->next;
    free(node);
    if (tmp != NULL)
    {
        return free_node_list(tmp);
    }
}

static uint32_t ELFhash(const char *str, int len)
{
    unsigned int hash = 0;
    unsigned int x = 0;
    int i = 0;
    for (i = 0; i < len; i++)
    {
        hash = (hash << 4) + *str;
        if ((x = hash & 0xf0000000) != 0)
        {
            hash ^= (x >> 24);
            hash &= ~x;
        }
        str++;
    }
    return (hash & 0x7fffffff);
}

static uint32_t calc_index(const char *str, int len)
{
    return ELFhash(str, len) % MAX_BUCKETS;
}

helfhash_table_t elfhash_table_create()
{
    hash_table_t *table = (hash_table_t *)malloc(sizeof(hash_table_t));
    if (table == NULL)
    {
        COM_LOG_ERROR("malloc error.");
        return NULL;
    }
    memset(table, 0, sizeof(*table));
    pthread_mutex_init(&table->mutex, NULL);
    return table;
}

void elfhash_table_destroy(helfhash_table_t htable)
{
    if (htable == NULL) return;
    hash_table_t *table = htable;
    int i = 0;
    for (i = 0; i < MAX_BUCKETS; i++)
    {
        if (table->node[i] != NULL)
        {
            free_node_list(table->node[i]);
        }
    }
    pthread_mutex_destroy(&table->mutex);
    free(table);
}

int elfhash_insert(helfhash_table_t htable, const char *key, uint32_t key_len, const void *value, uint32_t value_size)
{
    assert(htable != NULL);
    assert(value_size <= MAX_VAL_SIZE);
    int ret = 0;
    hash_table_t *table = htable;
    pthread_mutex_lock(&(table->mutex));
    hash_node_t *new_node = (hash_node_t *)malloc(sizeof(hash_node_t));
    if (new_node == NULL)
    {
        ret = -1;
        COM_LOG_ERROR("malloc new hash node fail. key: %s\n", key);
        goto end;
    }

    memset(new_node, 0, sizeof(*new_node));
    memcpy(new_node->key, key, key_len);
    new_node->key_size = key_len;
    memcpy(new_node->value, value, value_size);
    new_node->value_size = value_size;

    int idx = calc_index(key, key_len);
    if (table->node[idx] == NULL)
    {
        table->node[idx] = new_node;
    }
    else
    {
        hash_node_t *p = table->node[idx];
        while (p->next != NULL)
        {
            p = p->next;
        }
        p->next = new_node;
    }
end:
    pthread_mutex_unlock(&(table->mutex));
    return ret;
}

int elfhash_find_and_delete(helfhash_table_t htable, const char *key, uint32_t key_len, void *out_value, uint32_t *out_value_size, bool is_ptr, bool is_delete)
{
    assert(htable != NULL);
    int ret = 0;
    hash_table_t *table = htable;
    pthread_mutex_lock(&(table->mutex));
    int idx = calc_index(key, key_len);
    if (table->node[idx] == NULL)
    {
        COM_LOG_DEBUG("INFO: Not Found in hash_table for key: %s\n", key);
        ret = -1;
        goto end;
    }

    hash_node_t *pre = table->node[idx];
    hash_node_t *node = table->node[idx];

    bool found  = false;
    while (!found)
    {
        if (node == NULL)
        {
            break;
        }

        if (node->key_size != key_len)
        {
            if (pre != node)
            {
                pre = pre->next;
            }

            node = node->next;
            continue;
        }

        if (memcmp(node->key, key, key_len) != 0)
        {
            if (pre != node)
            {
                pre = pre->next;
            }

            node = node->next;
            continue;
        }

        found = true;
    }

    if (!found)
    {
        ret = -1;
        goto end;
    }

    if (out_value != NULL)
    {
        if (is_ptr)
        {
            *(uint8_t **)out_value = node->value;
        }
        else
        {
            memcpy(out_value, node->value, node->value_size);
        }
    }
    if (out_value_size != NULL)
    {
        *out_value_size = node->value_size;
    }

    if (is_delete)
    {
        hash_node_t *tmp = node->next;
        if (pre == node)
        {
            table->node[idx] = NULL;
        }
        else
        {
            pre->next = tmp;
        }

        free(node);
    }

end:
    pthread_mutex_unlock(&(table->mutex));
    return ret;
}

int elfhash_find(helfhash_table_t htable, const char *key, uint32_t key_len, void *out_value, uint32_t *out_value_size)
{
    return elfhash_find_and_delete(htable, key, key_len, out_value, out_value_size, false, false);
}

int elfhash_findx(helfhash_table_t htable, const char *key, uint32_t key_len, void *out_value_ptr, uint32_t *out_value_size)
{
    return elfhash_find_and_delete(htable, key, key_len, out_value_ptr, out_value_size, true, false);
}

int elfhash_delete(helfhash_table_t htable, const char *key, uint32_t key_len)
{
    return elfhash_find_and_delete(htable, key, key_len, NULL, NULL, false, true);
}


/*
void elfhash_dump(helfhash_table_t htable)
{
    hash_table_t* table = htable;
    int i = 0;
    for (i = 0; i < MAX_BUCKETS; i++)
    {
        if (table->node[i] == NULL)
        {
            printf("table->node[%d]: NULL\n", i);
            continue;
        }

        printf("table->node[%d]: ", i);
        hash_node_t* node = table->node[i];
        while (node != NULL)
        {
            printf("node->key:%s ", node->key);
            node = node->next;
        }
        printf("\n");
    }
}
*/
