#ifndef intarray_h
#define intarray_h

typedef struct int_array_t* IntArray;

IntArray IntArray_create();
void     IntArray_destroy(IntArray arr);
void     IntArray_add(IntArray arr, int num);
int      IntArray_get(IntArray arr, int index);
int      IntArray_length(IntArray arr);
const int* IntArray_get_array(IntArray arr);
// Iteration functions
void IntArray_start(IntArray arr);
int  IntArray_cur(IntArray arr);
int  IntArray_next(IntArray arr);
int  IntArray_at_end(IntArray arr);

#endif // intarray_h
