# Eltex Practices

Репозиторий с практическими заданиями по Eltex.

## Структура проекта

- `module1/` — практики по Linux (работа с командной строкой, bash-скрипты, системные утилиты).
- `module2/` — практики по программированию на C под Linux (модули, Makefile-фрагменты, CLI-приложения).
- `module3/` — практики по системному программированию: процессы, IPC, сокеты.
- `scripts/` — общие Git-хуки (pre-commit, pre-push).
- `.github/workflows/` — конфигурация CI.

## Сборка

```sh
make          # собрать все модули
make test     # запустить все тесты
make clean    # очистить артефакты сборки
```

## Запуск приложений

```sh
make run-calculator      ARGS="..."
make run-phonebook       ARGS="..."
make run-phonebook-static
make run-permissions
make run-packet_router
make run-priority_queue

make run-01              ARGS="file1.txt file2.txt"
```

Полный список целей `run-*` — в корневом `Makefile`.

## Форматирование

```sh
make format         # переформатировать все .c/.h
make format-check   # проверить форматирование (для CI)
```

Стиль задан `.clang-format`. Компилятор — `clang`, стандарт — C23.
