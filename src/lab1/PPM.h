// PPM.h
#ifndef PPM_H
#define PPM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

typedef struct Node {
    unsigned char symbol;
    int count;
    struct Node *next;
} Node;

typedef struct Context {
    Node *root;
} Context;

// Прототипы функций
Node *create_node(unsigned char symbol);
void init_context(Context *ctx);
Node *find_node(Context *ctx, unsigned char symbol);
void update_context(Context *ctx, unsigned char symbol);
void free_context(Context *ctx);
void create_directory_if_needed(const char *dir_path);
void compress_ppm(const char *input_file, const char *compressed_file, mode_t mode, uid_t uid, gid_t gid);
void compress_directory(const char *input_dir_name, const char *output_dir_name);
void decompress_ppm(const char *compressed_file, const char *output_file);
void decompress_directory(const char *src_dir, const char *dest_dir);

#endif // PPM_H
