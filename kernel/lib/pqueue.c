#include "pqueue.h"

static inline void
pq_swap(pq_item *a, pq_item *b)
{
  pq_item temp = *a;
  *a = *b;
  *b = temp;
}

static inline void
pq_heapify_up(pqueue *pq, uint64 child)
{
  uint64 parent;

  while (0 < child) {
    parent = (child - 1) / 2;
    if (pq->items[child].key >= pq->items[parent].key)
      break;
    pq_swap(&pq->items[child], &pq->items[parent]);
    child = parent;
  }
}

static inline void
pq_heapify_down(pqueue *pq, uint64 parent)
{
  uint64 smallest;
  uint64 left_child;
  uint64 right_child;

  while (1) {
    smallest = parent;
    left_child = 2 * parent + 1;
    right_child = 2 * parent + 2;

    if (pq->size > left_child && pq->items[left_child].key < pq->items[smallest].key)
      smallest = left_child;
    if (pq->size > right_child && pq->items[right_child].key < pq->items[smallest].key)
      smallest = right_child;

    if (smallest == parent)
      break;

    pq_swap(&pq->items[parent], &pq->items[smallest]);
    parent = smallest;
  }
}

void
pq_init(pqueue *pq)
{
  pq->size = 0;
}

int
pq_push(pqueue *pq, uint64 key, void *value)
{
  if (PQ_MAX_SIZE <= pq->size)
    return -1;

  pq->items[pq->size].key = key;
  pq->items[pq->size].value = value;
  pq_heapify_up(pq, pq->size);
  ++pq->size;

  return 0;
}

int
pq_peek(pqueue *pq, uint64 *key, void **value)
{
  if (0 == pq->size)
    return -1;
  if (key)
    *key = pq->items[0].key;
  if (value)
    *value = pq->items[0].value;
  return 0;
}

int
pq_pop(pqueue *pq, uint64 *key, void **value)
{
  if (0 == pq->size)
    return -1;

  if (key)
    *key = pq->items[0].key;
  if (value)
    *value = pq->items[0].value;
  --pq->size;

  if (0 < pq->size) {
    pq->items[0] = pq->items[pq->size];
    pq_heapify_down(pq, 0);
  }

  return 0;
}

int
pq_is_empty(pqueue *pq)
{
  return 0 == pq->size;
}

uint64
pq_size(pqueue *pq)
{
  return pq->size;
}

void
pq_destroy(pqueue *pq)
{
  pq->size = 0;
}
