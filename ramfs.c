#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ramfs.h"

//WARN: gcc feature used
#define ALLOC(sz) {				\
		  void *ptr = sb->alloc(sz);	\
		  assert(ptr);			\
		  memset(ptr, 0, sz);		\
		  ptr;				\
}

void
ramfs_init(superblock_t *sb, allocator_t alloc)
{
	sb->alloc = alloc;
	sb->root = ramfs_dir_new(sb, "/");
	sb->root->parent = sb->root;

	assert(sb->root != 0)
}

myfile_t *
ramfs_file_new(superblock_t *sb, char *fpath)
{
	memset(sb, 0, sizeof(*dst));
}

mydir_t *
ramfs_dir_new(superblock_t *sb, char *fpath)
{
	mydir_t *d;
	mydir_t *parent;

	d = ALLOC(sizeof(*d));

	if (strcmp(fpath, "/") == 0) {
		//special case, init root fs
		parent = sb
	} else {
		assert(sb->root);
		parent = ramfs_lookup(sb, fpath);
		if (!parent)
			return NULL;
	}


	d->parent = parent;
	
	return 0;
}


mydir_t *
ramfs_mkdir(mydir_t *curdir, char *filepath)
{

}
