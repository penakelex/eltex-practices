#pragma once

#include <priority_queue.h>

unsigned int simulation_next_id(void);
void simulation_generate(PriorityQueue *queue, unsigned int count,
                         unsigned int maximum_priority);
void simulation_extract_all(PriorityQueue *queue);
void simulation_extract_priority(PriorityQueue *queue, unsigned int priority);
void simulation_extract_at_least(PriorityQueue *queue, unsigned int threshold);