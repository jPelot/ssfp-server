#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "strarray.h"

#define INITIAL_SIZE 10

struct string_array_t {
  size_t strptr_allocated;
  size_t length;
  char **arr;
};

StrArray
StrArray_create()
{
  StrArray strarr = malloc(sizeof(struct string_array_t));
  strarr->arr = malloc(sizeof(char *) * INITIAL_SIZE);
  strarr->strptr_allocated = INITIAL_SIZE;
  strarr->length = 0;
  return strarr;
}

void
StrArray_destroy(StrArray arr)
{
  // Free contents of array
  for(int i = 0; i < arr->length; i++) {
    free(arr->arr[i]);
  }
  free(arr->arr); // Free array
  free(arr); // Free struct
}

void
StrArray_add(StrArray arr, const char *str)
{
  if (str == NULL) {
    str = "";
  }
  if (arr->length >= arr->strptr_allocated) {
    arr->strptr_allocated += INITIAL_SIZE;
    arr->arr = realloc(arr->arr, arr->strptr_allocated * sizeof(char *));
  }  
  int len = strlen(str);
  arr->arr[arr->length] = malloc(len+1);
  strcpy(arr->arr[arr->length], str);
  arr->length++;
}

const char*
StrArray_get(StrArray arr, int index)
{
  if (index < 0) {
    index = arr->length + index;
  }
  if (index < 0 || index >= arr->length) {
    return NULL;
  }
  return arr->arr[index];
}


void
StrArray_set(StrArray arr, int index, const char *str)
{
  if (index < 0 || index >=arr->length) {
    return;
  }
  free(arr->arr[index]);
  arr->arr[index] = malloc(strlen(str)+1);
  strcpy(arr->arr[index], str);
}

int
StrArray_length(StrArray arr)
{
  return arr->length;
}

void
StrArray_add_arr(StrArray arr, StrArray that)
{
  for(int i = 0; i < that->length; i++) {
    StrArray_add(arr, that->arr[i]);
  }
}

char*
StrArray_combine(StrArray arr)
{
  int total_length = 0, i;
  char *result, *temp_ptr;
  // Calculate total length of result string
  for(i = 0; i < arr->length; i++) {
    total_length += strlen(arr->arr[i]);
  }
  // Allocate memory for result string
  temp_ptr = result = malloc(total_length + 1);
  // Copy strarray to result string
  for(i = 0; i < arr->length; i++) {
    strcpy(temp_ptr,arr->arr[i]);
    temp_ptr += strlen(arr->arr[i]);
  }
  return result;
}

char*
StrArray_splice(StrArray arr, const char* delimit)
{
  size_t total_length = 0, i, delimit_length;
  char *result, *temp_ptr;
  // Calculate total length of result string
  delimit_length = strlen(delimit);
  for(i = 0; i < arr->length; i++) {
    total_length += strlen(arr->arr[i]);
    total_length += delimit_length;
  }
  // Allocate memory for result string
  temp_ptr = result = malloc(total_length + 1);
  // Copy strarray with delimiter to result string
  for(i = 0; i < arr->length; i++) {
    strcpy(temp_ptr,arr->arr[i]);
    temp_ptr += strlen(arr->arr[i]);
    strcpy(temp_ptr, delimit);
    temp_ptr += delimit_length;
  }
  return result;
}

const char **
StrArray_get_array(StrArray arr) {
  return (const char**)arr->arr;
}

char **
StrArray_copy_array(StrArray arr) {
  char ** out = malloc(sizeof(char*)*arr->length);
  for (int i = 0; i < arr->length; i++) {
    printf("Copy array: %s\n", arr->arr[i]);
    out[i] = malloc(strlen(arr->arr[i])+1);
    strcpy(out[i], arr->arr[i]);
  }
  return out;
}

/*
const char*
StrArray_last(StrArray arr)
{
  if (arr->length == 0) {
    return NULL;
  }
  return arr->arr[arr->length-1];
}
*/
