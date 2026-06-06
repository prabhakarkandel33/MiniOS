#include "ramfs.h"
#include "heap.h"
#include "terminal.h"
#include <stdint.h>

static ramfs_node_t* root = 0;
static uint32_t file_count = 0;

// -------------------------------------------------------
// simple string functions
// -------------------------------------------------------
static int fs_strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

static void fs_strcpy(char* dst, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

// -------------------------------------------------------
// initialize filesystem
// -------------------------------------------------------
void ramfs_init(void) {
    root       = 0;
    file_count = 0;
    // terminal_writestring("ramfs: initialized\n");
}

// -------------------------------------------------------
// create a file
// data can be NULL for empty file
// -------------------------------------------------------
ramfs_node_t* ramfs_create(const char* name, uint8_t* data, uint32_t size) {
    if (file_count >= RAMFS_MAX_FILES) {
        terminal_writestring("ramfs: max files reached\n");
        return 0;
    }

    // check for duplicate
    if (ramfs_find(name)) {
        terminal_writestring("ramfs: file exists: ");
        terminal_writestring(name);
        terminal_writestring("\n");
        return 0;
    }

    ramfs_node_t* node = (ramfs_node_t*)kmalloc(sizeof(ramfs_node_t));
    if (!node) return 0;

    fs_strcpy(node->name, name, RAMFS_NAME_MAX);
    node->size   = size;
    node->is_dir = 0;
    node->next   = 0;

    if (data && size > 0) {
        node->data = (uint8_t*)kmalloc(size);
        if (!node->data) { kfree(node); return 0; }
        for (uint32_t i = 0; i < size; i++)
            node->data[i] = data[i];
    } else {
        node->data = 0;
    }

    // add to linked list
    if (!root) {
        root = node;
    } else {
        ramfs_node_t* cur = root;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }

    file_count++;
    return node;
}

// -------------------------------------------------------
// find a file by name
// -------------------------------------------------------
ramfs_node_t* ramfs_find(const char* name) {
    ramfs_node_t* cur = root;
    while (cur) {
        if (fs_strcmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    }
    return 0;
}

// -------------------------------------------------------
// list all files
// -------------------------------------------------------
void ramfs_list(void) {
    if (!root) {
        terminal_writestring("(empty)\n");
        return;
    }
    ramfs_node_t* cur = root;
    while (cur) {
        terminal_writestring("  ");
        terminal_writestring(cur->name);
        terminal_writestring("  ");
        terminal_writehex(cur->size);
        terminal_writestring(" bytes\n");
        cur = cur->next;
    }
}

// -------------------------------------------------------
// delete a file
// -------------------------------------------------------
void ramfs_delete(const char* name) {
    if (!root) return;

    // handle head deletion
    if (fs_strcmp(root->name, name) == 0) {
        ramfs_node_t* old = root;
        root = root->next;
        if (old->data) kfree(old->data);
        kfree(old);
        file_count--;
        return;
    }

    ramfs_node_t* prev = root;
    ramfs_node_t* cur  = root->next;
    while (cur) {
        if (fs_strcmp(cur->name, name) == 0) {
            prev->next = cur->next;
            if (cur->data) kfree(cur->data);
            kfree(cur);
            file_count--;
            return;
        }
        prev = cur;
        cur  = cur->next;
    }
    terminal_writestring("ramfs: not found: ");
    terminal_writestring(name);
    terminal_writestring("\n");
}

int ramfs_write(const char* name, uint8_t* data, uint32_t size) {
    ramfs_node_t* node = ramfs_find(name);
    if (!node) {
        // file doesn't exist — create it
        return ramfs_create(name, data, size) ? 0 : -1;
    }

    // file exists — overwrite
    if (node->data) kfree(node->data);
    node->data = (uint8_t*)kmalloc(size);
    if (!node->data) return -1;
    for (uint32_t i = 0; i < size; i++)
        node->data[i] = data[i];
    node->size = size;
    return 0;
}

ramfs_node_t* ramfs_create_ref(const char* name, uint8_t* data, uint32_t size) {
    if (file_count >= RAMFS_MAX_FILES) return 0;
    if (ramfs_find(name)) return 0;

    ramfs_node_t* node = (ramfs_node_t*)kmalloc(sizeof(ramfs_node_t));
    if (!node) return 0;

    fs_strcpy(node->name, name, RAMFS_NAME_MAX);
    node->size   = size;
    node->is_dir = 0;
    node->next   = 0;
    node->data   = data;   // direct reference — no copy

    if (!root) {
        root = node;
    } else {
        ramfs_node_t* cur = root;
        while (cur->next) cur = cur->next;
        cur->next = node;
    }

    file_count++;
    return node;
}