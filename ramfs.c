#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "ramfs.h"

#define TODO_WRITEME do {				\
	printf("TODO: WRITEME %s\n", __FUNCTION__);	\
	return 0;					\
} while (0)

#define NODE(n) ((mynode_t *)n)


static void *
_alloc(mynode_t *n, int sz) {
	void *ptr;

	assert(n && n->sb && n->sb->alloc);
	assert(ptr);

	ptr = n->sb->alloc(NULL, sz);
	memset(ptr, 0, sz);
	return ptr;

}

static void
dir_init(mydir_t *d, mydir_t *parent, char *fname)
{
	assert(d && fname);

	d->type = TYPE_DIR;
	d->fname = strdup(fname);	//TODO: use user allocator
	if (parent) {
		d->parent = d;
		d->sb = parent->sb;
	}

	vector_init(&d->child_dirs, sizeof(mydir_t *), d->sb->alloc);
	vector_init(&d->child_files, sizeof(myfile_t *), d->sb-> alloc);
}

static void
dir_finalize(mydir_t *d)
{
	int nfiles, ndirs;

	assert(d);
	nfiles = vector_nmemb(&d->child_files);
	ndirs = vector_nmemb(&d->child_dirs);

	assert(nfiles == 0 && ndirs == 0);

	vector_free(&d->child_files);
	vector_free(&d->child_dirs);

	free(d->fname);
	d->sb->alloc(d, 0);
}

void
ramfs_init(superblock_t *sb, allocator_t alloc)
{
	sb->alloc = alloc;
	sb->root = alloc(NULL, sizeof(*sb->root));
	assert(sb->root);

	sb->root->sb = sb;
	sb->root->parent = sb->root;
	dir_init(sb->root, NULL, "/");
}

myfile_t *
ramfs_file_new(mydir_t *curdir, char *fpath)
{
	myfile_t *f;
	mydir_t *parent;

	parent = ramfs_lookup_dirname(curdir, fpath);
	if (!parent)
		return NULL;

	f = _alloc(curdir, sizeof(*f));

	f->type = TYPE_FILE;
	f->sb = curdir->sb;
	f->parent = parent;
	f->fname = basename(fpath);

	ramfs_dir_add(parent, (mynode_t *)f);

	return f;
}

mydir_t *
ramfs_dir_new(mydir_t *curdir, char *fpath)
{
	mydir_t *d;
	mydir_t *parent;

	parent = ramfs_lookup_dirname(curdir, fpath);
	if (!parent)
		return NULL;

	d = _alloc(curdir, sizeof(*d));

	dir_init(d, parent, basename(fpath));
	ramfs_dir_add(parent, d);

	return d;
}

int
ramfs_dir_add(mydir_t *parent, mynode_t *child)
{
	vector *vec;

	assert(child);

	if (child->type == TYPE_FILE) {
		vec = &parent->child_files;
	} else if (child->type == TYPE_DIR) {
		vec = &parent->child_dirs;
	} else {
		assert(0);
	}

	vector_push(vec, &child);

	return 0;
}

mydir_t *
ramfs_mkdir(mydir_t *curdir, char *filepath)
{
	TODO_WRITEME;
}

mynode_t *
ramfs_lookup(mydir_t *curdir, char *fpath)
{
	if (fpath == NULL)
		return NULL;

	return NULL;
}

mynode_t *
ramfs_lookup_dirname(mydir_t *curdir, char *fpath)
{
	if (fpath == NULL)
		return curdir;

	return curdir;
	return NULL;
}

myfile_t *
ramfs_file_open(mydir_t *curdir, char *filepath, int flags)
{

	TODO_WRITEME;
}

void
ramfs_debug_ls(mydir_t *d)
{
	int i, nfiles, ndirs;

	nfiles = vector_nmemb(&d->child_files);
	printf("dir %s, Nfiles = %d\n", d->fname, nfiles);
	for (i = 0; i < nfiles; i++) {
		myfile_t *fp;
		fp = *(myfile_t **)vector_get(&d->child_files, i);
		printf("\tfilename = %s\n", fp->fname);
	}

	ndirs = vector_nmemb(&d->child_dirs);
	printf("dir %s, Ndirs = %d\n", d->fname, ndirs);
	for (i = 0; i < ndirs; i++) {
		mydir_t *fp;
		fp = *(mydir_t **)vector_get(&d->child_dirs, i);
		printf("\tdirname = %s\n", fp->fname);
	}

	for (i = 0; i < ndirs; i++) {
		mydir_t *fp;
		fp = *(mydir_t **)vector_get(&d->child_dirs, i);
		ramfs_debug_ls(fp);
	}

}
