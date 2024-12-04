#include "PPM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CONTEXT 4
#define ALPHABET_SIZE 256

// Функция для создания нового узла
Node *create_node(unsigned char symbol) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->symbol = symbol;
    new_node->next = NULL;
    return new_node;
}

// Инициализация контекста
void init_context(Context *ctx) {
    ctx->root = NULL;
}

// Поиск узла по символу
Node *find_node(Context *ctx, unsigned char symbol) {
    Node *current = ctx->root;
    while (current != NULL) {
        if (current->symbol == symbol) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Обновление контекста
void update_context(Context *ctx, unsigned char symbol) {
    Node *new_node = create_node(symbol);
    new_node->next = ctx->root;
    ctx->root = new_node;
}

// Освобождение памяти, занятой контекстом
void free_context(Context *ctx) {
    Node *current = ctx->root;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
    ctx->root = NULL;
}

// Функция для создания директории
void create_directory_if_needed(const char *dir_path) {
    struct stat st = {0};
    if (stat(dir_path, &st) == -1) {
        if (mkdir(dir_path, 0700) == -1) {
            perror("Ошибка создания директории");
            exit(EXIT_FAILURE);
        }
    }
}

void compress_ppm(const char *input_file, const char *compressed_file, mode_t mode, uid_t uid, gid_t gid) {
    printf("Открытие файла: %s для чтения\n", input_file);
    printf("Создание файла: %s для записи\n", compressed_file);

    FILE *in = fopen(input_file, "rb");
    if (!in) {
        perror("Ошибка открытия входного файла");
        exit(EXIT_FAILURE);
    }

    FILE *out = fopen(compressed_file, "wb");
    if (!out) {
        perror("Ошибка открытия выходного файла");
        fclose(in); // Закрываем входной файл, если выходной открыть не удалось
        exit(EXIT_FAILURE);
    }

    Context contexts[MAX_CONTEXT];
    for (int i = 0; i < MAX_CONTEXT; i++) {
        init_context(&contexts[i]);
    }

    unsigned char context[MAX_CONTEXT] = {0};
    int context_len = 0;

    fwrite(&mode, sizeof(mode), 1, out);
    fwrite(&uid, sizeof(uid), 1, out);
    fwrite(&gid, sizeof(gid), 1, out);

    unsigned char buffer;
    while (fread(&buffer, 1, 1, in) == 1) {
        double cumulative_prob = 0.0;

        for (int i = 0; i < ALPHABET_SIZE; i++) {
            cumulative_prob += 1.0 / ALPHABET_SIZE;
            if (buffer == (unsigned char)i) {
                fwrite(&cumulative_prob, sizeof(cumulative_prob), 1, out);
                break;
            }
        }

        for (int i = 0; i < MAX_CONTEXT; i++) {
            if (i < context_len) {
                update_context(&contexts[i], buffer);
            }
        }

        memmove(context, context + 1, MAX_CONTEXT - 1);
        context[MAX_CONTEXT - 1] = buffer;
        if (context_len < MAX_CONTEXT) {
            context_len++;
        }
    }

    for (int i = 0; i < MAX_CONTEXT; i++) {
        free_context(&contexts[i]);
    }

    fclose(in);
    fclose(out);
}


// Функция для обработки всех файлов в директории
void compress_directory(const char *input_dir_name, const char *output_dir_name) {
    DIR *dir = opendir(input_dir_name);
    if (!dir) {
        perror("Ошибка открытия директории");
        return;
    }

    create_directory_if_needed(output_dir_name);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем текущую и родительскую директорию
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char input_path[1024];
        snprintf(input_path, sizeof(input_path), "%s/%s", input_dir_name, entry->d_name);

        char output_path[1024];
        snprintf(output_path, sizeof(output_path), "%s/%s.compressed", output_dir_name, entry->d_name);

        struct stat statbuf;
        if (stat(input_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                // Если это директория, создаем выходную директорию для вложенной папки
                char nested_output_dir[1024];
                snprintf(nested_output_dir, sizeof(nested_output_dir), "%s/%s.comp", output_dir_name, entry->d_name);
                create_directory_if_needed(nested_output_dir);  // Создаем директорию, если ее нет

                // Рекурсивно обрабатываем директорию
                compress_directory(input_path, nested_output_dir);
            } else if (S_ISREG(statbuf.st_mode)) {
                // Если это обычный файл, сжимаем его
                compress_ppm(input_path, output_path, statbuf.st_mode, statbuf.st_uid, statbuf.st_gid);
                printf("Сжат: %s -> %s\n", input_path, output_path);
            }
        }
    }

    closedir(dir);
}


// Функция декомпрессии одного файла
void decompress_ppm(const char *compressed_file, const char *output_file) {
    FILE *in = fopen(compressed_file, "rb");
    FILE *out = fopen(output_file, "wb");

    if (!in || !out) {
        perror("Ошибка открытия файла");
        exit(EXIT_FAILURE);
    }

    Context contexts[MAX_CONTEXT];
    for (int i = 0; i < MAX_CONTEXT; i++) {
        init_context(&contexts[i]);
    }

    unsigned char context[MAX_CONTEXT] = {0};
    int context_len = 0;

    mode_t mode;
    uid_t uid;
    gid_t gid;
    fread(&mode, sizeof(mode), 1, in);
    fread(&uid, sizeof(uid), 1, in);
    fread(&gid, sizeof(gid), 1, in);

    double probability;
    while (fread(&probability, sizeof(probability), 1, in) == 1) {
        unsigned char symbol = 0;
        double cumulative_prob = 0.0;

        for (int i = 0; i < ALPHABET_SIZE; i++) {
            cumulative_prob += 1.0 / ALPHABET_SIZE;
            if (cumulative_prob >= probability) {
                symbol = (unsigned char)i;
                break;
            }
        }

        fwrite(&symbol, 1, 1, out);

        for (int i = 0; i < MAX_CONTEXT; i++) {
            if (i < context_len) {
                update_context(&contexts[i], symbol);
            }
        }

        memmove(context, context + 1, MAX_CONTEXT - 1);
        context[MAX_CONTEXT - 1] = symbol;
        if (context_len < MAX_CONTEXT) {
            context_len++;
        }
    }

    for (int i = 0; i < MAX_CONTEXT; i++) {
        free_context(&contexts[i]);
    }

    fclose(in);
    fclose(out);
    chmod(output_file, mode);
    chown(output_file, uid, gid);
}

// Рекурсивная декомпрессия директории
void decompress_directory(const char *src_dir, const char *dest_dir) {
    DIR *dir = opendir(src_dir);
    if (!dir) {
        perror("Ошибка открытия директории");
        return;
    }

    // Создаём целевую директорию, если её нет
    mkdir(dest_dir, 0755);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем текущую и родительскую директорию
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char input_path[1024], output_path[1024];
        snprintf(input_path, sizeof(input_path), "%s/%s", src_dir, entry->d_name);
        snprintf(output_path, sizeof(output_path), "%s/%s", dest_dir, entry->d_name);

        struct stat statbuf;
        if (stat(input_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                char nested_output_dir[1024];
                snprintf(nested_output_dir, sizeof(nested_output_dir), "%s/%s", dest_dir, entry->d_name);
                decompress_directory(input_path, nested_output_dir);
            } else if (S_ISREG(statbuf.st_mode)) {
                snprintf(output_path, sizeof(output_path), "%s/%s.decomp", dest_dir, entry->d_name);
                decompress_ppm(input_path, output_path);
                printf("Разархивирован: %s -> %s\n", input_path, output_path);
            }
        }
    }

    closedir(dir);
}


