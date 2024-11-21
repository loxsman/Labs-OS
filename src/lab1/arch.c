#include "arch.h"

// Функция для хеширования пароля
void hash_password(const char* password, unsigned char* hash_output) {
  SHA256((unsigned char*)password, strlen(password), hash_output);
}

// Преобразование хеша в строку для хранения
void hash_to_string(const unsigned char* hash, char* hash_string) {
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    sprintf(hash_string + (i * 2), "%02x", hash[i]);
  }
}

void readFiles(const char* dirPath, FileHeader** files, int* alloc_FileHeader,
              int* index, const char* base_name, const char* relative_path) {
  DIR* dir = opendir(dirPath);
  if (!dir) {
    perror("Ошибка при открытии директории");
    exit(EXIT_FAILURE);
  }
  struct dirent* entry;
  // Флаг, показывающий, содержит ли директория файлы
  int hasFiles = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      hasFiles = 1; // Директория содержит хотя бы один файл
      // Проверка выделения памяти для файлов
      if (*index >= *alloc_FileHeader) {
        int new_size = *alloc_FileHeader + BLOCK_SIZE;
        FileHeader* new_fileHeader =
            realloc(*files, sizeof(FileHeader) * new_size);
        if (new_fileHeader == NULL) {
          free(*files);
          closedir(dir);
          perror("Ошибка выделения памяти\n");
          exit(EXIT_FAILURE);
        }
        *files = new_fileHeader;
        *alloc_FileHeader = new_size;
      }
      FileHeader fileHeader;
      strncpy(fileHeader.filename, entry->d_name,
              sizeof(fileHeader.filename) - 1);
      fileHeader.filename[sizeof(fileHeader.filename) - 1] = '\0';

      // Формируем полный путь к файлу
      snprintf(fileHeader.path, sizeof(fileHeader.path), "%s/%s", dirPath,
               entry->d_name);

      // Формируем относительный путь к файлу
      char relative_file_path[1024];
      snprintf(relative_file_path, sizeof(relative_file_path), "%s/%s",
               relative_path, entry->d_name);

      // Получаем информацию о размере
      struct stat st;
      if (stat(fileHeader.path, &st) == -1) {
        perror("Ошибка при получении информации о файле");
        continue;
      } else {
        fileHeader.size = st.st_size;
      }

      // Сохраняем относительный путь в filename
      strncpy(fileHeader.filename, relative_file_path,
              sizeof(fileHeader.filename) - 1);
      fileHeader.filename[sizeof(fileHeader.filename) - 1] = '\0';

      (*files)[(*index)++] = fileHeader;
    } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
               strcmp(entry->d_name, "..") != 0) {
      char subDirPath[1024];
      snprintf(subDirPath, sizeof(subDirPath), "%s/%s", dirPath, entry->d_name);

      // Обновляем относительный путь для подкаталога
      char new_relative_path[1024];
      snprintf(new_relative_path, sizeof(new_relative_path), "%s/%s",
               relative_path, entry->d_name);

      // Рекурсивно обрабатываем подкаталоги
      readFiles(subDirPath, files, alloc_FileHeader, index, base_name,
                new_relative_path);

      hasFiles = 1; // Если были подкаталоги, устанавливаем флаг
    }
  }

  // Если директория пуста (не содержит файлов или подкаталогов), сохраняем её в архив с размером 0
  if (!hasFiles) {
    if (*index >= *alloc_FileHeader) {
      int new_size = *alloc_FileHeader + BLOCK_SIZE;
      FileHeader* new_fileHeader = realloc(*files, sizeof(FileHeader) * new_size);
      if (new_fileHeader == NULL) {
        free(*files);
        closedir(dir);
        perror("Ошибка выделения памяти\n");
        exit(EXIT_FAILURE);
      }
      *files = new_fileHeader;
      *alloc_FileHeader = new_size;
    }

    FileHeader dirHeader;
    // Сохраняем относительный путь к директории с размером 0
    snprintf(dirHeader.path, sizeof(dirHeader.path), "%s", dirPath);
    snprintf(dirHeader.filename, sizeof(dirHeader.filename), "%s/", relative_path); // '/' в конце обозначает папку
    dirHeader.size = 0;
    (*files)[(*index)++] = dirHeader;
  }

  closedir(dir);
}


void free_fileHeader(FileHeader* fileHeader) {
  if (fileHeader != NULL) {
    free(fileHeader);
    fileHeader = NULL;
  }
}

int spec_symbol_in_pass(const char* pass, const char* spec_symbol) {
  for (int i = 0; i < strlen(pass); i++) {
    if (strchr(spec_symbol, pass[i])) {
      return OK;
    }
  }
  return NO_SPEC_SYMBOL;  // символ не найден
}

