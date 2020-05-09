#include "cnxml.h"
#include "cnxml_common.h"
#include "cnxml_string.h"
#include "cnxml_hashmap.h"

/*** TOKENIZER ***/

cnxml_tokenizer* cnxml_tokenizer_new(cnxml_context* ctx, const char* data, size_t data_len) {
  if (data == NULL) {
    return (cnxml_tokenizer*)CNXML_ERROR_BADARGS;
  }
  cnxml_tokenizer* tokenizer = ctx->alloc(sizeof(cnxml_tokenizer));
  if (tokenizer == NULL) {
    return (cnxml_tokenizer*)CNXML_ERROR_ALLOCFAIL;
  }
  tokenizer->ctx = ctx;
  tokenizer->data = data;
  tokenizer->data_len = data_len;
  tokenizer->current_index = 0;
  tokenizer->current_line = 1;
  tokenizer->current_column = 1;
  return tokenizer;
}

static bool cnxml_tokenizer_is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool cnxml_tokenizer_is_punctuation_or_whitespace(char c) {
  return cnxml_tokenizer_is_whitespace(c) || c == '<' || c == '>' || c == '=' || c == '/';
}

bool cnxml_tokenizer_is_eof(cnxml_tokenizer* tokenizer) {
  return tokenizer->current_index >= tokenizer->data_len;
}

char cnxml_tokenizer_cur_char(cnxml_tokenizer* tokenizer) {
  if (tokenizer->current_index >= tokenizer->data_len) return '\0';
  return tokenizer->data[tokenizer->current_index];
}

char cnxml_tokenizer_peek(cnxml_tokenizer* tokenizer, int chars) {
  int idx = tokenizer->current_index + chars;
  if (idx >= tokenizer->data_len) return '\0';
  return tokenizer->data[idx];
}

void cnxml_tokenizer_move(cnxml_tokenizer* tokenizer, int chars) {
  int prev_index = tokenizer->current_index;
  tokenizer->current_index += chars;
  if (tokenizer->current_index >= tokenizer->data_len) {
    tokenizer->current_index = (int)tokenizer->data_len; // one over so that eof is detected
    return;
  }
  for (int i = prev_index; i < tokenizer->current_index && i < tokenizer->data_len; i++) {
    if (tokenizer->data[i] == '\n') {
      tokenizer->current_line += 1;
      tokenizer->current_column = 1;
    } else {
      tokenizer->current_column += 1;
    }
  }

}

bool cnxml_tokenizer_match_string(cnxml_tokenizer* tokenizer, cnxml_string str) {
  for (int i = 0; i < str.len; i++) {
    if (cnxml_tokenizer_peek(tokenizer, i) != str.ptr[i]) return false;
  }
  return true;
}

void cnxml_tokenizer_skip_whitespace(cnxml_tokenizer* tokenizer) {
  cnxml_string comment_start = cnxml_string_new("<!--");
  cnxml_string comment_end = cnxml_string_new("-->");
  cnxml_string special_start = cnxml_string_new("<?");
  cnxml_string special_end = cnxml_string_new("?>");

  while (!cnxml_tokenizer_is_eof(tokenizer)) {
    if (cnxml_tokenizer_is_whitespace(cnxml_tokenizer_cur_char(tokenizer))) {
      cnxml_tokenizer_move(tokenizer, 1);
    } else if (cnxml_tokenizer_match_string(tokenizer, comment_start)) {
      cnxml_tokenizer_move(tokenizer, (int)comment_start.len);
      while (!cnxml_tokenizer_is_eof(tokenizer) && !cnxml_tokenizer_match_string(tokenizer, comment_end)) {
        cnxml_tokenizer_move(tokenizer, 1);
      }
      if (cnxml_tokenizer_match_string(tokenizer, comment_end)) {
        cnxml_tokenizer_move(tokenizer, (int)comment_end.len);
      }
    } else if (cnxml_tokenizer_cur_char(tokenizer) == '<' && cnxml_tokenizer_peek(tokenizer, 1) == '!') {
      cnxml_tokenizer_move(tokenizer, 2);
      while (!cnxml_tokenizer_is_eof(tokenizer) && cnxml_tokenizer_cur_char(tokenizer) != '>') {
        cnxml_tokenizer_move(tokenizer, 1);
      }
      if (cnxml_tokenizer_cur_char(tokenizer) == '>') cnxml_tokenizer_move(tokenizer, 1);
    } else if (cnxml_tokenizer_match_string(tokenizer, special_start)) {
      cnxml_tokenizer_move(tokenizer, (int)special_start.len);
      while (!cnxml_tokenizer_is_eof(tokenizer) && !cnxml_tokenizer_match_string(tokenizer, special_end)) {
        cnxml_tokenizer_move(tokenizer, 1);
      }
      if (cnxml_tokenizer_match_string(tokenizer, special_end)) cnxml_tokenizer_move(tokenizer, (int)special_end.len);
    } else {
      break;
    }
  }
}

