#include "cnxml_string.h"

/*** STRING ***/

cnxml_string cnxml_string_newlen(const char* cstring, size_t len) {
  if (cstring == NULL) {
    return (cnxml_string){NULL, 0};
  }
  return (cnxml_string){cstring, len};
}

cnxml_string cnxml_string_new(const char* cstring) {
  if (cstring == NULL) {
    return (cnxml_string){NULL, 0};
  }
  return cnxml_string_newlen(cstring, strlen(cstring));
}

cnxml_string cnxml_string_sub(cnxml_string str, int start_pos, int len) {
  if (start_pos == 0 && len == str.len) return str;
  if (start_pos > str.len || len > (str.len - start_pos - 1)) {
    return (cnxml_string){NULL, 0};
  } else {
    return (cnxml_string){str.ptr + start_pos, len};
  }
}

bool cnxml_string_equal(cnxml_string a, cnxml_string b) {
	if (a.len != b.len) return false;
	for (int i = 0; i < a.len; i++) {
		if (a.ptr[i] != b.ptr[i]) return false;
	}
	return true;
}

bool cnxml_string_cequal(cnxml_string a, const char* b) {
	int b_len = strlen(b);
	if (a.len != b_len) return false;
	for (int i = 0; i < a.len; i++) {
		if (a.ptr[i] != b[i]) return false;
	}
	return true;
}

void cnxml_string_print(FILE* f, cnxml_string str) {
  if (str.ptr == NULL) return;
  for (size_t i = 0; i < str.len; i++) {
    fprintf(f, "%c", str.ptr[i]);
  }
}

cnxml_string cnxml_string_concat(cnxml_context* ctx, cnxml_string a, cnxml_string b) {
	char* new_buf = ctx->alloc(a.len + b.len);
	for (size_t i = 0; i < a.len; i++) {
		new_buf[i] = a.ptr[i];
	}
	for (size_t i = 0; i < b.len; i++) {
		new_buf[a.len + i] = b.ptr[i];
	}
	return (cnxml_string){new_buf, a.len + b.len};
}


cnxml_string cnxml_string_concat3(cnxml_context* ctx, cnxml_string a, cnxml_string b, cnxml_string c) {
	const size_t new_len = a.len + b.len + c.len;
	char* new_buf = ctx->alloc(new_len);
	for (size_t i = 0; i < a.len; i++) {
		new_buf[i] = a.ptr[i];
	}
	for (size_t i = 0; i < b.len; i++) {
		new_buf[a.len + i] = b.ptr[i];
	}
	for (size_t i = 0; i < c.len; i++) {
		new_buf[a.len + b.len + i] = c.ptr[i];
	}
	return (cnxml_string){new_buf, new_len};
}

cnxml_string* cnxml_string_stored(cnxml_context* ctx, cnxml_string str) {
	cnxml_string* new_str = ctx->alloc(sizeof(cnxml_string));
	new_str->ptr = str.ptr;
	new_str->len = str.len;
	return new_str;
}

void cnxml_string_extract(cnxml_string str, const char** cstring_out, size_t* len_out) {
    if (cstring_out != NULL) *cstring_out = str.ptr;
    if (len_out != NULL) *len_out = str.len;
}
