# Task 2: System V Message Queues + Signals

Модель «Издатель-Брокер-Подписчик» на одной очереди сообщений
System V. Завершение процессов координируется сигналами.

## Возможности

- **Три роли.** `-b` — брокер, `-p <topic>` — издатель, `-s <topic>...`
  — подписчик (можно подписать один процесс на несколько тем).
- **Единственная очередь.** Ключ `ftok("/tmp", 'M')`; брокер создаёт
  очередь через `IPC_CREAT | IPC_EXCL`, повторный запуск брокера
  печатает диагностику и завершается.
- **Управляющие и данные в одной очереди.** Управляющие кадры идут
  с `mtype = 1`, доставка подписчику — с `mtype = <pid подписчика>`.
- **Корректное завершение.** На SIGINT/SIGTERM брокер рассылает
  SIGINT всем известным издателям и подписчикам, удаляет очередь
  через `msgctl(IPC_RMID)` и выходит. Клиенты завершаются по сигналу
  или при получении `EIDRM`/`EINVAL` из `msgrcv`.

## Архитектура

- `main.c` — разбор `argv` (`-b` | `-p <topic>` | `-s <topic>...`).
- `broker.c` — главный цикл `msgrcv` по `mtype = 1`, диспетчеризация
  кадров; динамические списки издателей, подписчиков и пар
  `(pid, тема)`; рассылка SIGINT при завершении.
- `publisher.c` — чтение stdin через `getline`, формирование
  `send`-кадров и `msgsnd`.
- `subscriber.c` — подписка/отписка по каждой теме, `msgrcv` по
  `mtype = свой pid`, печать доставленных сообщений.
- `queue.c` — обёртки `msgget`/`msgsnd`/`msgrcv`/`msgctl` и тип
  `QueueMessage` (`long type` + текстовый буфер).
- `protocol.c` — формирование и парсинг кадров `send`, `subscribe`,
  `unsubscribe` и кадров доставки `<topic>\n<payload>`.
- `signal_state.c` — общий `volatile sig_atomic_t` и `sigaction` для
  SIGINT/SIGTERM без `SA_RESTART`, чтобы блокирующие вызовы
  прерывались.
- `view.c` — весь вывод в `stderr`/`stdout`.
- `include/*.h` — публичные интерфейсы, `tests/test_protocol.c` —
  Unity-тесты парсинга и форматирования.

### Протокол

Управляющие кадры (`mtype = 1`, клиент → брокер):

```
send,<pid>,<topic>\n<payload>
subscribe,<pid>,<topic>
unsubscribe,<pid>,<topic>
```

Кадр доставки (`mtype = pid подписчика`, брокер → подписчик):

```
<topic>\n<payload>
```

Завершение — SIGINT от брокера клиентам и `IPC_RMID` для очереди.

## Использование

```bash
make
./build/bin/broker_chat -b                          # брокер
./build/bin/broker_chat -s news sports              # подписчик
./build/bin/broker_chat -p news                     # издатель
```

Пример в трёх терминалах:

```bash
# A
./build/bin/broker_chat -b
# B
./build/bin/broker_chat -s news
# C
./build/bin/broker_chat -p news
hello
# в B печатается: [news] hello
```

Через `make`:

```bash
make run-02 ARGS="-b"
make run-02 ARGS="-s news"
make run-02 ARGS="-p news"
```

Ошибочные сценарии:

```bash
./build/bin/broker_chat -b           # при живом брокере
# Broker already running on this queue

./build/bin/broker_chat -p news      # при мёртвом брокере
# Broker is not running

./build/bin/broker_chat              # нет аргументов
# Usage: ./build/bin/broker_chat -b | -p <topic> | -s <topic> [<topic> ...]
```

## Тестирование

```bash
make test
```

Unity-тесты (`tests/test_protocol.c`) покрывают:

- roundtrip `send`-кадра (pid, тема, payload);
- roundtrip `subscribe` и `unsubscribe`;
- отклонение мусорных строк, пустого и нечислового pid;
- roundtrip кадра доставки, включая пустой payload.

## Зависимости

- C23-совместимый компилятор (`clang` по умолчанию).
- POSIX.1-2024 + System V IPC: `ftok(3)`, `msgget(2)`, `msgsnd(2)`,
  `msgrcv(2)`, `msgctl(2)`, `sigaction(2)`.
- `curl` — только для первой загрузки Unity при `make test`.

## Ограничения

- Ключ `ftok("/tmp", 'M')` фиксирован — одновременно может работать
  только один брокер на систему.
- Текст сообщения ограничен `QUEUE_MESSAGE_MAX_TEXT - 1` (4095 байт);
  слишком длинная строка от издателя пропускается.
- Подписчик не получает сообщения, отправленные до подписки.
- `SIGKILL` брокера не перехватывается: очередь остаётся и чистится
  вручную (`ipcs -q`, `ipcrm -q <msqid>`).
