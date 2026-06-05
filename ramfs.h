#ifndef RAMFS_H
#define RAMFS_H

#include <stdint.h>

#define RAMFS_NAME_MAX 64
#define RAMFS_MAX_FILES 32

typedef struct ramfs_node {
    char     name[RAMFS_NAME_MAX];
    uint8_t* data;
    uint32_t size;
    uint8_t  is_dir;
    struct ramfs_node* next;
} ramfs_node_t;

void          ramfs_init(void);
ramfs_node_t* ramfs_create(const char* name, uint8_t* data, uint32_t size);
ramfs_node_t* ramfs_find(const char* name);
void          ramfs_list(void);
void          ramfs_delete(const char* name);
int ramfs_write(const char* name, uint8_t* data, uint32_t size);
ramfs_node_t* ramfs_create_ref(const char* name, uint8_t* data, uint32_t size);


#endif