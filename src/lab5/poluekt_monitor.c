#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_GRANDMOTHERS 2 // Количество бабушек
#define NUM_MOTHERS 1      // Количество мам
#define NUM_GIRLS 1        // Количество девушек

// Определяем мьютекс и условную переменную для монитора
pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER; // Мьютекс для контроля доступа
pthread_cond_t monitor_cond = PTHREAD_COND_INITIALIZER;    // Условная переменная для уведомлений

int confirmation_received = 0; // Флаг, обозначающий получение подтверждения

// Функция потока Полуэкта
void* poluekt_thread(void* arg) {
    printf("Полуэкт: Я на работе допоздна...\n");
    sleep(1); // Имитация задержки

    printf("Полуэкт: Пытаюсь позвонить для подтверждения...\n");

    pthread_mutex_lock(&monitor_mutex); // Блокируем мьютекс для доступа к критической секции

    if (!confirmation_received) {
        confirmation_received = 1;            // Устанавливаем флаг подтверждения
        printf("Полуэкт: Получил подтверждение от кого-то.\n");
        pthread_cond_broadcast(&monitor_cond); // Уведомляем всех родственников
    }

    pthread_mutex_unlock(&monitor_mutex); // Разблокируем мьютекс
    return NULL;
}

// Функция потока родственников (бабушек, мам и девушек)
void* relative_thread(void* arg) {
    char* name = (char*)arg; // Имя потока (родственника)

    printf("%s: Хочу узнать, все ли в порядке с Полуэктом...\n", name);
    sleep(1); // Имитация задержки

    pthread_mutex_lock(&monitor_mutex); // Блокируем мьютекс

    // Ждём, пока не будет получено подтверждение
    while (!confirmation_received) {
        pthread_cond_wait(&monitor_cond, &monitor_mutex); // Ждём уведомления
    }

    printf("%s: Получил(а) подтверждение, что с Полуэктом все в порядке.\n", name);

    pthread_mutex_unlock(&monitor_mutex); // Разблокируем мьютекс
    return NULL;
}

int main() {
    pthread_t poluekt; // Поток для Полуэкта
    pthread_t relatives[NUM_GRANDMOTHERS + NUM_MOTHERS + NUM_GIRLS]; // Массив потоков для родственников

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

    // Уничтожаем мьютекс и условную переменную
    pthread_mutex_destroy(&monitor_mutex);
    pthread_cond_destroy(&monitor_cond);

    return 0;
}
