#include <stdlib.h>
#include <string.h>

#include "common.h"

void *
xmalloc(size_t sz)
{
	void *tmp;
	if ((tmp = malloc(sz)) == NULL)
		error(1, "malloc_err");
	return tmp;
}

void *
xrealloc(void *ptr, size_t sz)
{
	void *tmp;
	if ((tmp = realloc(ptr, sz)) == NULL)
		error(1, "malloc_err");
	return tmp;
}

void *
xstrdup(char *str)
{
	char *dup;

	dup = strdup(str);
	if (dup == NULL)
		error(1, "malloc_err");

	return dup;
}

/*
 * free data if its not NULL
 */
void
xfree(void *data)
{
	if (data != NULL)
		free(data);
}


