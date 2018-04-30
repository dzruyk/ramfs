#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <string.h>

#include "macros.h"

typedef int boolean_t;

typedef enum {
	ret_not_found     = -6,
	ret_entry_exists  = -5,
        ret_no_access     = -4,
        ret_out_of_memory = -3,
        ret_invalid       = -2,
        ret_err           = -1,
        ret_ok            = 0,
} ret_t;

void *xmalloc(size_t sz);
void *xrealloc(void *ptr, size_t sz);
void *xstrdup(char *str);
void xfree(void *data);


#endif /*COMMON_H_*/
