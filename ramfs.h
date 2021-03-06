#ifndef _RAMFS__H_
#define _RAMFS__H_

#include "hash/hash.h"

#define RAMNODE(ptr) ((ramnode_t*) ptr)

typedef enum {
	TYPE_FILE = 0,
	TYPE_DIR,
} ramfs_type;

typedef void *(*allocator_t)(void *oldptr, size_t sz);

struct ramdir;
typedef struct ramdir ramdir_t;
typedef ramdir_t ramnode_t;

typedef struct {
	ramdir_t *root;
	allocator_t alloc;
} superblock_t;

struct ramdir {
	/* common fields */
	ramfs_type type;
	char *fname;
	ramdir_t *parent;
	superblock_t *sb;
	struct stat attr;

	/* special fields */
	struct hash_table *kids;
};

typedef struct {
	/* common fields */
	ramfs_type type;
	char *fname;
	ramdir_t *parent;
	superblock_t *sb;
	struct stat attr;

	/* special fields */
	int datalen;
	uint8_t *data;

	// We can delete file from directory, but still may have
	// some references to it.
	// Delete file only when last descriptor is closed.
	int nrefs;
} ramfile_t;

/* Initialize new fs, create root directory. */
void ramfs_init(superblock_t *sb, allocator_t alloc);
void ramfs_destroy(superblock_t *sb);

ramfile_t *ramfs_file_new(ramdir_t *curdir, const char *fpath);
ramdir_t *ramfs_dir_new(ramdir_t *curdir, const char *fpath);

/* Common operations */
int ramfs_node_move(ramdir_t *curdir, const char *src, const char *dst);
ramnode_t *ramfs_lookup(ramdir_t *curdir, const char *fpath);
ramnode_t *ramfs_lookup_dirname(ramdir_t *curdir, const char *fpath);

/* File operations */
ramfile_t *ramfs_file_open(ramfile_t *fp, int flags);
int ramfs_file_close(ramfile_t *fp);
int ramfs_file_read(ramfile_t *fp, char *buf, int sz, off_t off);
int ramfs_file_write(ramfile_t *fp, const char *buf, int sz, off_t off);
int ramfs_file_truncate(ramfile_t *fp, off_t sz);

int ramfs_file_unlink(ramfile_t *fp);

/* Dir operations */
#define ramfs_mkdir ramfs_dir_new
int ramfs_dir_rm(ramdir_t *dp);

/* Internal stuff */
int ramfs_dir_add(ramdir_t *parent, ramnode_t *child);

/* Debugging stuff */
void ramfs_debug_ls(ramdir_t *sb);

#endif
