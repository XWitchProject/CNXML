#include "cnxml.h"
#include <stdio.h>
#include <stdlib.h>
#include "cnxml_hashmap.h"
//#include <gperftools/profiler.h>

int attr_iter(cnxml_any userdata, cnxml_string key, cnxml_any val) {
  cnxml_string valstr = *((cnxml_string*)val);

  cnxml_string_print(stdout, key);
  printf("=\"");
  cnxml_string_print(stdout, valstr);
  printf("\"\n");

  return CNXML_MAP_OK;
}

void writer(cnxml_any userdata, const char* data, size_t len) {
  FILE* f = (FILE*)userdata;
  fwrite(data, 1, len, f);
}

int main(int argc, const char** argv) {
  //ProfilerStart("test.log");
  if (argc < 2) {
    printf("need at least one arg\n");
    return 1;
  }

  const char* path = argv[1];
  cnxml_context* ctx = cnxml_context_new(malloc, realloc, free);

  FILE* f = fopen(path, "r");
  fseek(f, 0, SEEK_END);
  size_t length = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buf = ctx->alloc(length);
  fread(buf, 1, length, f);
  fclose(f);

  cnxml_tokenizer* tokenizer = cnxml_tokenizer_new(ctx, buf, length);

  cnxml_parser* parser = cnxml_parser_new(NULL, tokenizer);

  cnxml_element elem = cnxml_parser_read_element(parser);

  for (int i = 0; i < parser->error_count; i++) {
    cnxml_parser_error* err = parser->error_buffer[i];
    cnxml_parser_error_print(stdout, err);
    printf("\n");
  }

  // printf("NAME: ");
  // cnxml_string_print(stdout, elem.name);
  // printf("\n");

  // printf("CHILDCOUNT: %d\n", cnxml_element_list_length(elem.children));

  // printf("TEXT: ");
  // cnxml_string_print(stdout, elem.text_content);
  // printf("\n");

  printf("ATTRIBUTES:\n");
  cnxml_hashmap_iterate(elem.attributes, attr_iter, NULL);

  // while (!cnxml_tokenizer_is_eof(tokenizer)) {
  //   cnxml_token t = cnxml_tokenizer_next_token(tokenizer);
  //   cnxml_tokenizer_print_token(stdout, t);
  // }

  cnxml_element_write(elem, writer, stdout);

  free(buf);
  cnxml_element_free(elem);
  cnxml_parser_free(parser);
  cnxml_tokenizer_free(tokenizer);
  cnxml_context_free(ctx);
  //ProfilerStop();
  return 0;
}
