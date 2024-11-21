#include "unarch.h"

// Функция создания директории
void create_dir(const char* path) {
  char command[1024];
  sprintf(command, "mkdir -p %s", path); 
  system(command);
}

// Функция для проверки, является ли строка числом
int is_numeric(const char* str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (!isdigit(str[i])) {
      return 0;  // Не число
    }
  }
  return 1;  // Число
}

// Функция для хеширования пароля
void hash_password(const char* password, unsigned char* hash_output) {
  SHA256((unsigned char*)password, strlen(password), hash_output);
}

// Преобразование хеша в строку для хранения и сравнения
void hash_to_string(const unsigned char* hash, char* hash_string) {
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    sprintf(hash_string + (i * 2), "%02x", hash[i]);
  }
}

// Основная функция для разархивации
int unarch(char* path_to_arch, char* base_path_for_unarch) {
  FILE* archive_file = fopen(path_to_arch, "rb");
  // Если не получилось открыть файл
  if (!archive_file) {
    perror("Ошибка при открытии архивного файла"); 
    exit(EXIT_FAILURE);
  }

  char str[256];
  fgets(str, sizeof(str), archive_file);
  // Если первая строка не #arch.bin (Проверка, что это архив)
  if (strcmp(str, "#arch.bin\n") != 0) {
    printf("%s", "Данный файл не является архивом");
    fclose(archive_file);
    exit(EXIT_FAILURE);
  }

  char pass_or_num[256];
  fgets(pass_or_num, sizeof(pass_or_num), archive_file);

  pass_or_num[strcspn(pass_or_num, "\n")] = '\0';

  int num_files;  // Объявляем переменную num_files
  // Если вторая строка - число, то это количество файлов в архиве, значит пароля нет
  if (is_numeric(pass_or_num)) {
    printf("Пароль не установлен для этого архива.\n");
    num_files = atoi(pass_or_num); 
  // Если вторая строка - не число, то это пароль
  } else {
    char input_pass[256];
    printf("Введите пароль от архива: ");
    fgets(input_pass, sizeof(input_pass), stdin);

    input_pass[strcspn(input_pass, "\n")] = '\0';

    // Хешируем введенный пароль
    unsigned char input_hash[SHA256_DIGEST_LENGTH];
    hash_password(input_pass, input_hash);

    // Преобразуем хеш в строку
    char input_hash_string[SHA256_DIGEST_LENGTH * 2 + 1];
    hash_to_string(input_hash, input_hash_string);

    // Сравниваем хеш введенного пароля с хешем из архива
    if (strcmp(pass_or_num, input_hash_string) != 0) {
      printf("Неверный пароль!\n");
      fclose(archive_file);
      exit(EXIT_FAILURE);
    }

    // Считываем количество файлов после проверки пароля
    fscanf(archive_file, "%d\n", &num_files);
  }
  // Считываем информацию о каждом файле
  FileHeader* files = malloc(num_files * sizeof(FileHeader));
  for (int i = 0; i < num_files; i++) {
    fscanf(archive_file, "%s\n%ld\n", files[i].path, &files[i].size);
  }

  char archive_name[1024];
  snprintf(archive_name, sizeof(archive_name), "%s/%s", base_path_for_unarch, "archive_folder");
  create_dir(archive_name);

  for (int i = 0; i < num_files; i++) {
    char buf[4096];
    snprintf(buf, sizeof(buf), "%s/%s", archive_name, files[i].path);

    // Проверка на пустую папку
    if (files[i].size == 0 && buf[strlen(buf) - 1] == '/') { 
      create_dir(buf); 
      continue; 
    }

    //создание директорий для файлов
    char dir_path[4096];
    strncpy(dir_path, buf, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash != NULL) {
      *last_slash = '\0'; 
    }
    create_dir(dir_path);

    FILE* output_file = fopen(buf, "wb");
    if (!output_file) {
      perror("Ошибка при создании файла");
      exit(EXIT_FAILURE);
    }

    //Чтение и запись файлов в выходной файл
    char* buffer = malloc(files[i].size);
    fread(buffer, 1, files[i].size, archive_file); 
    fwrite(buffer, 1, files[i].size, output_file);

    fclose(output_file);
    free(buffer); 
  }
  //Освобождение памяти
  fclose(archive_file); 
  free(files); 
  return EXIT_SUCCESS; 
}