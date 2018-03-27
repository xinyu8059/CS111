/* Name: Natasha Sarkar
 * Email: nat41575@gmail.com
 * ID: 904743795
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <getopt.h>

#include "ext2_fs.h"

#define SUPER_OFFSET 1024

int disk_fd;

struct ext2_super_block superblock;
unsigned int block_size;

/* returns the offset for a block number */
unsigned long block_offset(unsigned int block) {
	return SUPER_OFFSET + (block - 1) * block_size;
}

/* function to read in the superblock & print its csv summary*/
void read_superblock() {
	pread(disk_fd, &superblock, sizeof(superblock), SUPER_OFFSET);

	fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", 
		superblock.s_blocks_count, //total number of blocks
		superblock.s_inodes_count, //total number of inodes
		block_size,	//block size
		superblock.s_inode_size, //i-node size
		superblock.s_blocks_per_group, //blocks per group
		superblock.s_inodes_per_group,//inodes per group
		superblock.s_first_ino //first non-reserved inode 
	);
}

/* for each free block, print the number of the free block */
void free_blocks(int group, unsigned int block) {
	//1 means 'used', 0 means 'free'
	char* bytes = (char*) malloc(block_size);
	unsigned long offset = block_offset(block);
	unsigned int curr = superblock.s_first_data_block + group * superblock.s_blocks_per_group;
	pread(disk_fd, bytes, block_size, offset);

	unsigned int i, j;
	for (i = 0; i < block_size; i++) {
		char x = bytes[i];
		for (j = 0; j < 8; j++) {
			int used = 1 & x;
			if (!used) {
				fprintf(stdout, "BFREE,%d\n", curr);
			}
			x >>= 1;
			curr++;
		}
	}
	free(bytes);
}

/* store the time in the format mm/dd/yy hh:mm:ss, GMT
 * 'raw_time' is a 32 bit value representing the number of
 * seconds since January 1st, 1970
 */
void get_time(time_t raw_time, char* buf) {
	time_t epoch = raw_time;
	struct tm ts = *gmtime(&epoch);
	strftime(buf, 80, "%m/%d/%y %H:%M:%S", &ts);
}

/* given location of directory entry block, produce directory entry summary */
void read_dir_entry(unsigned int parent_inode, unsigned int block_num) {
	struct ext2_dir_entry dir_entry;
	unsigned long offset = block_offset(block_num);
	unsigned int num_bytes = 0;

	while(num_bytes < block_size) {
		memset(dir_entry.name, 0, 256);
		pread(disk_fd, &dir_entry, sizeof(dir_entry), offset + num_bytes);
		if (dir_entry.inode != 0) { //entry is not empty
			memset(&dir_entry.name[dir_entry.name_len], 0, 256 - dir_entry.name_len);
			fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
				parent_inode, //parent inode number
				num_bytes, //logical byte offset
				dir_entry.inode, //inode number of the referenced file
				dir_entry.rec_len, //entry length
				dir_entry.name_len, //name length
				dir_entry.name //name, string, surrounded by single-quotes
			);
		}
		num_bytes += dir_entry.rec_len;
	}
}