cnxml_string cnxml_tokenizer_read_quoted_string(cnxml_tokenizer* tokenizer) {
  int start_idx = tokenizer->current_index;
  int len = 0;
  while (!cnxml_tokenizer_is_eof(tokenizer) && cnxml_tokenizer_cur_char(tokenizer) != '"') {
    len += 1;
    cnxml_tokenizer_move(tokenizer, 1);
  }
  cnxml_tokenizer_move(tokenizer, 1);
  return cnxml_string_newlen(tokenizer->data + start_idx, len);
}

cnxml_string cnxml_tokenizer_read_unquoted_string(cnxml_tokenizer* tokenizer) {
  int start_idx = tokenizer->current_index;
  int len = 0;
  while (!cnxml_tokenizer_is_eof(tokenizer) && !cnxml_tokenizer_is_punctuation_or_whitespace(cnxml_tokenizer_cur_char(tokenizer))) {
    len += 1;
    cnxml_tokenizer_move(tokenizer, 1);
  }
  return cnxml_string_newlen(tokenizer->data + start_idx, len);
}

cnxml_token cnxml_tokenizer_next_token(cnxml_tokenizer* tokenizer) {
  cnxml_tokenizer_skip_whitespace(tokenizer);

  if (cnxml_tokenizer_is_eof(tokenizer)) return (cnxml_token){CNXML_TOKEN_EOF, CNXML_STRING_EMPTY};

  char c = cnxml_tokenizer_cur_char(tokenizer);
  cnxml_tokenizer_move(tokenizer, 1);

  cnxml_string str_lt = cnxml_string_newlen("<", 1);
  cnxml_string str_gt = cnxml_string_newlen(">", 1);
  cnxml_string str_slash = cnxml_string_newlen("/", 1);
  cnxml_string str_equal = cnxml_string_newlen("=", 1);

  switch (c) {
  case '\0': return (cnxml_token){CNXML_TOKEN_EOF, CNXML_STRING_EMPTY};
  case '<': return (cnxml_token){CNXML_TOKEN_OPENLESS, str_lt};
  case '>': return (cnxml_token){CNXML_TOKEN_CLOSEGREATER, str_gt};
  case '/': return (cnxml_token){CNXML_TOKEN_SLASH, str_slash};
  case '=': return (cnxml_token){CNXML_TOKEN_EQUAL, str_equal};
  case '"': return (cnxml_token){CNXML_TOKEN_STRING, cnxml_tokenizer_read_quoted_string(tokenizer)};
  default:
    // rewind one back because we just yoinked the first char
    tokenizer->current_index -= 1;
    return (cnxml_token){CNXML_TOKEN_STRING, cnxml_tokenizer_read_unquoted_string(tokenizer)};
  }
}

const char* cnxml_tokenizer_token_type_name(cnxml_token_type type) {
  switch(type) {
  case CNXML_TOKEN_UNKNOWN: return "UNKNOWN";
  case CNXML_TOKEN_OPENLESS: return "OPENLESS";
  case CNXML_TOKEN_CLOSEGREATER: return "CLOSEGREATER";
  case CNXML_TOKEN_SLASH: return "SLASH";
  case CNXML_TOKEN_EQUAL: return "EQUAL";
  case CNXML_TOKEN_STRING: return "STRING";
  case CNXML_TOKEN_EOF: return "EOF";
  default: return "UNKNOWN";
  }
}

void cnxml_tokenizer_print_token(FILE* f, cnxml_token tok) {
  fprintf(f, "TOKEN{");
  fprintf(f, cnxml_tokenizer_token_type_name(tok.type));
  fprintf(f, ", '");
  cnxml_string_print(f, tok.content);
  fprintf(f, "'}");
}

void cnxml_tokenizer_free(cnxml_tokenizer* tokenizer) {
  tokenizer->ctx->dealloc(tokenizer);
}

/*** PARSER ***/

