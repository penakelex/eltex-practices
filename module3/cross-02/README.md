# cross-02 — управление такси

Кросс-задание 2 (fork + poll/epoll/select + timers + IPC).

Терминальное приложение-опорный пункт такси с CLI. Команды:

- `create_driver` — порождает дочерний процесс-driver, печатает его pid;
- `send_task <pid> <task_timer>` — выдаёт задачу driver'у на `task_timer`
  секунд; повторный `send_task` во время занятости возвращает `Busy
  <осталось секунд>`;
- `get_status <pid>` — статус driver'а: `Available` или `Busy <осталось>`;
- `get_drivers` — список всех driver'ов этого инстанса CLI с их статусами.

Завершение — по EOF на stdin (Ctrl+D): CLI рассылает driver'ам команду
shutdown и дожидается их через `waitpid`.

## Выбор механизмов

**Мультиплексирование — `select`.** В задаче 7 уже использованы `epoll`
(сервер) и `poll` (клиент), а условие кросс-задания требует механизм, не
задействованный в задаче 7, — остаётся `select`. Его ограничения
(`FD_SETSIZE`, линейный опрос набора fd) здесь несущественны: CLI держит
дескрипторы только stdin и пары pipe на каждого driver'а.

**IPC — неименованные pipe.** На каждого driver'а создаётся пара pipe
(CLI→driver и driver→CLI) до `fork`, концы делятся между процессами.
Pipe естественен для порождённых процессов и встраивается в тот же
`select`-цикл без потоков и сигнальных костылей.

**Таймеры — `timerfd` (CLOCK_MONOTONIC).** Driver ждёт в одном `select`
сразу два дескриптора: pipe с командами и свой timerfd. По срабатыванию
таймера driver меняет состояние Busy -> Available и уведомляет CLI.
Никаких циклов ожидания без полезных действий.

## Устройство

- `src/taxi_controller.c` — CLI-цикл на `select` (stdin + pipe'ы
  driver'ов), разбор команд, запуск и остановка driver'ов;
- `src/driver.c` — цикл дочернего процесса на `select` (pipe + timerfd),
  состояние Available/Busy;
- `src/taxi_protocol.c` — сообщения CLI↔driver фиксированного размера
  (`write`/`read` целиком);
- `src/driver_table.c` — таблица driver'ов (pid, fd, последний известный
  статус);
- `src/taxi_view.c` — весь вывод.

## Сборка и тесты

```sh
make            # сборка build/bin/taxi
make test       # юнит-тесты Unity
```

## Запуск

```sh
./build/bin/taxi          # или make run / make run-cross-02 из module3
```

Пример сессии:

```
> create_driver
driver 4812 created
> send_task 4812 5
driver 4812: task accepted, busy for 5 s
> send_task 4812 5
driver 4812: Busy 4
> get_status 4812
driver 4812: Busy 3
driver 4812: Available
> get_drivers
driver 4812: Available
```
