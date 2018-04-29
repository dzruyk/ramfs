#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ramfs.h"

//WARN: gcc feature used
#define ALLOC(sz) ({					\
		assert(curdir && curdir->sb && curdir->sb->alloc); \
		void *ptr = curdir->sb->alloc(sz);	\
		assert(ptr);				\
		memset(ptr, 0, sz);			\
		ptr;					\
})

void
ramfs_init(superblock_t *sb, allocator_t alloc)
{
	sb->alloc = alloc;
	sb->root = alloc(sizeof(*sb->root));
	assert(sb->root);
	memset(sb->root, 0, sizeof(*sb->root));

	sb->root->parent = sb->root;
	sb->root->sb = sb;

	assert(sb->root != 0);
}

myfile_t *
ramfs_file_new(mydir_t *curdir, char *fpath)
{
	myfile_t *f;

	f = ALLOC(sizeof(*f));

	memset(f, 0, sizeof(*f));
}

mydir_t *
ramfs_dir_new(mydir_t *curdir, char *fpath)
{
	mydir_t *d;
	mydir_t *parent;

	assert(sb->root);

	parent = ramfs_lookup(sb, fpath);
	if (!parent)
		return NULL;

	d = ALLOC(sizeof(*d));

	d->parent = parent;

	ramfs_dir_add(parent, d);

	return d;
}


mydir_t *
ramfs_mkdir(mydir_t *curdir, char *filepath)
{

}
