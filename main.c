#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ramfs.h"

int
open_test(superblock_t *sb)
{
	ramfile_t *fp;
	ramdir_t *dp;
	char buf[1024];
	int n;

	dp = ramfs_lookup(sb->root, "/tmp/dir");
	if (dp != NULL && dp->type == TYPE_DIR) {
		fp = ramfs_file_new(dp, "file.txt");
		fp = ramfs_file_open(fp, O_RDWR);
		ramfs_file_write(fp, "hello", 5, 0);
		ramfs_file_close(fp);

		n = ramfs_file_read(fp, buf, 1024, 0);
		assert(n == 5);
		assert(strncmp(buf, "hello", 5) == 0);
	}

	return 0;
}

int
main(int argc, const char *argv[])
{
	superblock_t sb;

	ramfs_init(&sb, realloc);
	assert( ramfs_lookup(sb.root, "/"));
	assert( ramfs_dir_new(sb.root, "/tmp"));
	assert(	ramfs_file_new(sb.root, "/tmp/hello"));
	assert(	ramfs_file_new(sb.root, "/../../../../../q.txt"));

	assert( ramfs_lookup(sb.root, "/tmp/dir/subdir/hello") == NULL);

	open_test(&sb);

	ramfs_debug_ls(sb.root);

	return 0;
}