cnxml_parser* cnxml_parser_new(cnxml_context* ctx, cnxml_tokenizer* tokenizer) {
  if (tokenizer == NULL) {
    return (cnxml_parser*)CNXML_ERROR_BADARGS;
  }
  if (ctx == NULL) {
     ctx = tokenizer->ctx;
  }
  cnxml_parser* parser = ctx->alloc(sizeof(cnxml_parser));
  if (parser == NULL) {
    return (cnxml_parser*)CNXML_ERROR_ALLOCFAIL;
  }

  parser->ctx = ctx;
  parser->tokenizer = tokenizer;
  parser->error_buffer = NULL;
  parser->error_count = 0;
  return parser;
}

bool cnxml_parser_has_errors(cnxml_parser* parser) {
  return parser->error_buffer != NULL;
}

const char* cnxml_parser_error_message(cnxml_context* ctx, cnxml_parser_error* err) {
  if (ctx == NULL || err == NULL) {
    return (const char*)CNXML_ERROR_BADARGS;
  }

  const char* base_msg;
  switch(err->type) {
  case CNXML_PARSER_ERROR_NO_CLOSING_SYMBOL_FOUND:
    base_msg = "No closing '>' found for element.";
    break;
  case CNXML_PARSER_ERROR_NO_OPENING_SYMBOL_FOUND:
    base_msg = "Couldn't find a '>' to start parsing with.";
    break;
  case CNXML_PARSER_ERROR_MISMATCHED_CLOSING_TAG:
    base_msg = "Closing element is in the wrong order.";
    break;
  case CNXML_PARSER_ERROR_MISSING_EQUALS_SIGN:
    base_msg = "Expected '=' after attribute.";
    break;
  case CNXML_PARSER_ERROR_MISSING_ATTRIBUTE_VALUE:
    base_msg = "Expected value after '=' in attribute, but none found.";
    break;
  case CNXML_PARSER_ERROR_MISSING_ELEMENT_NAME:
    base_msg = "Expected name of element after opening '<'.";
    break;
  case CNXML_PARSER_ERROR_TOO_MANY_ERRORS:
    base_msg = "Too many errors were thrown. No more errors will be reported.";
    break;
  default:
    base_msg = "Unknown error.";
    break;
  }

  size_t base_msg_len = strlen(base_msg);

  if (err->actual_name.len != 0) {
    if (err->expected_name.len == 0) {
      static const char* text = " Name: ";
      static const size_t text_len = 7;
      size_t new_buf_len = base_msg_len + text_len + err->actual_name.len + 1;
      char* new_buf = ctx->alloc(new_buf_len * sizeof(char));
      memcpy(new_buf, base_msg, base_msg_len);
      memcpy(new_buf + base_msg_len, text, text_len);
      memcpy(new_buf + base_msg_len + text_len, err->actual_name.ptr, err->actual_name.len);
      new_buf[new_buf_len - 1] = '\0';
      return new_buf;
    } else {
      static const char* expected = " Expected: ";
      static const size_t expected_len = 11;
      static const char* actual = ", got: ";
      static const size_t actual_len = 7;
      size_t new_buf_len = base_msg_len + expected_len + err->expected_name.len + actual_len + err->actual_name.len + 1;
      char* new_buf = ctx->alloc(new_buf_len * sizeof(char));
      size_t offs = 0;
      memcpy(new_buf + offs, base_msg, base_msg_len);
      offs += base_msg_len;
      memcpy(new_buf + offs, expected, expected_len);
      offs += expected_len;
      memcpy(new_buf + offs, err->expected_name.ptr, err->expected_name.len);
      offs += err->expected_name.len;
      memcpy(new_buf + offs, actual, actual_len);
      offs += actual_len;
      memcpy(new_buf + offs, err->actual_name.ptr, err->actual_name.len);
      new_buf[new_buf_len - 1] = '\0';
      return new_buf;
    }
  } else {
    return base_msg;
  }
}

