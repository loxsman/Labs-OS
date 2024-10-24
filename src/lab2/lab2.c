#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>

// Функция для логирования действий и команд
void log_action(const char *action) {
    FILE *log_file = fopen("terminal.log", "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }
    fprintf(log_file, "%s\n", action);
    fclose(log_file);
}

// Реализация команды ls (вывод содержимого директории)
void ls() {
    DIR *dir;
    struct dirent *entry;
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return;
    }

    if ((dir = opendir(cwd)) == NULL) {
        perror("opendir() error");
        return;
    }

    printf("Contents of %s:\n", cwd);
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

// Реализация команды cat (вывод содержимого файла)
void cat(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open() error");
        return;
    }

    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    if (bytes_read < 0) {
        perror("read() error");
    }

    close(fd);
}

// Реализация команды nice (изменение приоритета процесса)
void nice_command(const char *priority_str, const char *command) {
    int priority = atoi(priority_str);

    pid_t pid = fork();
    if (pid == 0) {
        if (setpriority(PRIO_PROCESS, 0, priority) == -1) {
            perror("setpriority() error");
            exit(EXIT_FAILURE);
        }
        execlp("/bin/sh", "sh", "-c", command, (char *)NULL);
        perror("execlp() error");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Command exited with status: %d\n", WEXITSTATUS(status));
        } else {
            printf("Command did not terminate normally.\n");
        }
    } else {
        perror("fork() error");
    }
}

// Реализация команды killall (убить все процессы с определенным именем)
void killall(const char *process_name) {
    char command[256];
    snprintf(command, sizeof(command), "pgrep %s", process_name);
    
    FILE *pgrep_output = popen(command, "r");
    if (pgrep_output == NULL) {
        perror("popen() error");
        return;
    }

    char pid_str[16];
    while (fgets(pid_str, sizeof(pid_str), pgrep_output) != NULL) {
        pid_t pid = atoi(pid_str);
        if (kill(pid, SIGKILL) == -1) {
            perror("kill() error");
        } else {
            printf("Killed process with PID: %d\n", pid);
        }
    }

    pclose(pgrep_output);
}

// Реализация команды cd (смена текущей директории)
void cd(const char *path) {
    if (chdir(path) != 0) {
        perror("chdir() error");
    } else {
        printf("Directory changed to %s\n", path);
    }
}

// Основной цикл работы терминала
void terminal_loop() {
    char command[256];
    while (1) {
        printf("Custom Terminal> ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = 0;
        log_action(command);

        if (strncmp(command, "ls", 2) == 0) {
            ls();
        } else if (strncmp(command, "cat ", 4) == 0) {
            cat(command + 4);
        } else if (strncmp(command, "nice ", 5) == 0) {
            char *priority_str = strtok(command + 5, " ");
            char *cmd = strtok(NULL, "");
            if (priority_str && cmd) {
                nice_command(priority_str, cmd);
            } else {
                printf("Usage: nice <priority> <command>\n");
            }
        } else if (strncmp(command, "killall ", 8) == 0) {
            killall(command + 8);
        } else if (strncmp(command, "cd ", 3) == 0) {
            cd(command + 3);
        } else {
            printf("Unknown command: %s\n", command);
        }
    }
}

// Основная функция программы
int main() {
    terminal_loop();
    return 0;
}
