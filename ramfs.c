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


static void *
_alloc(ramnode_t *n, int sz) {
	void *ptr;

	assert(n && n->sb && n->sb->alloc);

	ptr = n->sb->alloc(NULL, sz);
	assert(ptr);
	memset(ptr, 0, sz);

	return ptr;
}

static int
ramfs_hash_cmp(const void *a, const void *b)
{
	assert(a && b);

	return strcmp((char*)a, (char*)b);
}

static unsigned long
ramfs_hash_cb(const void *data)
{
	char *s;
	int i, mult, res;

	assert(data);

	mult = 31;
	res = 0;
	s = (char*)data;

	for (i = 0; i < strlen(data); i++)
		res = res * mult + s[i];
	return res;
}

static void
dir_init(ramdir_t *d, ramdir_t *parent, char *fname)
{
	assert(d && fname);

	d->type = TYPE_DIR;
	d->fname = strdup(fname);	//TODO: use user allocator
	if (parent) {
		d->parent = d;
		d->sb = parent->sb;
	}

	d->kids = hash_table_new(0, ramfs_hash_cb, ramfs_hash_cmp);
}

static void
dir_finalize(ramdir_t *d)
{
	int nkids;

	assert(d);
	nkids = d->kids->size;

	assert(nkids == 0);

	hash_table_destroy(&d->kids);

	free(d->fname);
	d->sb->alloc(d, 0);
}

static void
file_finalize(ramfile_t *f)
{
	if (f->datalen > 0)
		f->sb->alloc(f->data, 0);
	f->sb->alloc(f, 0);
}

static ramnode_t *
dir_search(ramdir_t *curdir, char *filename)
{
	ramnode_t *n = NULL;

	//special cases
	if (strcmp(filename, ".") == 0)
		return curdir;

	if (strcmp(filename, "..") == 0)
		return curdir->parent;

	hash_table_lookup(curdir->kids, filename, (void **)&n);

	return n;
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

	f->type = TYPE_FILE;
	f->sb = curdir->sb;
	f->parent = parent;
	f->fname = basename(fpath);

	ramfs_dir_add(parent, RAMNODE(f));

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
	ramfs_dir_add(parent, d);

	return d;
}

int
ramfs_dir_add(ramdir_t *parent, ramnode_t *child)
{
	assert(child);
	hash_table_insert_unique(parent->kids, child->fname, child);
	return 0;
}

ramdir_t *
ramfs_mkdir(ramdir_t *curdir, char *filepath)
{
	TODO_WRITEME;
}


ramnode_t *
ramfs_lookup(ramdir_t *curdir, char *fpath)
{
	ramdir_t *d;

	if (fpath == NULL)
		return NULL;

	d = ramfs_lookup_dirname(curdir, fpath);
	if (!d)
		return NULL;

	return dir_search(d, basename(fpath));
}

ramdir_t *
ramfs_lookup_dirname(ramdir_t *curdir, char *fpath)
{
	ramdir_t *child;
	char *ptr;
	char buf[128];

	if (fpath == NULL)
		return curdir;

	if (*fpath == '/') {
		curdir = curdir->sb->root;
		fpath++;
	}

	while (fpath && *fpath) {
		ptr = strchr(fpath, '/');
		if (!ptr) break;
		// it's last component
		if (ptr[1] == '\0') break;

		//FIXME: strncpy is error prone
		memset(buf, 0, sizeof(buf));
		strncpy(buf, fpath, ptr - fpath);

		child = dir_search(curdir, buf);
		if (!child || child->type != TYPE_DIR)
			return NULL;

		curdir = child;
		fpath = ptr + 1;
	}

	return curdir;
}

ramfile_t *
ramfs_file_open(ramdir_t *curdir, char *filepath, int flags)
{
	ramfile_t *f;

	//TODO: handle flags

	f = (ramfile_t *)ramfs_lookup(curdir, filepath);
	if (f == NULL || f->type != TYPE_FILE)
		return NULL;

	f->nrefs++;
	return f;
}

int
ramfs_file_close(ramfile_t *fp)
{
	fp->nrefs--;

	if (!fp->parent && fp->nrefs <= 0) {
		file_finalize(fp);
	}

	return 0;
}

int
ramfs_file_read(ramfile_t *fp, char *buf, int sz, off_t off)
{
	int n;

	assert(buf);

	if (fp->datalen == 0 || off >= fp->datalen)
		return 0;

	n = sz;
	if (sz + off > fp->datalen)
		n = fp->datalen - off;
	memcpy(buf, fp->data + off, n);

	return n;
}

int
ramfs_file_write(ramfile_t *fp, const char *buf, int sz, off_t  off)
{
	void *ptr;

	assert(buf && sz >= 0);

	if (fp->datalen < off + sz) {
		fp->datalen = off + sz;

		ptr = fp->sb->alloc(fp->data, fp->datalen);
		if (ptr == NULL)
			return -1;
		fp->data = ptr;
	}
	memcpy(fp->data + off, buf, sz);

	return sz;
}

void
ramfs_debug_ls(ramdir_t *d)
{
	struct hash_table_iter *iter;
	ramdir_t *dp;
	char *fname;

	printf("dir %s, nchilds = %ld\n", d->fname, d->kids->count);
	iter = hash_table_iterate_init(d->kids);
	while (hash_table_iterate(iter, (void **)&fname, (void **)&dp)) {
		printf("\tfilename = %s\n", dp->fname);
	}
	hash_table_iterate_deinit(&iter);

	iter = hash_table_iterate_init(d->kids);
	while (hash_table_iterate(iter, (void **)&fname, (void **)&dp)) {
		if (dp->type != TYPE_DIR) continue;
		ramfs_debug_ls(dp);
	}
	hash_table_iterate_deinit(&iter);

}
