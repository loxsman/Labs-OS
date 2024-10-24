#include <stdio.h>    
#include <stdlib.h>  
#include <fcntl.h>   
#include <unistd.h>   

int main(int argc, char *argv[]) {
    // Проверка на количество аргументов командной строки
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]); 
        exit(EXIT_FAILURE); 
    }

    // Открываем файл, переданный в аргументах
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) { 
        perror("Error opening file"); 
        exit(EXIT_FAILURE); 
    }

    char buffer[1024]; 
    ssize_t bytesRead; 
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytesRead); // Выводим прочитанные данные в стандартный вывод
    }

    // Если произошла ошибка при чтении, выводим сообщение
    if (bytesRead == -1) {
        perror("Error reading file"); 
        close(fd); 
        exit(EXIT_FAILURE); 
    }

    close(fd); 
    return 0; 
}
