#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

// Размер изображения (например, 512x512 пикселей)
#define WIDTH 512
#define HEIGHT 512

// Структура для передачи данных в поток
typedef struct {
    int thread_id;
    int start_row;
    int end_row;
    int (*image)[WIDTH];
    int (*result)[WIDTH];
} ThreadData;

// Ядра Собела по X и Y
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

// Функция для применения фильтра Собела в определённой части изображения
void* applySobel(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int gx, gy;

    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 1; j < WIDTH - 1; j++) {
            gx = 0;
            gy = 0;

            // Применяем ядра фильтра Собела к пикселям изображения
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    gx += data->image[i + k][j + l] * kernelX[k + 1][l + 1];
                    gy += data->image[i + k][j + l] * kernelY[k + 1][l + 1];
                }
            }

            // Вычисляем градиент изображения
            data->result[i][j] = sqrt(gx * gx + gy * gy);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_threads>\n", argv[0]);
        return -1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Number of threads must be positive!\n");
        return -1;
    }

    // Инициализация изображения
    int image[HEIGHT][WIDTH];
    int result[HEIGHT][WIDTH];
    
    // Заполняем изображение случайными значениями (для примера)
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            image[i][j] = rand() % 256;
            result[i][j] = 0;
        }
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    // Время выполнения
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Разбиваем изображение на части для обработки потоками
    int rows_per_thread = HEIGHT / num_threads;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].start_row = i * rows_per_thread;
        thread_data[i].end_row = (i == num_threads - 1) ? HEIGHT : (i + 1) * rows_per_thread;
        thread_data[i].image = image;
        thread_data[i].result = result;

        pthread_create(&threads[i], NULL, applySobel, &thread_data[i]);
    }

    // Ожидание завершения всех потоков
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);

    // Рассчитываем время выполнения
    double elapsed = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsed += (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Execution time with %d threads: %f ms\n", num_threads, elapsed);

    // Для проверки выводим часть результата
    printf("Resulting image (sample):\n");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }

    return 0;
}
