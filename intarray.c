#include <stdlib.h>
#include "intarray.h"

#define INITIAL_SIZE 10

struct int_array_t {
  int strptr_allocated;
  int length;
  int index;
  int *arr;
};

IntArray
IntArray_create()
{
  IntArray intarr = malloc(sizeof(struct int_array_t));
  intarr->arr = malloc(sizeof(int) * INITIAL_SIZE);
  intarr->strptr_allocated = INITIAL_SIZE;
  intarr->length = 0;
  intarr->index  = 0;
  return intarr;
}

void
IntArray_destroy(IntArray arr)
{
  free(arr->arr);
  free(arr);
}

void
IntArray_add(IntArray arr, int num)
{
  if(arr->length >= arr->strptr_allocated) {
    arr->strptr_allocated += INITIAL_SIZE;
    arr->arr = realloc(arr->arr, arr->strptr_allocated * sizeof(int));
  }  
  arr->arr[arr->length] = num;
  arr->length++;
}

int
IntArray_get(IntArray arr, int index)
{
  if (index < 0 || index >= arr->length) {
    return 0;    
  }
  return arr->arr[index];
}

int
IntArray_length(IntArray arr)
{
  return arr->length;
}

// Iteration Functions

void
IntArray_start(IntArray arr)
{
  arr->index = 0;
}

int
IntArray_cur(IntArray arr)
{
  if (arr->index >= arr->length) {
    return 0;
  }
  return arr->arr[arr->index];
}

int
IntArray_next(IntArray arr)
{
  int out = IntArray_cur(arr);
  arr->index++;
  return out;
}

int
IntArray_at_end(IntArray arr)
{
  return (arr->index >= arr->length);
}

const int*
IntArray_get_array(IntArray arr)
{
  return (const int*)arr->arr;
}