void cnxml_parser_report_error(cnxml_parser* parser, cnxml_parser_error_type type, cnxml_string actual_name, cnxml_string expected_name) {
  if (parser->error_count >= CNXML_PARSER_ERROR_BUFFER_SIZE) {
    // no errors are reported if the buffer is full
    return;
  }

  cnxml_context* ctx = parser->ctx;
  cnxml_parser_error* err = ctx->alloc(sizeof(cnxml_parser_error));  

  err->type = type;
  err->actual_name = actual_name;
  err->expected_name = expected_name;
  err->line = parser->tokenizer->current_line;
  err->column = parser->tokenizer->current_column;
  err->message = cnxml_parser_error_message(ctx, err);

  if (parser->error_buffer == NULL) {
    parser->error_buffer = ctx->alloc(sizeof(cnxml_parser_error*) * CNXML_PARSER_ERROR_BUFFER_SIZE);
  }

  size_t idx = parser->error_count;
  parser->error_count += 1;
  parser->error_buffer[idx] = err;

  if (parser->error_count == CNXML_PARSER_ERROR_BUFFER_SIZE - 1) {
    // if buffer is 1 away from being full, a dummy error is added
    // informing the user of the fact that no more errors will be
    // reported
    err = ctx->alloc(sizeof(cnxml_parser_error));
    err->type = CNXML_PARSER_ERROR_TOO_MANY_ERRORS;
    err->actual_name = CNXML_STRING_EMPTY;
    err->expected_name = CNXML_STRING_EMPTY;
    err->line = parser->tokenizer->current_line;
    err->column = parser->tokenizer->current_column;
    err->message = cnxml_parser_error_message(ctx, err);
    parser->error_count = CNXML_PARSER_ERROR_BUFFER_SIZE;
    parser->error_buffer[CNXML_PARSER_ERROR_BUFFER_SIZE - 1] = err;
  }
}

void cnxml_parser_error_print(FILE* f, cnxml_parser_error* error) {
  fprintf(f, "%s [%d:%d]", error->message, error->line, error->column);
}

cnxml_element INTERNAL_cnxml_parser_read_element(cnxml_parser* parser, bool skip_opening_tag) {
  cnxml_token tok;
  if (!skip_opening_tag) {
    tok = cnxml_tokenizer_next_token(parser->tokenizer);
    if (tok.type != CNXML_TOKEN_OPENLESS) {
      cnxml_parser_report_error(parser, CNXML_PARSER_ERROR_NO_OPENING_SYMBOL_FOUND, CNXML_STRING_EMPTY, CNXML_STRING_EMPTY);
    }
  }
  tok = cnxml_tokenizer_next_token(parser->tokenizer);
  if (tok.type != CNXML_TOKEN_STRING) {
    cnxml_parser_report_error(parser, CNXML_PARSER_ERROR_MISSING_ELEMENT_NAME, CNXML_STRING_EMPTY, CNXML_STRING_EMPTY);
  }

  cnxml_element elem = cnxml_element_new(parser->ctx, tok.content);
  bool self_closing = false;

  while (true) {
    tok = cnxml_tokenizer_next_token(parser->tokenizer);
    switch (tok.type) {
    case CNXML_TOKEN_EOF:
      return elem;
    case CNXML_TOKEN_SLASH:
      if (cnxml_tokenizer_cur_char(parser->tokenizer) == '>') {
        cnxml_tokenizer_move(parser->tokenizer, 1);
        self_closing = true;
      }
      goto break_loop;
    case CNXML_TOKEN_CLOSEGREATER:
      goto break_loop;
    case CNXML_TOKEN_STRING:
      cnxml_parser_read_attribute(parser, &elem, tok.content);
      break;
    }
  }

break_loop:

  if (self_closing) return elem;

  while (true) {
    tok = cnxml_tokenizer_next_token(parser->tokenizer);
    switch (tok.type) {
    case CNXML_TOKEN_EOF:
      return elem;    
    case CNXML_TOKEN_OPENLESS:
      if (cnxml_tokenizer_cur_char(parser->tokenizer) == '/') {
        cnxml_tokenizer_move(parser->tokenizer, 1);

        cnxml_token end_name = cnxml_tokenizer_next_token(parser->tokenizer);
        if (end_name.type == CNXML_TOKEN_STRING && cnxml_string_equal(end_name.content, elem.name)) {
          cnxml_token close_greater = cnxml_tokenizer_next_token(parser->tokenizer);
          if (close_greater.type == CNXML_TOKEN_CLOSEGREATER) {
            return elem;
          } else {
            cnxml_parser_report_error(parser, CNXML_PARSER_ERROR_NO_CLOSING_SYMBOL_FOUND, end_name.content, CNXML_STRING_EMPTY);
          }
        } else {
          cnxml_parser_report_error(parser, CNXML_PARSER_ERROR_MISMATCHED_CLOSING_TAG, elem.name, end_name.content);
        }

        return elem;
      } else {
        if (elem.children == NULL) {
          elem.children = cnxml_element_list_new(parser->ctx);
        }
        cnxml_element_list_append(elem.children, INTERNAL_cnxml_parser_read_element(parser, true));
      }
      break;
    default:
      cnxml_element_add_text_content(&elem, tok.content);
    }
  }
}