/* for an allocated inode, print its summary */
void read_inode(unsigned int inode_table_id, unsigned int index, unsigned int inode_num) {
	struct ext2_inode inode;

	unsigned long offset = block_offset(inode_table_id) + index * sizeof(inode);
	pread(disk_fd, &inode, sizeof(inode), offset);

	if (inode.i_mode == 0 || inode.i_links_count == 0) {
		return;
	}

	char filetype = '?';
	//get bits that determine the file type
	uint16_t file_val = (inode.i_mode >> 12) << 12;
	if (file_val == 0xa000) { //symbolic link
		filetype = 's';
	} else if (file_val == 0x8000) { //regular file
		filetype = 'f';
	} else if (file_val == 0x4000) { //directory
		filetype = 'd';
	}

	unsigned int num_blocks = 2 * (inode.i_blocks / (2 << superblock.s_log_block_size));

	fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,",
		inode_num, //inode number
		filetype, //filetype
		inode.i_mode & 0xFFF, //mode, low order 12-bits
		inode.i_uid, //owner
		inode.i_gid, //group
		inode.i_links_count //link count
	);

	char ctime[20], mtime[20], atime[20];
    	get_time(inode.i_ctime, ctime); //creation time
    	get_time(inode.i_mtime, mtime); //modification time
    	get_time(inode.i_atime, atime); //access time
    	fprintf(stdout, "%s,%s,%s,", ctime, mtime, atime);
		
	fprintf(stdout, "%d,%d", 
	    	inode.i_size, //file size
		num_blocks //number of blocks
	);

	unsigned int i;
	for (i = 0; i < 15; i++) { //block addresses
		fprintf(stdout, ",%d", inode.i_block[i]);
	}
	fprintf(stdout, "\n");

	//if the filetype is a directory, need to create a directory entry
	for (i = 0; i < 12; i++) { //direct entries
		if (inode.i_block[i] != 0 && filetype == 'd') {
			read_dir_entry(inode_num, inode.i_block[i]);
		}
	}

	//indirect entry
	if (inode.i_block[12] != 0) {
		uint32_t *block_ptrs = malloc(block_size);
		uint32_t num_ptrs = block_size / sizeof(uint32_t);

		unsigned long indir_offset = block_offset(inode.i_block[12]);
		pread(disk_fd, block_ptrs, block_size, indir_offset);

		unsigned int j;
		for (j = 0; j < num_ptrs; j++) {
			if (block_ptrs[j] != 0) {
				if (filetype == 'd') {
					read_dir_entry(inode_num, block_ptrs[j]);
				}
				fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
					inode_num, //inode number
					1, //level of indirection
					12 + j, //logical block offset
					inode.i_block[12], //block number of indirect block being scanned
					block_ptrs[j] //block number of reference block
				);
			}
		}
		free(block_ptrs);
	}

	//doubly indirect entry
	if (inode.i_block[13] != 0) {
		uint32_t *indir_block_ptrs = malloc(block_size);
		uint32_t num_ptrs = block_size / sizeof(uint32_t);

		unsigned long indir2_offset = block_offset(inode.i_block[13]);
		pread(disk_fd, indir_block_ptrs, block_size, indir2_offset);

		unsigned int j;
		for (j = 0; j < num_ptrs; j++) {
			if (indir_block_ptrs[j] != 0) {
				fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
					inode_num, //inode number
					2, //level of indirection
					256 + 12 + j, //logical block offset
					inode.i_block[13], //block number of indirect block being scanned
					indir_block_ptrs[j] //block number of reference block
				);

				//search through this indirect block to find its directory entries
				uint32_t *block_ptrs = malloc(block_size);
				unsigned long indir_offset = block_offset(indir_block_ptrs[j]);
				pread(disk_fd, block_ptrs, block_size, indir_offset);

				unsigned int k;
				for (k = 0; k < num_ptrs; k++) {
					if (block_ptrs[k] != 0) {
						if (filetype == 'd') {
							read_dir_entry(inode_num, block_ptrs[k]);
						}
						fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
							inode_num, //inode number
							1, //level of indirection
							256 + 12 + k, //logical block offset
					 		indir_block_ptrs[j], //block number of indirect block being scanned
							block_ptrs[k] //block number of reference block
						);
					}
				}
				free(block_ptrs);
			}
		}
		free(indir_block_ptrs);
	}

	//triply indirect entry
	if (inode.i_block[14] != 0) {
		uint32_t *indir2_block_ptrs = malloc(block_size);
		uint32_t num_ptrs = block_size / sizeof(uint32_t);

		unsigned long indir3_offset = block_offset(inode.i_block[14]);
		pread(disk_fd, indir2_block_ptrs, block_size, indir3_offset);

		unsigned int j;
		for (j = 0; j < num_ptrs; j++) {
			if (indir2_block_ptrs[j] != 0) {
				fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
					inode_num, //inode number
					3, //level of indirection
					65536 + 256 + 12 + j, //logical block offset
					inode.i_block[14], //block number of indirect block being scanned
					indir2_block_ptrs[j] //block number of reference block
				);

				uint32_t *indir_block_ptrs = malloc(block_size);
				unsigned long indir2_offset = block_offset(indir2_block_ptrs[j]);
				pread(disk_fd, indir_block_ptrs, block_size, indir2_offset);

				unsigned int k;
				for (k = 0; k < num_ptrs; k++) {
					if (indir_block_ptrs[k] != 0) {
						fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
							inode_num, //inode number
							2, //level of indirection
							65536 + 256 + 12 + k, //logical block offset
				 			indir2_block_ptrs[j], //block number of indirect block being scanned
							indir_block_ptrs[k] //block number of reference block	
						);	
						uint32_t *block_ptrs = malloc(block_size);
						unsigned long indir_offset = block_offset(indir_block_ptrs[k]);
						pread(disk_fd, block_ptrs, block_size, indir_offset);

						unsigned int l;
						for (l = 0; l < num_ptrs; l++) {
							if (block_ptrs[l] != 0) {
								if (filetype == 'd') {
									read_dir_entry(inode_num, block_ptrs[l]);
								}
								fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
									inode_num, //inode number
									1, //level of indirection
									65536 + 256 + 12 + l, //logical block offset
				 					indir_block_ptrs[k], //block number of indirect block being scanned
									block_ptrs[l] //block number of reference block	
								);
							}
						}
						free(block_ptrs);
					}
				}
				free(indir_block_ptrs);
			}
		}
		free(indir2_block_ptrs);
	}
}

