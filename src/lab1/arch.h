#ifndef ARCH
#define ARCH

#define BLOCK_SIZE 10
#define OK 1
#define WRONG_SYMBOL -1
#define SHORT_PASS -2
#define NO_SPEC_SYMBOL -3
#define NO_CAP_LET -4
#define NO_DIGIT -5 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>
#include <openssl/sha.h> 

typedef struct {
    char filename[1024];
    char path[1024];
    char otn_path[1024];
    long size;
}   FileHeader;

void hash_password(const char* password, unsigned char* hash_output);

void hash_to_string(const unsigned char* hash, char* hash_string);

void free_fileHeader(FileHeader*fileHeader);

void readFiles(const char* dirPath, FileHeader** files, int* alloc_FileHeader, int* index, const char* base_name, const char* relative_path);

int spec_symbol_in_pass(const char* pass, const char* spec_symbol);

int check_pass(const char* pass);

void create_arch(int index, char* path_for_arch, char* path_to_arch, FileHeader *files);
#endif