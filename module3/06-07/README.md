# Общая библиотека заданий 6 и 7 (`libchat_common`)

Статическая библиотека с кодом, общим для UDP-чата (`../06`) и TCP-чата
(`../07`):

- `protocol.h` — бинарный формат пакетов чата (`JOIN`, `LEAVE`, `TEXT`,
  `FILE_META`, `FILE_CHUNK`, `FILE_END`) и их сериализация.
- `message.h` — разбор строки пользовательского ввода (текст, команда
  `/file <путь>`).
- `signal_state.h` — флаг завершения по `SIGINT`/`SIGTERM`.
- `view.h` — весь вывод на экран.

## Сборка и тесты

```bash
make        # build/lib/libchat_common.a
make test   # юнит-тесты протокола и разбора ввода
```
