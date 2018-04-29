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
}

int
main(int argc, const char *argv[])
{
	superblock_t sb;
	myfile_t *fp;
	mydir_t *dp;

	ramfs_init(&sb, malloc);
	ramfs_dir_new(&sb, "/tmp/");
	ramfs_file_new(&sb, "/tmp/hello");
	ramfs_file_new(&sb, "/../../../../../q.txt");

	ramfs_lookup(sb.root, "/tmp/dir/subdir/hello");

	open_test(&sb);

	return 0;
}
