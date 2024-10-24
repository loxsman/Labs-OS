#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

#define WIDTH 1024
#define HEIGHT 1024

typedef struct {
    int thread_id;
    int start_row;
    int end_row;
    unsigned char (*image)[WIDTH];
    unsigned char (*result)[WIDTH];
} ThreadData;

int kernelX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

int kernelY[3][3] = {
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}
};

void* applySobel(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int gx, gy;

    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 1; j < WIDTH - 1; j++) {
            gx = 0;
            gy = 0;

            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    gx += data->image[i + k][j + l] * kernelX[k + 1][l + 1];
                    gy += data->image[i + k][j + l] * kernelY[k + 1][l + 1];
                }
            }

            int magnitude = (int)sqrt(gx * gx + gy * gy);
            if (magnitude > 255) magnitude = 255;
            data->result[i][j] = (unsigned char)magnitude;
        }
    }

    pthread_exit(NULL);
}

// Функция для чтения изображения в формате PGM
int readPGM(const char* filename, unsigned char image[HEIGHT][WIDTH]) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return -1;
    }

    char format[3];
    int width, height, max_value;
    
    // Читаем заголовок PGM файла
    fscanf(file, "%s\n%d %d\n%d\n", format, &width, &height, &max_value);
    if (width != WIDTH || height != HEIGHT) {
        printf("Error: Image dimensions do not match %dx%d\n", WIDTH, HEIGHT);
        fclose(file);
        return -1;
    }

    // Читаем пиксели изображения
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            image[i][j] = fgetc(file);
        }
    }

    fclose(file);
    return 0;
}

// Функция для записи результата в PGM файл
void writePGM(const char* filename, unsigned char result[HEIGHT][WIDTH]) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    // Записываем заголовок PGM файла
    fprintf(file, "P5\n%d %d\n255\n", WIDTH, HEIGHT);

    // Записываем пиксели результата
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fputc(result[i][j], file);
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <num_threads> <input_image.pgm>\n", argv[0]);
        return -1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Number of threads must be positive!\n");
        return -1;
    }

    unsigned char image[HEIGHT][WIDTH];
    unsigned char result[HEIGHT][WIDTH] = {0};

    // Чтение изображения
    if (readPGM(argv[2], image) != 0) {
        return -1;
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int rows_per_thread = HEIGHT / num_threads;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? HEIGHT : (i + 1) * rows_per_thread;
        thread_data[i].image = image;
        thread_data[i].result = result;

        pthread_create(&threads[i], NULL, applySobel, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);

    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed += (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Execution time with %d threads: %f ms\n", num_threads, elapsed);

    // Записываем результат в PGM файл
    writePGM("result.pgm", result);

    return 0;
}
