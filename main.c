#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "ramfs.h"

int
open_test(superblock_t *sb)
{
	myfile_t *fp;
	mydir_t *dp;

	dp = ramfs_lookup(sb->root, "/tmp/dir");
	if (dp != NULL && dp->type == TYPE_DIR) {
		fp = ramfs_file_open(dp, "file.txt", O_RDWR);
	}

	return 0;
}

int
main(int argc, const char *argv[])
{
	superblock_t sb;
	myfile_t *fp;
	mydir_t *dp;

	ramfs_init(&sb, realloc);
	assert( ramfs_dir_new(sb.root, "/tmp/"));
	assert(	ramfs_file_new(sb.root, "/tmp/hello"));
	assert(	ramfs_file_new(sb.root, "/../../../../../q.txt"));

	assert( ramfs_lookup(sb.root, "/tmp/dir/subdir/hello") == NULL);

	open_test(&sb);

	ramfs_debug_ls(sb.root);

	return 0;
}