// Функция проверки пароля
int check_pass(const char* pass) {
  const char* spec_symbol = "!@#$^&%%*()—_+=;:,./\\?|`~[]{";
  int big_let = 0, digit = 0, symbol = 0;

  if (strlen(pass) < 8) return SHORT_PASS;

  for (int i = 0; i < strlen(pass); i++) {
    if (isupper(pass[i]))
      big_let++;
    else if (isdigit(pass[i]))
      digit++;
    else if (strchr(spec_symbol, pass[i]))
      symbol++;
    else if (pass[i] == ' ' || (!isalpha(pass[i])))
      return WRONG_SYMBOL;
  }

  if (big_let == 0) return NO_CAP_LET;
  if (digit == 0) return NO_DIGIT;
  if (symbol == 0) return NO_SPEC_SYMBOL;

  return OK;
}

//Основная функция создания архива
void create_arch(int index, char* path_for_arch, char* path_to_arch, FileHeader* files) {
  char buff[100];
  snprintf(buff, sizeof(path_to_arch) + 2, "/%s", path_to_arch);
  buff[sizeof(buff) - 1] = '\0';

  // Проверка на существование архива
  for (int i = 0; i < index; i++) {
    files[i].filename[sizeof(files[i].filename) - 1] = '\0';

    if (strcmp(buff, files[i].filename) == 0) {
      printf("Такой архив уже есть!\n");
      exit(EXIT_FAILURE);
    }
  }

  char choice;
  // Запрашиваем у пользователя выбор
  printf("Хотите установить пароль для архива? (y/n): ");
  scanf(" %c", &choice); 

  char* pass = NULL;

  // Проверка выбора пользователя
  switch (choice) {
    case 'y': {
      printf(
          "Введите пароль для архива\nПароль должен содержать:\n1-хотя бы одну "
          "заглавную букву\n2-хотя бы одну цифру\n3-хотя бы один спец. символ\nИ "
          "не должен содержать:\n1-Пробелы\n2-буквы языка, помимо латинского "
          "алфавита\n");
      pass = getpass("");
      pass[strcspn(pass, "\n")] = '\0';
      int check_result = check_pass(pass);

      if (check_result == OK) {
        printf("Пароль принят!\n");
      } else {
        if (check_result == WRONG_SYMBOL) {
          printf("Некорректный пароль, введите новый пароль\n");
        } else if (check_result == SHORT_PASS) {
          printf("Пароль слишком короткий, введите новый пароль\n");
        } else if (check_result == NO_CAP_LET) {
          printf("В пароле нет заглавных букв, введите новый пароль\n");
        } else if (check_result == NO_DIGIT) {
          printf("В пароле нет цифр, введите новый пароль\n");
        } else if (check_result == NO_SPEC_SYMBOL) {
          printf("Пароль не содержит спец. символов\n");
        }
        exit(EXIT_FAILURE);
      }
      break;
    }
    case 'n':
      break;
    default:
      printf("Неверный выбор. Программа завершена.\n");
      exit(EXIT_FAILURE);
  }

  FILE* archiveFile = fopen(path_to_arch, "wb");
  if (!archiveFile) {
    perror("Ошибка при открытии архива для записи");
    exit(EXIT_FAILURE);
  }

  fprintf(archiveFile, "%s\n", "#arch.bin");
  //Если есть пароль, записываем его второй стрчокой
  if (pass != NULL) {
    unsigned char password_hash[SHA256_DIGEST_LENGTH];
    hash_password(pass, password_hash);
    char password_hash_string[SHA256_DIGEST_LENGTH * 2 + 1];
    hash_to_string(password_hash, password_hash_string);

    fprintf(archiveFile, "%s\n", password_hash_string);
  }
  fprintf(archiveFile, "%d\n", index);

  // Запись информации о файлах
  for (int i = 0; i < index; i++) {
    fwrite(files[i].filename, sizeof(char), strlen(files[i].filename) + 1, archiveFile);
    fprintf(archiveFile, "\n%ld\n", files[i].size);
  }

  // Запись содержимого файлов в архив
  for (int i = 0; i < index; i++) {
    FILE* file = fopen(files[i].path, "rb");
    if (file) {
      char buffer[4096];
      size_t bytesRead;
      while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        fwrite(buffer, 1, bytesRead, archiveFile);
      }
      fclose(file);
    } else {
      perror("Ошибка при открытии файла для записи");
      fclose(archiveFile);
      free_fileHeader(files);
      exit(EXIT_FAILURE);
    }
  }

  fclose(archiveFile);
}