/* for each inode, print if free; if not, print inode summary */
void read_inode_bitmap(int group, int block, int inode_table_id) {
	int num_bytes = superblock.s_inodes_per_group / 8;
	char* bytes = (char*) malloc(num_bytes);

	unsigned long offset = block_offset(block);
	unsigned int curr = group * superblock.s_inodes_per_group + 1;
    	unsigned int start = curr;
    	pread(disk_fd, bytes, num_bytes, offset);

    	int i, j;
    	for (i = 0; i < num_bytes; i++) {
    		char x = bytes[i];
    		for (j = 0; j < 8; j++) {
    			int used = 1 & x;
    			if (used) { //inode is allocated
    				read_inode(inode_table_id, curr - start, curr);
    			} else { //free inode
    				fprintf(stdout, "IFREE,%d\n", curr);
    			}
    			x >>= 1;
    			curr++;
    		}
    	}
    	free(bytes);
}

/* for each group, read bitmaps for blocks and inodes */
void read_group(int group, int total_groups) {
	struct ext2_group_desc group_desc;
	uint32_t descblock = 0;
	if (block_size == 1024) {
		descblock = 2;
	} else {
		descblock = 1;
	}

	unsigned long offset = block_size * descblock + 32 * group;
	pread(disk_fd, &group_desc, sizeof(group_desc), offset);

	unsigned int num_blocks_in_group = superblock.s_blocks_per_group;
	if (group == total_groups - 1) {
		num_blocks_in_group = superblock.s_blocks_count - (superblock.s_blocks_per_group * (total_groups - 1));
	}

	unsigned int num_inodes_in_group = superblock.s_inodes_per_group;
	if (group == total_groups - 1) {
		num_inodes_in_group = superblock.s_inodes_count - (superblock.s_inodes_per_group * (total_groups - 1));
	}

	fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
		group, //group number
		num_blocks_in_group, //total number of blocks in this group
		num_inodes_in_group, //total number of inodes
		group_desc.bg_free_blocks_count, //number of free blocks 
		group_desc.bg_free_inodes_count, //number of free inodes
		group_desc.bg_block_bitmap, //block number of free block bitmap for this group
		group_desc.bg_inode_bitmap, //block number of free inode bitmap for this group
		group_desc.bg_inode_table //block number of first block of i-nodes in this group
	);

	unsigned int block_bitmap = group_desc.bg_block_bitmap;
	free_blocks(group, block_bitmap);

	unsigned int inode_bitmap = group_desc.bg_inode_bitmap;
	unsigned int inode_table = group_desc.bg_inode_table;
	read_inode_bitmap(group, inode_bitmap, inode_table);
}

/* read information about all the groups */
int main (int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Bad arguments\n");
		exit(1);
	}

    	struct option options[] = {
		{0, 0, 0, 0}
	};
	
	if (getopt_long(argc, argv, "", options, NULL) != -1) {
		fprintf(stderr, "Bad arguments\n");
		exit(1);
	}

	if ((disk_fd = open(argv[1], O_RDONLY)) == -1) {
		fprintf(stderr, "Could not open file\n");
		exit(1);
	}
	block_size = EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size;
	read_superblock();
	int num_groups = superblock.s_blocks_count / superblock.s_blocks_per_group;
	if ((double) num_groups < (double) superblock.s_blocks_count / superblock.s_blocks_per_group) {
		num_groups++;
	}
	//fprintf(stderr, "%d", num_groups);
	int i; 
	for (i = 0; i < num_groups; i++) {
		read_group(i, num_groups);
	}

	return 0;
}
