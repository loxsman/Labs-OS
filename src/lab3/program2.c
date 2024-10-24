#include <stdio.h>    
#include <stdlib.h>   
#include <unistd.h>   
#include <sys/wait.h> 
#include <fcntl.h>    

#define BUFFER_SIZE 1024 // Размер буфера для чтения данных

// Функция для выполнения побитовой операции XOR между данными из двух файлов
void xor_files(int fd1, int fd2, const char* output_file) {
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) { 
        perror("Error opening output file"); 
        exit(EXIT_FAILURE); 
    }

    char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE]; // Буферы для хранения данных из файлов
    ssize_t bytesRead1, bytesRead2; // Количество прочитанных байтов из каждого файла

    // Читаем данные из обоих файлов и выполняем XOR, пока есть данные
    while ((bytesRead1 = read(fd1, buffer1, BUFFER_SIZE)) > 0 &&
           (bytesRead2 = read(fd2, buffer2, BUFFER_SIZE)) > 0) {
        // Выполняем побитовую операцию XOR
        for (ssize_t i = 0; i < bytesRead1 && i < bytesRead2; i++) {
            buffer1[i] ^= buffer2[i]; 
        }
        write(output_fd, buffer1, bytesRead1);
    }

    close(output_fd); 
}

int main(int argc, char *argv[]) {
    // Проверка на количество аргументов командной строки
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <file1> <file2> <output_file>\n", argv[0]); 
        exit(EXIT_FAILURE); 
    }

    int pipe1[2], pipe2[2]; // Массивы для двух каналов

    // Создаем два канала для IPC
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Error creating pipes"); 
        exit(EXIT_FAILURE); 
    }

    // Создаем первый дочерний процесс
    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Дочерний процесс 1: запускаем программу 1 для первого файла
        close(pipe1[0]); 
        dup2(pipe1[1], STDOUT_FILENO); 
        execlp("./program1", "./program1", argv[1], NULL); 
        perror("Error executing program1"); 
        exit(EXIT_FAILURE); 
    }

    // Создаем второй дочерний процесс
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Дочерний процесс 2: запускаем программу 1 для второго файла
        close(pipe2[0]); 
        dup2(pipe2[1], STDOUT_FILENO); 
        execlp("./program1", "./program1", argv[2], NULL); 
        perror("Error executing program1"); 
        exit(EXIT_FAILURE); 
    }

    // Родительский процесс
    close(pipe1[1]); // Закрываем дескриптор записи в первый канал
    close(pipe2[1]); // Закрываем дескриптор записи во второй канал

    // Выполняем побитовую операцию XOR между выходными данными двух процессов
    xor_files(pipe1[0], pipe2[0], argv[3]);

    close(pipe1[0]); 
    close(pipe2[0]); 

    wait(NULL); 
    wait(NULL);

    return 0; 
}
