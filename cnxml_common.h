#ifndef CNXML_COMMON_MACRO
#define CNXML_COMMON_MACRO

#include <stddef.h>

// apart from the cnxml_error enum,
// cnxml also encodes errors in pointer return values
// using predefined pointers in the 1-20 range

typedef enum {
	CNXML_ERROR_OK = 0,
	CNXML_ERROR_ALLOCFAIL = 1,
	CNXML_ERROR_BADARGS = 2
} cnxml_error;

#define CNXML_ERRORPTR_FIRST ((size_t)1)
#define CNXML_ERRORPTR_LAST ((size_t)20)

// #define CNXML_ERROR_ALLOCFAIL ((size_t)1)
// #define CNXML_ERROR_BADARGS ((size_t)2)

#ifdef _MSC_VER
  #define CNXML_EXPORT __declspec(dllexport)
  #define CNXML_API __cdecl
#else
  #define CNXML_EXPORT
  #define CNXML_API
#endif

typedef void* cnxml_alloc_func(size_t size);
typedef void* cnxml_realloc_func(void* ptr, size_t new_size);
typedef void cnxml_dealloc_func(void* ptr);

typedef struct {
  cnxml_alloc_func* alloc;
  cnxml_realloc_func* realloc;
  cnxml_dealloc_func* dealloc;
} cnxml_context;

CNXML_EXPORT cnxml_context* CNXML_API cnxml_context_new(cnxml_alloc_func* alloc, cnxml_realloc_func* realloc, cnxml_dealloc_func* dealloc);
CNXML_EXPORT void CNXML_API cnxml_context_free(cnxml_context* ctx);

#endif//CNXML_COMMON_MACRO