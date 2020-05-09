#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "cnxml_common.h"
#include "cnxml_hashmap.h"
#include "cnxml_string.h"
#ifndef CNXML_H
#define CNXML_H

#define CNXML_IS_ERROR(val) (((size_t)val >= CNXML_ERRORPTR_FIRST) && ((size_t)val <= CNXML_ERRORPTR_LAST))

typedef enum {
  CNXML_TOKEN_UNKNOWN,
  CNXML_TOKEN_OPENLESS,
  CNXML_TOKEN_CLOSEGREATER,
  CNXML_TOKEN_SLASH,
  CNXML_TOKEN_EQUAL,
  CNXML_TOKEN_STRING,
  CNXML_TOKEN_EOF,
} cnxml_token_type;

typedef struct {
  cnxml_context* ctx;
  const char* data;
  size_t data_len;
  int current_index;
  int current_line;
  int current_column;
} cnxml_tokenizer;

typedef struct {
  cnxml_token_type type;
  cnxml_string content;
} cnxml_token;

typedef enum {
  CNXML_PARSER_ERROR_NO_CLOSING_SYMBOL_FOUND,
  CNXML_PARSER_ERROR_NO_OPENING_SYMBOL_FOUND,
  CNXML_PARSER_ERROR_MISMATCHED_CLOSING_TAG,
  CNXML_PARSER_ERROR_MISSING_EQUALS_SIGN,
  CNXML_PARSER_ERROR_MISSING_ATTRIBUTE_VALUE,
  CNXML_PARSER_ERROR_MISSING_ELEMENT_NAME,
  CNXML_PARSER_ERROR_TOO_MANY_ERRORS
} cnxml_parser_error_type;

typedef struct {
  cnxml_parser_error_type type;
  cnxml_string expected_name; // OPTIONAL
  cnxml_string actual_name;   // OPTIONAL
  int line;
  int column;
  const char* message;
} cnxml_parser_error;

typedef struct {
  cnxml_context* ctx;
  cnxml_tokenizer* tokenizer;
  cnxml_parser_error** error_buffer; // NULL IF NO ERRORS!
  size_t error_count;               // 0 IF NO ERRORS
} cnxml_parser;

typedef struct _cnxml_element_list cnxml_element_list;

typedef struct {
  cnxml_context* ctx;
  cnxml_string name;
  cnxml_map attributes;
  cnxml_element_list* children;
  cnxml_string text_content;
} cnxml_element;

struct _cnxml_element_list {
  cnxml_context* ctx;
  cnxml_element* ptr;
  int len;
  int capacity;
};

typedef void cnxml_writer_func(cnxml_any userdata, const char* buffer, size_t length);

#define CNXML_ELEMENT_LIST_GROW_AMOUNT 16
#define CNXML_PARSER_ERROR_BUFFER_SIZE 16

/*** TOKENIZER API ***/
CNXML_EXPORT cnxml_tokenizer* CNXML_API cnxml_tokenizer_new(cnxml_context* ctx, const char* data, size_t data_len);
static CNXML_EXPORT bool CNXML_API cnxml_tokenizer_is_whitespace(char c);
static CNXML_EXPORT bool CNXML_API cnxml_tokenizer_is_punctuation_or_whitespace(char c);
CNXML_EXPORT bool CNXML_API cnxml_tokenizer_is_eof(cnxml_tokenizer* tokenizer);
CNXML_EXPORT char CNXML_API cnxml_tokenizer_cur_char(cnxml_tokenizer* tokenizer);
CNXML_EXPORT char CNXML_API cnxml_tokenizer_peek(cnxml_tokenizer* tokenizer, int chars);
CNXML_EXPORT void CNXML_API cnxml_tokenizer_move(cnxml_tokenizer* tokenizer, int chars);
CNXML_EXPORT bool CNXML_API cnxml_tokenizer_match_string(cnxml_tokenizer* tokenizer, cnxml_string str);
CNXML_EXPORT void CNXML_API cnxml_tokenizer_skip_whitespace(cnxml_tokenizer* tokenizer);
CNXML_EXPORT cnxml_string CNXML_API cnxml_tokenizer_read_quoted_string(cnxml_tokenizer* tokenizer);
CNXML_EXPORT cnxml_string CNXML_API cnxml_tokenizer_read_unquoted_string(cnxml_tokenizer* tokenizer);
CNXML_EXPORT cnxml_token CNXML_API cnxml_tokenizer_next_token(cnxml_tokenizer* tokenizer);
CNXML_EXPORT const char* CNXML_API cnxml_tokenizer_token_type_name(cnxml_token_type type);
CNXML_EXPORT void CNXML_API cnxml_tokenizer_print_token(FILE* f, cnxml_token tok);
CNXML_EXPORT void CNXML_API cnxml_tokenizer_free(cnxml_tokenizer* tokenizer);

/*** PARSER API ***/
CNXML_EXPORT cnxml_parser* CNXML_API cnxml_parser_new(cnxml_context* ctx, cnxml_tokenizer* tokenizer);
CNXML_EXPORT bool CNXML_API cnxml_parser_has_errors(cnxml_parser* parser);
CNXML_EXPORT const char* CNXML_API cnxml_parser_error_message(cnxml_context* ctx, cnxml_parser_error* err);
CNXML_EXPORT void CNXML_API cnxml_parser_report_error(cnxml_parser* parser, cnxml_parser_error_type type, cnxml_string actual_name, cnxml_string expected_name);
CNXML_EXPORT void CNXML_API cnxml_parser_error_print(FILE* f, cnxml_parser_error* error);
CNXML_EXPORT cnxml_element CNXML_API cnxml_parser_read_element(cnxml_parser* parser);
CNXML_EXPORT void CNXML_API cnxml_parser_read_attribute(cnxml_parser* parser, cnxml_element* target, cnxml_string name);
CNXML_EXPORT void CNXML_API cnxml_parser_free(cnxml_parser* parser);

/*** MISCELLANEOUS ***/
CNXML_EXPORT cnxml_element_list* CNXML_API cnxml_element_list_new(cnxml_context* ctx);
CNXML_EXPORT cnxml_error CNXML_API cnxml_element_list_append(cnxml_element_list* list, cnxml_element elem);
CNXML_EXPORT cnxml_element* CNXML_API cnxml_element_list_get(cnxml_element_list* list, int index);
CNXML_EXPORT int CNXML_API cnxml_element_list_length(cnxml_element_list* list);
CNXML_EXPORT void CNXML_API cnxml_element_list_free(cnxml_element_list* list);
CNXML_EXPORT cnxml_element CNXML_API cnxml_element_new(cnxml_context* ctx, cnxml_string name);
CNXML_EXPORT void CNXML_API cnxml_element_add_text_content(cnxml_element* elem, cnxml_string str);
CNXML_EXPORT void CNXML_API cnxml_element_write(cnxml_element elem, cnxml_writer_func writer, cnxml_any userdata);
CNXML_EXPORT void CNXML_API cnxml_element_write_indent(cnxml_element elem, cnxml_writer_func writer, cnxml_any userdata, cnxml_string indent_str);
CNXML_EXPORT void CNXML_API cnxml_element_free(cnxml_element elem);
CNXML_EXPORT void CNXML_API cnxml_element_free_alone(cnxml_element elem);

#endif CNXML_H

