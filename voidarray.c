#include <stdlib.h>
#include <stdio.h>
#include "voidarray.h"

#define INITIAL_SIZE 10

struct void_array_t {
  int strptr_allocated;
  int length;
  void **arr;
};

VoidArray
VoidArray_create()
{
  VoidArray intarr = malloc(sizeof(struct void_array_t));
  intarr->arr = malloc(sizeof(void*) * INITIAL_SIZE);
  intarr->strptr_allocated = INITIAL_SIZE;
  intarr->length = 0;
  return intarr;
}

void
VoidArray_destroy(VoidArray arr)
{
  free(arr->arr);
  free(arr);
}

void
VoidArray_add(VoidArray arr, void *ptr)
{
  if(arr->length >= arr->strptr_allocated) {
    arr->strptr_allocated += INITIAL_SIZE;
    arr->arr = realloc(arr->arr, arr->strptr_allocated * sizeof(void*));
  }  
  arr->arr[arr->length] = ptr;
  arr->length++;
}

void*
VoidArray_get(VoidArray arr, int index)
{
  if (index < 0) {
    index = arr->length + index;
  }
  //printf("VoidArray_get(): %d\n", index);
  if (index < 0 || index >= arr->length) {
    return NULL;    
  }
  return arr->arr[index];
}

void
VoidArray_set(VoidArray arr, void *ptr, int index)
{
  if (index >= arr->length || index < 0) {
    return;
  }
  arr->arr[index] = ptr;
}

int
VoidArray_length(VoidArray arr)
{
  return arr->length;
}

const void**
VoidArray_get_array(VoidArray arr)
{
  return (const void**)arr->arr;
}
