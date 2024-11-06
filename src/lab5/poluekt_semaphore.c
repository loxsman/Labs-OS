#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_GRANDMOTHERS 2 // Количество бабушек
#define NUM_MOTHERS 1      // Количество мам
#define NUM_GIRLS 1        // Количество девушек

// Определяем семафоры
sem_t sem_poluekt;           // Семафор для контроля звонков Полуэкта
sem_t sem_confirmation;      // Семафор для передачи подтверждения

int confirmation_received = 0; // Флаг, обозначающий получение подтверждения

// Функция потока Полуэкта
void* poluekt_thread(void* arg) {
    printf("Полуэкт: Я на работе допоздна...\n");
    sleep(1); // Имитация задержки

    printf("Полуэкт: Пытаюсь позвонить для подтверждения...\n");
    sem_wait(&sem_poluekt); // Попытка зайти в критическую секцию

    if (!confirmation_received) {
        printf("Полуэкт: Получил подтверждение от кого-то.\n");
        confirmation_received = 1;          // Устанавливаем флаг подтверждения
        sem_post(&sem_confirmation);        // Разрешаем родственникам получить подтверждение
    }

    sem_post(&sem_poluekt); // Освобождение критической секции
    return NULL;
}

// Функция потока родственников (бабушек, мам и девушек)
void* relative_thread(void* arg) {
    char* name = (char*)arg; // Имя потока (родственника)

    printf("%s: Хочу узнать, все ли в порядке с Полуэктом...\n", name);
    sleep(1); // Имитация задержки

    sem_wait(&sem_confirmation); // Ожидание подтверждения от Полуэкта
    printf("%s: Получил(а) подтверждение, что с Полуэктом все в порядке.\n", name);

    sem_post(&sem_confirmation); // Освобождаем семафор для других родственников
    return NULL;
}

int main() {
    pthread_t poluekt; // Поток для Полуэкта
    pthread_t relatives[NUM_GRANDMOTHERS + NUM_MOTHERS + NUM_GIRLS]; // Массив потоков для родственников

    // Инициализация семафоров
    sem_init(&sem_poluekt, 0, 1);
    sem_init(&sem_confirmation, 0, 0);

    // Создание потока для Полуэкта
    pthread_create(&poluekt, NULL, poluekt_thread, NULL);

    // Создаем потоки для бабушек
    for (int i = 0; i < NUM_GRANDMOTHERS; i++) {
        char* name = malloc(20);
        sprintf(name, "Бабушка %d", i + 1); // Назначаем имя бабушке
        pthread_create(&relatives[i], NULL, relative_thread, (void*)name);
    }

    // Создаем поток для мамы
    char* mother_name = "Мама";
    pthread_create(&relatives[NUM_GRANDMOTHERS], NULL, relative_thread, (void*)mother_name);

    // Создаем потоки для девушек
    for (int i = 0; i < NUM_GIRLS; i++) {
        char* name = malloc(20);
        sprintf(name, "Девушка %d", i + 1); // Назначаем имя девушке
        pthread_create(&relatives[NUM_GRANDMOTHERS + NUM_MOTHERS + i], NULL, relative_thread, (void*)name);
    }

    // Ожидаем завершения потока Полуэкта
    pthread_join(poluekt, NULL);

    // Ожидаем завершения потоков родственников
    for (int i = 0; i < NUM_GRANDMOTHERS + NUM_MOTHERS + NUM_GIRLS; i++) {
        pthread_join(relatives[i], NULL);
    }

    // Уничтожаем семафоры
    sem_destroy(&sem_poluekt);
    sem_destroy(&sem_confirmation);

    return 0;
}
