# Priority Queue

Очередь с приоритетом на 256 уровней (0..255). Каждый уровень — циклический буфер фиксированной ёмкости.

## Соглашения

- **255 — самый высокий приоритет**, 0 — самый низкий. «Первым в очереди» считается элемент с максимальным приоритетом.
- Внутри одного приоритета — строгий FIFO.
- При переполнении уровня новый элемент вытесняет самый старый на этом уровне (циклический буфер с перезаписью). `add` никогда не возвращает ошибку — запись проходит всегда.
- Ёмкость каждого уровня задаётся константой `CIRCULAR_BUFFER_CAPACITY` (по умолчанию 64).

## API

```c
void priority_queue_initialize(PriorityQueue *queue);
void priority_queue_clear(PriorityQueue *queue);

void priority_queue_add(PriorityQueue *queue, const Message *message);

bool priority_queue_extract_first(PriorityQueue *queue, Message *out);
bool priority_queue_extract_with_priority(PriorityQueue *queue,
                                          unsigned int priority,
                                          Message *out);
bool priority_queue_extract_with_priority_at_least(PriorityQueue *queue,
                                                   unsigned int threshold,
                                                   Message *out);

bool priority_queue_is_empty(const PriorityQueue *queue);
size_t priority_queue_count(const PriorityQueue *queue);
size_t priority_queue_count_with_priority(const PriorityQueue *queue,
                                          unsigned int priority);
```

## Сборка и запуск

```bash
make
./build/bin/priority_queue --help
```

## Режимы запуска

```bash
# Интерактивное меню
./build/bin/priority_queue

# Случайная генерация и извлечение всех
./build/bin/priority_queue --seed 42 --generate 20 --extract-all

# Извлечение конкретного приоритета
./build/bin/priority_queue --seed 7 --generate 10 --extract-priority 128

# Извлечение по порогу
./build/bin/priority_queue --seed 7 --generate 10 --extract-at-least 200
```

## Тесты

```bash
make test
```

- `test_circular_buffer` — пустой/полный буфер, FIFO, переполнение с вытеснением, кольцевой обход.
- `test_priority_queue` — `add`, `extract_first` (по убыванию приоритета, FIFO внутри уровня), `extract_with_priority` (точный), `extract_with_priority_at_least` (порог включительно), переполнение уровня, согласованность `total`.