cnxml_element cnxml_parser_read_element(cnxml_parser* parser) {
  return INTERNAL_cnxml_parser_read_element(parser, false);
}

void cnxml_parser_read_attribute(cnxml_parser* parser, cnxml_element* target, cnxml_string name) {
  cnxml_token tok = cnxml_tokenizer_next_token(parser->tokenizer);
  if (tok.type == CNXML_TOKEN_EQUAL) {
    tok = cnxml_tokenizer_next_token(parser->tokenizer);
    if (tok.type == CNXML_TOKEN_STRING) {
      cnxml_string* stored_value = cnxml_string_stored(parser->ctx, tok.content);
      cnxml_hashmap_put(target->attributes, name, stored_value);
    } else {
      cnxml_parser_report_error(parser, CNXML_PARSER_ERROR_MISSING_ATTRIBUTE_VALUE, name, CNXML_STRING_EMPTY);
    }
  } else {
    cnxml_parser_report_error(parser, CNXML_PARSER_ERROR_MISSING_EQUALS_SIGN, name, CNXML_STRING_EMPTY);
  }
}

void cnxml_parser_free(cnxml_parser* parser) {
  if (parser->error_buffer != NULL) {
    for (int i = 0; i < CNXML_PARSER_ERROR_BUFFER_SIZE; i++) {
      cnxml_parser_error* err = parser->error_buffer[i];
      parser->ctx->dealloc(err);
    }
    parser->ctx->dealloc(parser->error_buffer);
  }
  parser->ctx->dealloc(parser);
}


/*** MISCELLANEOUS ***/
cnxml_element_list* cnxml_element_list_new(cnxml_context* ctx) {
  cnxml_element_list* list = ctx->alloc(sizeof(cnxml_element_list));
  if (list == NULL) return (cnxml_element_list*)(CNXML_ERROR_ALLOCFAIL);
  list->ctx = ctx;
  list->capacity = CNXML_ELEMENT_LIST_GROW_AMOUNT;
  list->len = 0;
  list->ptr = ctx->alloc(sizeof(cnxml_element) * list->capacity);
  if (list->ptr == NULL) return (cnxml_element_list*)(CNXML_ERROR_ALLOCFAIL);
  return list; 
}

cnxml_error cnxml_element_list_append(cnxml_element_list* list, cnxml_element elem) {
  if (list->len == list->capacity) {
    int new_capacity = list->capacity + CNXML_ELEMENT_LIST_GROW_AMOUNT;
    void* new_ptr = list->ctx->realloc(list->ptr, sizeof(cnxml_element) * new_capacity);
    if (new_ptr == NULL) {
      return CNXML_ERROR_ALLOCFAIL;
    }
    list->capacity = new_capacity;
    list->ptr = new_ptr;
  }
  list->len += 1;
  list->ptr[list->len - 1] = elem;
  return CNXML_ERROR_OK;
}

cnxml_element* cnxml_element_list_get(cnxml_element_list* list, int index) {
  if (list == NULL) return NULL;
  if (index >= list->len) return NULL;
  return list->ptr + index;
}

int cnxml_element_list_length(cnxml_element_list* list) {
  if (list == NULL) return 0;
  return list->len;
}

void cnxml_element_list_free(cnxml_element_list* list) {
  if (list == NULL) return;
  list->ctx->dealloc(list->ptr);
}

cnxml_element cnxml_element_new(cnxml_context* ctx, cnxml_string name) {
  cnxml_element elem;
  elem.ctx = ctx;
  elem.name = name;
  elem.attributes = cnxml_hashmap_new(ctx);
  elem.children = NULL;
  elem.text_content = CNXML_STRING_EMPTY;
  return elem;
}

void cnxml_element_add_text_content(cnxml_element* elem, cnxml_string str) {
  if (elem->text_content.len == 0) {
    elem->text_content = str;
    return;
  }

  elem->text_content = cnxml_string_concat3(elem->ctx, elem->text_content, cnxml_string_newlen(" ", 1), str);
}

typedef struct {
  size_t attrs_len;
  int* attr_index;
  cnxml_element elem;
  cnxml_any writer_ud;
  cnxml_writer_func* writer;
} INTERNAL_cnxml_element_write_attr_iter_userdata;

