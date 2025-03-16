#ifndef strarray_h
#define strarray_h

typedef struct string_array_t* StrArray;

StrArray StrArray_create();
void     StrArray_destroy(StrArray arr);
void     StrArray_add(StrArray arr, const char *str);
void     StrArray_set(StrArray arr, int index, const char *str);
int      StrArray_length(StrArray arr);

// StrArray_get():
// Returns the C string at the index. If index is out of bounds,
// NULL will be returned. Negative indexes will index the array
// starting from the end.
const char* StrArray_get(StrArray arr, int index);

// StrArray_last():
// Returns the last C string in the array. If array is empty,
// then NULL is returned.
//const char* StrArray_last(StrArray arr);

// StrArray_add_arr():
// Adds the contents of array that into arr
void    StrArray_add_arr(StrArray arr, StrArray that);

char* StrArray_combine(StrArray arr);
char* StrArray_splice(StrArray arr, const char* str);

const char ** StrArray_get_array(StrArray);
char ** StrArray_copy_array(StrArray);
#endif // strarray_h
