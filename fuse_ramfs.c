/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <assert.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "ramfs.h"
#include "hash/hash.h"
#include "hash/common.h"

superblock_t *ramfs;

void*
my_realloc(void *ptr, size_t sz)
{
	return xrealloc(ptr, sz);
}

static int
fuseram_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	ramnode_t *n;

	memset(stbuf, 0, sizeof(struct stat));
	n = ramfs_lookup(ramfs->root, path);
	if (!n)
		return -ENOENT;

	*stbuf = n->attr;

	return res;
}

/*
static int
fuseram_opendir(const char *path, struct fuse_file_info *fp)
{
	//UNIMPLEMENTED
	return 0;

}
*/

static int
fuseram_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	struct hash_table_iter *iter;
	ramdir_t *dp;
	ramdir_t *child;
	char *fname;

	dp = ramfs_lookup(ramfs->root, path);
	if (!dp || dp->type != TYPE_DIR)
		return -ENOENT;

	filler(buf, ".", &dp->attr, 0);
	filler(buf, "..", &dp->parent->attr, 0);

	iter = hash_table_iterate_init(dp->kids);
	while (hash_table_iterate(iter, (void **)&fname, (void **)&child)) {
		filler(buf, fname, &child->attr, 0);
	}
	hash_table_iterate_deinit(&iter);

	return 0;
}

static int
fuseram_mkdir(const char *path, mode_t mode)
{
	ramdir_t *dp;

	dp = ramfs_lookup(ramfs->root, path);
	if (dp) {
		if (dp->type == TYPE_DIR)
			return -EEXIST;
		return -ENOTDIR;
	}
	dp = ramfs_dir_new(ramfs->root, path);
	if (!dp)
		return -EFAULT;

	return 0;
}

static int
fuseram_rmdir(const char *path)
{
	int res;
	ramdir_t *dp;

	dp = ramfs_lookup(ramfs->root, path);
	if (!dp)
		return -ENOENT;
	if (dp->type != TYPE_DIR)
		return -EINVAL;

	res = ramfs_dir_rm(dp);
	if (res != 0)
		return - res;

	return 0;
}

//WARN: Assume that  *to* param does not exist
static int
fuseram_rename(const char *from, const char *to)
{
	int res;

	res = ramfs_node_move(ramfs->root, from, to);
	if (res != 0)
		return -res;

	return 0;
}

static int
fuseram_open(const char *path, struct fuse_file_info *fi)
{
	ramfile_t *fp;

	fp = (ramfile_t *)ramfs_lookup(ramfs->root, path);
	if (!fp || fp->type != TYPE_FILE)
		return -ENOENT;

	fi->fh = (unsigned long)ramfs_file_open(fp, fi->flags);

	return 0;
}

static int
fuseram_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	ramfile_t *fp;

	fp = (ramfile_t *)ramfs_lookup(ramfs->root, path);
	if (fp)
		return -EEXIST;

	fp = ramfs_file_new(ramfs->root, path);
	if (!fp)
		return -ENOTDIR;

	fi->fh = (unsigned long)ramfs_file_open(fp, mode);

	return 0;
}

static int
fuseram_close(const char *path, struct fuse_file_info *fi)
{
	ramfile_t *fp;

	assert(fi && fi->fh);

	fp = (ramfile_t *)fi->fh;

	assert(fp);

	return ramfs_file_close(fp);
}

static int
fuseram_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	int n;
	ramfile_t *fp;

	assert(fi && fi->fh);

	fp = (ramfile_t *)fi->fh;

	assert(fp->type == TYPE_FILE);

	n = ramfs_file_read(fp, buf, size, offset);

	return n;
}

static int
fuseram_write(const char *path, const char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	int n;
	ramfile_t *fp;

	assert(fi && fi->fh);

	fp = (ramfile_t *)fi->fh;

	assert(fp->type == TYPE_FILE);

	n = ramfs_file_write(fp, buf, size, offset);

	return n;
}

static int
fuseram_truncate(const char *path, off_t size)
{
	ramfile_t *fp;
	int res;

	fp = (ramfile_t *)ramfs_lookup(ramfs->root, path);
	if (!fp)
		return -ENOENT;

	res = ramfs_file_truncate(fp, size);
	if (res != 0)
		return -res;

	return 0;
}

static int
fuseram_unlink(const char *path)
{
	int res;
	ramfile_t *fp;

	fp = (ramfile_t *)ramfs_lookup(ramfs->root, path);
	if (!fp || fp->type != TYPE_FILE)
		return -ENOENT;

	res = ramfs_file_unlink(fp);
	if (res != 0)
		return -res;

	return 0;
}

/* xattr operations are optional and can safely be left unimplemented */
/*
static int fuseram_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	return 0;
}

static int fuseram_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	return 0;
}

static int fuseram_listxattr(const char *path, char *list, size_t size)
{
	return 0;
}

static int fuseram_removexattr(const char *path, const char *name)
{
	return 0;
}
*/

static struct fuse_operations fuseram_oper = {
	.getattr	= fuseram_getattr,

	//.getxattr	= fuseram_getxattr,
	//.setxattr	= fuseram_setxattr,
	//.listxattr	= fuseram_listxattr,
	//.removexattr	= fuseram_removexattr,

	//.opendir	= fuseram_opendir,
	.readdir	= fuseram_readdir,
	.mkdir		= fuseram_mkdir,
	.rmdir		= fuseram_rmdir,
	.rename		= fuseram_rename,

	.open		= fuseram_open,
	.create		= fuseram_create,
	.release	= fuseram_close,
	.read		= fuseram_read,
	.write		= fuseram_write,
	.truncate	= fuseram_truncate,
	.unlink		= fuseram_unlink,
};

int
main(int argc, char *argv[])
{
	char teststring[] = "Hello, world\n";
	ramfile_t *fp;

	ramfs = xmalloc(sizeof(*ramfs));
	ramfs_init(ramfs, my_realloc);

	ramfs_dir_new(ramfs->root, "/tmp");
	fp = ramfs_file_new(ramfs->root, "/tmp/hello.txt");
	ramfs_file_write(fp, teststring, strlen(teststring), 0);
	ramfs_file_close(fp);

	return fuse_main(argc, argv, &fuseram_oper, NULL);
}

