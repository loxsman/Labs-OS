#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_TREE_HT 256

typedef struct {
    char file_name[256];
    unsigned long file_size;
} HeaderEntry;


// Структура узла Хаффмана
typedef struct MinHeapNode {
    unsigned char data;
    unsigned freq;
    struct MinHeapNode *left, *right;
} MinHeapNode;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    MinHeapNode** array;
} MinHeap;

MinHeapNode* newNode(unsigned char data, unsigned freq) {
    MinHeapNode* temp = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

MinHeap* createMinHeap(unsigned capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode**)malloc(minHeap->capacity * sizeof(MinHeapNode*));
    return minHeap;
}

void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

MinHeapNode* extractMin(MinHeap* minHeap) {
    MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap* minHeap, MinHeapNode* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

int isLeaf(MinHeapNode* root) {
    return !(root->left) && !(root->right);
}

MinHeap* createAndBuildMinHeap(unsigned char data[], int freq[], int size) {
    MinHeap* minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(data[i], freq[i]);
    minHeap->size = size;
    for (int i = (minHeap->size - 2) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
    return minHeap;
}

MinHeapNode* buildHuffmanTree(unsigned char data[], int freq[], int size) {
    MinHeapNode *left, *right, *top;
    MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);

    while (minHeap->size != 1) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = newNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }

    return extractMin(minHeap);
}

void printCodes(MinHeapNode* root, int arr[], int top, int huffmanTable[MAX_TREE_HT][256]) {
    if (root->left) {
        arr[top] = 0;
        printCodes(root->left, arr, top + 1, huffmanTable);
    }
    if (root->right) {
        arr[top] = 1;
        printCodes(root->right, arr, top + 1, huffmanTable);
    }
    if (isLeaf(root)) {
        for (int i = 0; i < top; i++) {
            huffmanTable[root->data][i] = arr[i];
        }
        huffmanTable[root->data][top] = -1; // Маркер конца кода
    }
}

void buildHuffmanTable(unsigned char data[], int freq[], int size, int huffmanTable[MAX_TREE_HT][256]) {
    MinHeapNode* root = buildHuffmanTree(data, freq, size);
    int arr[MAX_TREE_HT], top = 0;
    printCodes(root, arr, top, huffmanTable);
}

// Сжатие данных и запись в архив
void write_file(FILE *archive, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("File opening failed");
        return;
    }

    unsigned char buffer[256] = {0};
    int freq[256] = {0};

    int file_size = 0;
    while (fread(&buffer[file_size], 1, 1, file) > 0) {
        freq[buffer[file_size]]++;
        file_size++;
    }

    // Построение таблицы Хаффмана
    int huffmanTable[MAX_TREE_HT][256] = {{0}};
    buildHuffmanTable(buffer, freq, 256, huffmanTable);

    HeaderEntry header;
    strcpy(header.file_name, file_path);
    header.file_size = file_size;

    fwrite(&header, sizeof(HeaderEntry), 1, archive);

    fseek(file, 0, SEEK_SET);
    while (fread(&buffer, 1, 1, file) > 0) {
        for (int i = 0; huffmanTable[buffer[0]][i] != -1; i++) {
            fputc(huffmanTable[buffer[0]][i] + '0', archive);
        }
    }

    fclose(file);
}

void process_directory(FILE *archive, const char *directory_path) {
    DIR *dir = opendir(directory_path);
    if (!dir) {
        perror("Directory opening failed");
        return;
    }

    struct dirent *entry;
    char path[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", directory_path, entry->d_name);

        struct stat st;
        stat(path, &st);

        if (S_ISDIR(st.st_mode)) {
            process_directory(archive, path);
        } else {
            write_file(archive, path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input_directory> <output_archive>\n", argv[0]);
        return 1;
    }

    const char *input_directory = argv[1];
    const char *output_archive = argv[2];

    FILE *archive = fopen(output_archive, "wb");
    if (!archive) {
        perror("Archive creation failed");
        return 1;
    }

    process_directory(archive, input_directory);

    fclose(archive);
    printf("Archiving completed.\n");

    return 0;
}
