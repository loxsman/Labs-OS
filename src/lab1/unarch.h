#ifndef UNARCH
#define UNARCH


#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <openssl/sha.h>

typedef struct {
  char filename[1024];
  char path[1024];
  long size;
} FileHeader;

void hash_password(const char* password, unsigned char* hash_output);

void hash_to_string(const unsigned char* hash, char* hash_string);

void create_dir(const char* path);

void delete_file(const char* path);

int unarch(char* path_to_arch,char* path_for_unarch);

#endif