#include "unarch.h"

int main(int argc, char* argv[]) {
  if (argc == 3) {
    char* path_to_arch = argv[1];
    char* path_for_unarch = argv[2];
    unarch(path_to_arch, path_for_unarch);
    return EXIT_SUCCESS;
  }
}