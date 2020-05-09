#include "cnxml_common.h"

cnxml_context* cnxml_context_new(cnxml_alloc_func* alloc, cnxml_realloc_func* realloc, cnxml_dealloc_func* dealloc) {\
  if (alloc == NULL || realloc == NULL || dealloc == NULL) return (cnxml_context*)CNXML_ERROR_BADARGS;
  // allocate the context itself with the alloc func
  cnxml_context* ctx = alloc(sizeof(cnxml_context));
  if (ctx == NULL) return (cnxml_context*)CNXML_ERROR_ALLOCFAIL;
  ctx->alloc = alloc;
  ctx->realloc = realloc;
  ctx->dealloc = dealloc;
  return ctx;
}

void cnxml_context_free(cnxml_context* ctx) {
	ctx->dealloc(ctx);
}