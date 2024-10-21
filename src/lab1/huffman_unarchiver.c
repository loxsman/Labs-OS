#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_TREE_HT 256

typedef struct {
    char file_name[256];
    unsigned long file_size;
} HeaderEntry;

typedef struct MinHeapNode {
    unsigned char data;
    unsigned freq;
    struct MinHeapNode *left, *right;
} MinHeapNode;

void decode_huffman(FILE *archive, MinHeapNode *root, unsigned long file_size, const char *output_file) {
    FILE *output = fopen(output_file, "wb");
    if (!output) {
        perror("Error creating output file");
        return;
    }

    MinHeapNode *current = root;
    unsigned char bit;
    unsigned long decoded_size = 0;
    unsigned char buffer = 0;
    int bit_pos = 0;

    while (decoded_size < file_size) {
        if (bit_pos == 0) {
            fread(&buffer, 1, 1, archive);
        }

        bit = (buffer >> (7 - bit_pos)) & 1;
        bit_pos = (bit_pos + 1) % 8;

        if (bit == 0)
            current = current->left;
        else
            current = current->right;

        if (!current->left && !current->right) {
            fwrite(&current->data, 1, 1, output);
            decoded_size++;
            current = root;
        }
    }

    fclose(output);
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

void extract_file(FILE *archive, const HeaderEntry *header, MinHeapNode *huffman_root) {
    decode_huffman(archive, huffman_root, header->file_size, header->file_name);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input_archive> <output_directory>\n", argv[0]);
        return 1;
    }

    const char *input_archive = argv[1];
    const char *output_directory = argv[2];

    FILE *archive = fopen(input_archive, "rb");
    if (!archive) {
        perror("Archive opening failed");
        return 1;
    }

    mkdir(output_directory, 0777);
    chdir(output_directory);

    HeaderEntry header;

    while (fread(&header, sizeof(HeaderEntry), 1, archive)) {
        char *dir = strdup(header.file_name);
        char *last_slash = strrchr(dir, '/');
        if (last_slash) {
            *last_slash = '\0';
            mkdir(dir, 0777);
        }
        free(dir);

        unsigned char data[256] = {0};
        int freq[256] = {0};

        MinHeapNode* huffman_root = buildHuffmanTree(data, freq, 256);
        extract_file(archive, &header, huffman_root);
    }

    fclose(archive);
    printf("Extraction completed.\n");

    return 0;
}
