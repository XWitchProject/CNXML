#ifndef CNXML_STRING
#define CNXML_STRING

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "cnxml_common.h"

typedef struct {
  char* ptr;
  size_t len;
} cnxml_string;

#define CNXML_STRING_EMPTY ((cnxml_string){NULL, 0})

/*** STRING API ***/

CNXML_EXPORT cnxml_string CNXML_API cnxml_string_new(const char* cstring);
CNXML_EXPORT cnxml_string CNXML_API cnxml_string_newlen(const char* cstring, size_t len);
CNXML_EXPORT cnxml_string CNXML_API cnxml_string_sub(cnxml_string str, int start_pos, int len);
CNXML_EXPORT cnxml_string CNXML_API cnxml_string_concat(cnxml_context* ctx, cnxml_string a, cnxml_string b);
CNXML_EXPORT cnxml_string CNXML_API cnxml_string_concat3(cnxml_context* ctx, cnxml_string a, cnxml_string b, cnxml_string c);
CNXML_EXPORT cnxml_string* CNXML_API cnxml_string_stored(cnxml_context* ctx, cnxml_string str);
CNXML_EXPORT bool CNXML_API cnxml_string_equal(cnxml_string a, cnxml_string b);
CNXML_EXPORT bool CNXML_API cnxml_string_cequal(cnxml_string a, const char* b);
CNXML_EXPORT void CNXML_API cnxml_string_print(FILE* f, cnxml_string str);

#endif//CNXML_STRING