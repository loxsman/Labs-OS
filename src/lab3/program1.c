#include <stdio.h>    // Для стандартного ввода-вывода
#include <stdlib.h>   // Для функций управления памятью и системных вызовов
#include <fcntl.h>    // Для работы с файловыми дескрипторами
#include <unistd.h>   // Для системных вызовов

int main(int argc, char *argv[]) {
    // Проверка на количество аргументов командной строки
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]); // Выводим сообщение об ошибке
        exit(EXIT_FAILURE); // Завершаем программу с ошибкой
    }

    // Открываем файл, переданный в аргументах
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) { // Проверка на успешность открытия файла
        perror("Error opening file"); // Выводим ошибку
        exit(EXIT_FAILURE); // Завершаем программу с ошибкой
    }

    char buffer[1024]; // Буфер для чтения данных из файла
    ssize_t bytesRead; // Количество прочитанных байтов
    // Читаем файл до тех пор, пока есть данные
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytesRead); // Выводим прочитанные данные в стандартный вывод
    }

    // Если произошла ошибка при чтении, выводим сообщение
    if (bytesRead == -1) {
        perror("Error reading file"); // Выводим ошибку
        close(fd); // Закрываем файл
        exit(EXIT_FAILURE); // Завершаем программу с ошибкой
    }

    close(fd); // Закрываем файл после завершения работы
    return 0; // Успешное завершение программы
}
