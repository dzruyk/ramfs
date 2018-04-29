#ifndef _RAMFS__H_
#define _RAMFS__H_

#include "vector/vector.h"

typedef enum {
	TYPE_NO = 0,
	TYPE_FILE,
	TYPE_DIR,
} ramfs_type;

typedef void *(*allocator_t)(void *oldptr, size_t sz);

struct mydir;
typedef struct mydir ramdir_t;
typedef ramdir_t mynode_t;

typedef struct {
	ramdir_t *root;
	allocator_t alloc;
} superblock_t;

struct mydir {
	/* common fields */
	char *fname;
	ramdir_t *parent;
	superblock_t *sb;
	struct stat attr;

	/* special fields */
	vector child_dirs;
	vector child_files;
};

typedef struct {
	/* common fields */
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

ramfile_t *ramfs_file_new(ramdir_t *curdir, char *fpath);
ramdir_t *ramfs_dir_new(ramdir_t *curdir, char *fpath);

ramfile_t *ramfs_lookup_file(ramdir_t *curdir, char *fpath);
ramdir_t *ramfs_lookup_dir(ramdir_t *curdir, char *fpath);

mynode_t *ramfs_lookup_dirname(ramdir_t *curdir, char *fpath);

/* File operations */
ramfile_t *ramfs_file_open(ramdir_t *curdir, char *filepath, int flags);
int ramfs_file_read(ramfile_t *fp, const char *buf, int sz, int off);
int ramfs_file_write(ramfile_t *fp, char *buf, int sz, int off);
int ramfs_file_close(ramfile_t *dst);
int ramfs_file_move(ramdir_t *dstdir, ramfile_t *file);
int ramfs_file_rm(ramdir_t *curdir, char *filepath);

/* Dir operations */
ramdir_t *ramfs_mkdir(ramdir_t *curdir, char *filepath);

/* Internal stuff */
ramfs_type ramfs_obj_type(ramdir_t *curdir, char *fpath);
int ramfs_dir_add_dir(ramdir_t *parent, ramdir_t *child);
int ramfs_dir_add_file(ramdir_t *parent, ramfile_t *child);

/* Debugging stuff */
void ramfs_debug_ls(ramdir_t *sb);

#endif