int INTERNAL_cnxml_element_write_attr_iter(cnxml_any a_userdata, cnxml_string key, cnxml_any a_value) {
  INTERNAL_cnxml_element_write_attr_iter_userdata userdata = *((INTERNAL_cnxml_element_write_attr_iter_userdata*)a_userdata);
  cnxml_string value = *((cnxml_string*)a_value);

  userdata.writer(userdata.writer_ud, key.ptr, key.len);
  userdata.writer(userdata.writer_ud, "=\"", 2);
  userdata.writer(userdata.writer_ud, value.ptr, value.len);
  userdata.writer(userdata.writer_ud, "\"", 1);

  if (*userdata.attr_index != userdata.attrs_len - 1) userdata.writer(userdata.writer_ud, " ", 1);

  *userdata.attr_index += 1;

  return CNXML_MAP_OK;
}

void INTERNAL_cnxml_writer_writeline(cnxml_writer_func writer, cnxml_any writer_userdata, int indent, cnxml_string indent_str) {
  writer(writer_userdata, "\n", 1);
  for (int i = 0; i < indent; i++) {
    writer(writer_userdata, indent_str.ptr, indent_str.len);
  }
}

void INTERNAL_cnxml_element_write(cnxml_element elem, cnxml_writer_func writer, cnxml_any writer_userdata, int indent, cnxml_string indent_str) {
  writer(writer_userdata, "<", 1);
  writer(writer_userdata, elem.name.ptr, elem.name.len);

  size_t attrs_len = cnxml_hashmap_length(elem.attributes);

  if (attrs_len > 0) writer(writer_userdata, " ", 1);

  int attr_idx = 0;
  INTERNAL_cnxml_element_write_attr_iter_userdata attr_iter_userdata = (INTERNAL_cnxml_element_write_attr_iter_userdata){
    attrs_len,
    &attr_idx,
    elem,
    writer_userdata,
    writer
  };
  cnxml_hashmap_iterate(
    elem.attributes,
    INTERNAL_cnxml_element_write_attr_iter,
    &attr_iter_userdata
  );

  size_t child_count = cnxml_element_list_length(elem.children);

  if (child_count == 0 && elem.text_content.len == 0) {
    writer(writer_userdata, " />", 3);
    return;
  }

  writer(writer_userdata, ">", 1);

  indent += 1;
  INTERNAL_cnxml_writer_writeline(writer, writer_userdata, indent, indent_str);

  if (elem.text_content.len > 0) {
    writer(writer_userdata, elem.text_content.ptr, elem.text_content.len);
  }


  for (int i = 0; i < child_count; i++) {
    cnxml_element child = *cnxml_element_list_get(elem.children, i);
    INTERNAL_cnxml_element_write(child, writer, writer_userdata, indent, indent_str);
    if (i != child_count - 1) {
      INTERNAL_cnxml_writer_writeline(writer, writer_userdata, indent, indent_str);
    }
  }

  indent -= 1;
  INTERNAL_cnxml_writer_writeline(writer, writer_userdata, indent, indent_str);

  writer(writer_userdata, "</", 2);
  writer(writer_userdata, elem.name.ptr, elem.name.len);
  writer(writer_userdata, ">", 1);
}

void cnxml_element_write_indent(cnxml_element elem, cnxml_writer_func writer, cnxml_any userdata, cnxml_string indent_str) {
  INTERNAL_cnxml_element_write(elem, writer, userdata, 0, indent_str);
}

void cnxml_element_write(cnxml_element elem, cnxml_writer_func writer, cnxml_any userdata) {
  INTERNAL_cnxml_element_write(elem, writer, userdata, 0, cnxml_string_newlen("\t", 1));
}

void cnxml_element_free(cnxml_element elem) {
  for (size_t i = 0; i < cnxml_element_list_length(elem.children); i++) {
    cnxml_element child = *cnxml_element_list_get(elem.children, i);
    cnxml_element_free(child);
  }
  cnxml_element_free_alone(elem);
}

int INTERNAL_cnxml_element_free_attr_iter(cnxml_any a_userdata, cnxml_string key, cnxml_any a_value) {
  cnxml_context* ctx = (cnxml_context*)a_userdata;
  cnxml_string* stored_value = (cnxml_string*)a_value;

  ctx->dealloc(stored_value);
  return CNXML_MAP_OK;
}

void cnxml_element_free_alone(cnxml_element elem) {
  cnxml_hashmap_iterate(elem.attributes, INTERNAL_cnxml_element_free_attr_iter, elem.ctx);
  cnxml_hashmap_free(elem.attributes);
  cnxml_element_list_free(elem.children);
}