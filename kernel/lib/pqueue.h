#pragma once

#include "../types.h"

#define PQ_MAX_SIZE 64

typedef struct pq_item {
  uint64 key;
  void *value;
} pq_item;

typedef struct pqueue {
  pq_item items[PQ_MAX_SIZE];
  uint64 size;
} pqueue;

void
pq_init(pqueue *pq);

int
pq_push(pqueue *pq, uint64 key, void *value);

int
pq_peek(pqueue *pq, uint64 *key, void **value);

int
pq_pop(pqueue *pq, uint64 *key, void **value);

int
pq_is_empty(pqueue *pq);

uint64
pq_size(pqueue *pq);

void
pq_destroy(pqueue *pq);
