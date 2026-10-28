#include <locale.h>
#include <menu.h>
#include <phonebook.h>

int main() {
  setlocale(LC_ALL, "");

  Phonebook phonebook;
  phonebook_initialize(&phonebook);

  menu_run(&phonebook);

  phonebook_free(&phonebook);
  return 0;
}