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
dir_init(ramdir_t *d, ramdir_t *parent, char *fname)
{
	assert(d && fname);

	d->fname = strdup(fname);	//TODO: use user allocator
	if (parent) {
		d->parent = d;
		d->sb = parent->sb;
	}

	vector_init(&d->child_dirs, sizeof(ramdir_t *), d->sb->alloc);
	vector_init(&d->child_files, sizeof(ramfile_t *), d->sb-> alloc);
}

static void
dir_finalize(ramdir_t *d)
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

ramfile_t *
ramfs_file_new(ramdir_t *curdir, char *fpath)
{
	ramfile_t *f;
	ramdir_t *parent;

	parent = ramfs_lookup_dirname(curdir, fpath);
	if (!parent)
		return NULL;

	f = _alloc(curdir, sizeof(*f));

	f->sb = curdir->sb;
	f->parent = parent;
	f->fname = basename(fpath);

	ramfs_dir_add_file(parent, f);

	return f;
}

ramdir_t *
ramfs_dir_new(ramdir_t *curdir, char *fpath)
{
	ramdir_t *d;
	ramdir_t *parent;

	parent = ramfs_lookup_dirname(curdir, fpath);
	if (!parent)
		return NULL;

	d = _alloc(curdir, sizeof(*d));

	dir_init(d, parent, basename(fpath));
	ramfs_dir_add_dir(parent, d);

	return d;
}

int
ramfs_dir_add_file(ramdir_t *parent, ramfile_t *child)
{
	assert(child);
	vector_push(&parent->child_files, &child);
	return 0;
}

int
ramfs_dir_add_dir(ramdir_t *parent, ramdir_t *child)
{
	assert(child);
	vector_push(&parent->child_dirs, &child);
	return 0;
}

ramdir_t *
ramfs_mkdir(ramdir_t *curdir, char *filepath)
{
	TODO_WRITEME;
}


ramfile_t *
ramfs_lookup_file(ramdir_t *curdir, char *fpath)
{
	if (fpath == NULL)
		return NULL;

	return NULL;
}

ramdir_t *
ramfs_lookup_dir(ramdir_t *curdir, char *fpath)
{
	if (fpath == NULL)
		return NULL;

	return NULL;
}

ramdir_t *
ramfs_lookup_dirname(ramdir_t *curdir, char *fpath)
{
	if (fpath == NULL)
		return curdir;

	return NULL;
}

ramfile_t *
ramfs_file_open(ramdir_t *curdir, char *filepath, int flags)
{

	TODO_WRITEME;
}

void
ramfs_debug_ls(ramdir_t *d)
{
	int i, nfiles, ndirs;

	nfiles = vector_nmemb(&d->child_files);
	printf("dir %s, Nfiles = %d\n", d->fname, nfiles);
	for (i = 0; i < nfiles; i++) {
		ramfile_t *fp;
		fp = *(ramfile_t **)vector_get(&d->child_files, i);
		printf("\tfilename = %s\n", fp->fname);
	}

	ndirs = vector_nmemb(&d->child_dirs);
	printf("dir %s, Ndirs = %d\n", d->fname, ndirs);
	for (i = 0; i < ndirs; i++) {
		ramdir_t *fp;
		fp = *(ramdir_t **)vector_get(&d->child_dirs, i);
		printf("\tdirname = %s\n", fp->fname);
	}

	for (i = 0; i < ndirs; i++) {
		ramdir_t *fp;
		fp = *(ramdir_t **)vector_get(&d->child_dirs, i);
		ramfs_debug_ls(fp);
	}

}
