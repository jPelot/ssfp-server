#ifndef voidarray_h
#define voidarray_h

typedef struct void_array_t* VoidArray;

VoidArray VoidArray_create();
void      VoidArray_destroy(VoidArray arr);
int       VoidArray_length(VoidArray arr);

void     VoidArray_add(VoidArray arr, void *ptr);
void     VoidArray_set(VoidArray arr, void *ptr, int index);
void*    VoidArray_get(VoidArray arr, int index);
const void**   VoidArray_get_array(VoidArray); 
#endif // voidarray_h
