#include <locale.h>
#include <menu.h>
#include <phonebook.h>
#include <stdio.h>
#include <string.h>

static void print_usage(const char *program) {
  printf("Использование: %s [--collection array|list|tree]\n", program);
  puts("  --collection array   динамический массив (по умолчанию)");
  puts("  --collection list    двусвязный упорядоченный список");
  puts("  --collection tree    бинарное дерево с периодической балансировкой");
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  const char *collection_name = "array";

  for (int index = 1; index < argc; ++index) {
    if (strcmp(argv[index], "--collection") == 0) {
      if (index + 1 >= argc) {
        fprintf(stderr, "Ожидается имя коллекции после --collection\n");
        return 1;
      }

      collection_name = argv[++index];
    } else if (strcmp(argv[index], "--help") == 0 ||
               strcmp(argv[index], "-h") == 0) {
      print_usage(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "Неизвестный аргумент: %s\n", argv[index]);
      return 1;
    }
  }

  Phonebook phonebook;

  if (!phonebook_initialize(&phonebook, collection_name)) {
    fprintf(stderr, "Неизвестный тип коллекции: %s\n", collection_name);
    return 1;
  }

  menu_run(&phonebook);

  phonebook_free(&phonebook);
  return 0;
}