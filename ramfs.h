#ifndef _RAMFS__H_
#define _RAMFS__H_

#include <stdint.h>

#include <sys/stat.h>

#include "vector/vector.h"

typedef enum {
	TYPE_FILE = 0,
	TYPE_DIR,
} ramfs_type;

typedef void *(*allocator_t)(void *oldptr, size_t sz);

struct mydir;
typedef struct mydir mydir_t;
typedef mydir_t mynode_t;

typedef struct {
	mydir_t *root;
	allocator_t alloc;
} superblock_t;

#define FSNODE_COMMON			\
		ramfs_type type;	\
		char *fname;		\
		mydir_t *parent;	\
		superblock_t *sb;	\
					\
		struct stat attr

struct mydir {
	FSNODE_COMMON;

	vector child_dirs;
	vector child_files;
};

typedef struct {
	FSNODE_COMMON;

	int datalen;
	uint8_t *data;

	// We can delete file from directory, but still may have
	// some references to it.
	// Delete file only when last descriptor is closed.
	int nrefs;
} myfile_t;

/* Initialize new fs, create root directory. */
void ramfs_init(superblock_t *sb, allocator_t alloc);

myfile_t *ramfs_file_new(mydir_t *curdir, char *fpath);
mydir_t *ramfs_dir_new(mydir_t *curdir, char *fpath);

/* Return found fs node (directory or file).
 * NOTE: don't forget to checkout node->type */
mynode_t *ramfs_lookup(mydir_t *curdir, char *fpath);
mynode_t *ramfs_lookup_dirname(mydir_t *curdir, char *fpath);

/* File operations */
myfile_t *ramfs_file_open(mydir_t *curdir, char *filepath, int flags);
int ramfs_file_read(myfile_t *fp, const char *buf, int sz, int off);
int ramfs_file_write(myfile_t *fp, char *buf, int sz, int off);
int ramfs_file_close(myfile_t *dst);
int ramfs_file_move(mydir_t *dstdir, myfile_t *file);
int ramfs_file_rm(mydir_t *curdir, char *filepath);

/* Dir operations */
mydir_t *ramfs_mkdir(mydir_t *curdir, char *filepath);
int ramfs_dir_add(mydir_t *parent, mynode_t *child);

/* Debugging stuff */
void ramfs_debug_ls(mydir_t *sb);

#endif
