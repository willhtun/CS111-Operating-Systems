#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <math.h>
#include "ext2_fs.h"

static int imageFileDes = -1;
static int blockSize = 0;
static int groupSize = 0;
int* inodeBitmap = 0;

struct ext2_super_block superBlock;
struct ext2_group_desc* groupDesc;

#define OFFSET	1024
#define OFFSET_BLOCK(block) (((block-1) * blockSize) + OFFSET)

void getTime(int timeIn, char* buffer) {
	time_t local = timeIn;
	struct tm t = *gmtime(&local);
	strftime(buffer, 100, "%m/%d/%y %H:%M:%S", &t);
}

void indirect_func_levelOne(int input_inodeID, int blockID, int blockOffset) {
	int i = 0;
	int numEntries = blockSize / sizeof(int);
	int directoryEntries[numEntries];
	memset(directoryEntries, 0, sizeof(directoryEntries));
	int temp_readDE = pread(imageFileDes, directoryEntries, blockSize, OFFSET_BLOCK(blockID));
	if (temp_readDE < 0) {
		printf("%s", "Error reading directoryEntries in block.\n");
		if (groupDesc != NULL)
			free(groupDesc);
		if (inodeBitmap != NULL)
			free(inodeBitmap);
		close(imageFileDes);
		exit(2);
	}
	for (i = 0; i < numEntries; i++) {
		if (directoryEntries[i] != 0) {
			printf("INDIRECT,%u,%u,%u,%u,%u\n", input_inodeID, 1, blockOffset, blockID, directoryEntries[i]);
		}
		blockOffset++;
	}
}

void indirect_func_levelTwo(int input_inodeID, int blockID, int blockOffset) {
	int i = 0;
	int numEntries = blockSize / sizeof(int);
	int directoryEntries[numEntries];
	memset(directoryEntries, 0, sizeof(directoryEntries));
	int temp_readDE = pread(imageFileDes, directoryEntries, blockSize, OFFSET_BLOCK(blockID));
	if (temp_readDE < 0) {
		printf("%s", "Error reading directoryEntries in block.\n");
		if (groupDesc != NULL)
			free(groupDesc);
		if (inodeBitmap != NULL)
			free(inodeBitmap);
		close(imageFileDes);
		exit(2);
	}
	for (i = 0; i < numEntries; i++) {
		if (directoryEntries[i] != 0) {
			printf("INDIRECT,%u,%u,%u,%u,%u\n", input_inodeID, 2, blockOffset, blockID, directoryEntries[i]);
			indirect_func_levelOne(input_inodeID, directoryEntries[i], blockOffset);
		}
		blockOffset += 268;
	}
}

void indirect_func_levelThree(int input_inodeID, int blockID, int blockOffset) {
	int i = 0;
	int numEntries = blockSize / sizeof(int);
	int directoryEntries[numEntries];
	memset(directoryEntries, 0, sizeof(directoryEntries));
	int temp_readDE = pread(imageFileDes, directoryEntries, blockSize, OFFSET_BLOCK(blockID));
	if (temp_readDE < 0) {
		printf("%s", "Error reading directoryEntries in block.\n");
		if (groupDesc != NULL)
			free(groupDesc);
		if (inodeBitmap != NULL)
			free(inodeBitmap);
		close(imageFileDes);
		exit(2);
	}
	for (i = 0; i < numEntries; i++) {
		if (directoryEntries[i] != 0) {
			printf("INDIRECT,%u,%u,%u,%u,%u\n", input_inodeID, 3, blockOffset, blockID, directoryEntries[i]);
			indirect_func_levelTwo(input_inodeID, directoryEntries[i], blockOffset);
		}
		blockOffset += 65804;
	}
}

void indirect_func_directory(struct ext2_inode* inode, int inode_no, int blockID, int size, int level) {
	int i = 0;
	int numEntries = blockSize / sizeof(int);
	int directoryEntries[numEntries];
	memset(directoryEntries, 0, sizeof(directoryEntries));
	int temp_readDE = pread(imageFileDes, directoryEntries, blockSize, OFFSET_BLOCK(blockID));
	if (temp_readDE < 0) {
		printf("%s", "Error reading directoryEntries in block.\n");
		if (groupDesc != NULL)
			free(groupDesc);
		if (inodeBitmap != NULL)
			free(inodeBitmap);
		close(imageFileDes);
		exit(2);
	}

	char block[blockSize];
	struct ext2_dir_entry *directoryEntry;

	for (i = 0; i < numEntries; i++) {
		if (directoryEntries[i] != 0) {
			if (level == 2 || level == 3) {
				indirect_func_directory(inode, inode_no, directoryEntries[i], size, level - 1);
			}
			int temp_blockRead = pread(imageFileDes, block, blockSize, OFFSET_BLOCK(directoryEntries[i]));
			if (temp_blockRead < 0) {
				printf("%s", "Error reading blocks.\n");
				if (groupDesc != NULL)
					free(groupDesc);
				if (inodeBitmap != NULL)
					free(inodeBitmap);
				close(imageFileDes);
				exit(2);
			}
			directoryEntry = (struct ext2_dir_entry *) block;

			while ((size < (int)inode->i_size) && directoryEntry->file_type) {
				char file_name[EXT2_NAME_LEN + 1];
				memcpy(file_name, directoryEntry->name, directoryEntry->name_len);
				file_name[directoryEntry->name_len] = 0;
				if (directoryEntry->inode > 0) {
					printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_no, size, directoryEntry->inode, directoryEntry->rec_len, directoryEntry->name_len, file_name);
				}
				size = size + directoryEntry->rec_len;
				directoryEntry = (void*)directoryEntry + directoryEntry->rec_len;
			}
		}
	}
}

void getAllDirectory_function(struct ext2_inode* inode, int inode_no) {
	char block[blockSize];
	struct ext2_dir_entry *directoryEntry;
	int size = 0;
	int i;
	for (i = 0; i < EXT2_NDIR_BLOCKS; i++) {
		if (pread(imageFileDes, block, blockSize, OFFSET_BLOCK(inode->i_block[i])) < 0) {
			printf("%s", "Can't read block.\n");
			if (groupDesc != NULL)
				free(groupDesc);
			if (inodeBitmap != NULL)
				free(inodeBitmap);
			close(imageFileDes);
			exit(2);
		}
		directoryEntry = (struct ext2_dir_entry *) block;

		while ((size < (int)inode->i_size) && directoryEntry->file_type) {
			char file_name[EXT2_NAME_LEN + 1];
			memcpy(file_name, directoryEntry->name, directoryEntry->name_len);
			file_name[directoryEntry->name_len] = 0;
			if (directoryEntry->inode != 0) {
				printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_no, size, directoryEntry->inode, directoryEntry->rec_len, directoryEntry->name_len, file_name);
			}
			size += directoryEntry->rec_len;
			directoryEntry = (void*)directoryEntry + directoryEntry->rec_len;
		}
	}

	if (inode->i_block[EXT2_IND_BLOCK] != 0) {
		indirect_func_directory(inode, inode_no, inode->i_block[EXT2_IND_BLOCK], size, 1);
	}
	if (inode->i_block[EXT2_DIND_BLOCK] != 0) {
		indirect_func_directory(inode, inode_no, inode->i_block[EXT2_DIND_BLOCK], size, 2);
	}
	if (inode->i_block[EXT2_TIND_BLOCK] != 0) {
		indirect_func_directory(inode, inode_no, inode->i_block[EXT2_TIND_BLOCK], size, 3);
	}
}

int main(int argc, char** argv) {

	// Local Variables =========================================================================================
	long i = 0, j = 0, k = 0;

	// Arguments ===============================================================================================
	if (argc != 2) {
		fprintf(stderr, "Invalid arguments. Pass in an image file with executable.\n");
		exit(1);
	}

	char* imageName = malloc((strlen(argv[1]) + 1) * sizeof(char));
	imageName = argv[1];
	imageFileDes = open(imageName, O_RDONLY);
	if (imageFileDes < 0) {
		fprintf(stderr, "Cannot open image file.\n");
		exit(1);
	}

	// Superblock ==============================================================================================
	int temp_sbRead = pread(imageFileDes, &superBlock, sizeof(struct ext2_super_block), 1024);
	if (temp_sbRead < 0) {
		printf("%s", "Can't read superblock.\n");
		if (groupDesc != NULL)
			free(groupDesc);
		if (inodeBitmap != NULL)
			free(inodeBitmap);
		close(imageFileDes);
		exit(2);
	}

	blockSize = EXT2_MIN_BLOCK_SIZE << superBlock.s_log_block_size,

	printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", superBlock.s_blocks_count, superBlock.s_inodes_count, blockSize, superBlock.s_inode_size, superBlock.s_blocks_per_group, superBlock.s_inodes_per_group, superBlock.s_first_ino);

	// Groups ==================================================================================================
	int descSize;
	int blocksLeft = superBlock.s_blocks_count;
	int blocksLeft_g = superBlock.s_blocks_per_group;
	int inodesLeft = superBlock.s_inodes_count;
	int inodesLeft_g = superBlock.s_inodes_per_group;

	groupSize = ((superBlock.s_blocks_count - 1) / superBlock.s_blocks_per_group) + 1;
	descSize = groupSize * sizeof(struct ext2_group_desc);
	groupDesc = malloc(descSize);

	int temp_groupRead = pread(imageFileDes, groupDesc, descSize, OFFSET + blockSize);
	if (temp_groupRead < 0) {
		printf("%s", "Cant read group descriptor from image file.\n");
		exit(2);
	}

	for (i = 0; i < groupSize; i++) {
		if ((int)superBlock.s_blocks_per_group > blocksLeft) {
			blocksLeft_g = blocksLeft;
		}

		if ((int)superBlock.s_inodes_per_group > inodesLeft) {
			inodesLeft_g = inodesLeft;
		}

		blocksLeft = blocksLeft - superBlock.s_blocks_per_group;
		inodesLeft = inodesLeft - superBlock.s_inodes_per_group;

		printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", (int)i, blocksLeft_g, inodesLeft_g, groupDesc[i].bg_free_blocks_count, groupDesc[i].bg_free_inodes_count, groupDesc[i].bg_block_bitmap, groupDesc[i].bg_inode_bitmap, groupDesc[i].bg_inode_table);
	}

	// Block Bitmap ============================================================================================
	int byte;
	for (i = 0; i < groupSize; i++) {
		for (j = 0; j < blockSize; j++) {
			int byteMask = 1;
			int temp_byteRead = pread(imageFileDes, &byte, 1, (blockSize * groupDesc[i].bg_block_bitmap) + j);
			if (temp_byteRead < 0) {
				printf("%s", "Can't read image file descriptor.\n");
				if (groupDesc != NULL)
					free(groupDesc);
				if (inodeBitmap != NULL)
					free(inodeBitmap);
				close(imageFileDes);
				exit(2);
			}
			for (k = 0; k < 8; k++) {
				if ((byte & byteMask) == 0) {
					fprintf(stdout, "BFREE,%lu\n", i * superBlock.s_blocks_per_group + j * 8 + k + 1);
				}
				byteMask <<= 1;
			}
		}
	}

	// Inode Bitmap ============================================================================================
	inodeBitmap = malloc(sizeof(int) * groupSize * blockSize);
	for (i = 0; i < groupSize; i++) {
		for (j = 0; j < blockSize; j++) {
			int byte;
			int temp_byteRead;
			temp_byteRead = pread(imageFileDes, &byte, 1, (blockSize * groupDesc[i].bg_inode_bitmap) + j);
			if (temp_byteRead < 0) {
				printf("%s", "Can't read image file descriptor.\n");
				if (groupDesc != NULL)
					free(groupDesc);
				if (inodeBitmap != NULL)
					free(inodeBitmap);
				close(imageFileDes);
				exit(2);
			}
			inodeBitmap[i + j] = byte;
			int byteMask = 1;
			for (k = 0; k < 8; k++) {
				if ((byte & byteMask) == 0) {
					fprintf(stdout, "IFREE,%lu\n", i * superBlock.s_inodes_per_group + j * 8 + k + 1);
				}
				byteMask <<= 1;
			}
		}
	}

	// Scanning Inodes =========================================================================================
	int inodeID;
	int inode_valid, inode_found;
	struct ext2_inode inode_s;
	char i_ctime[100], i_mtime[100], i_atime[100];
	int byteMask;
	long curr_inodeID;

	for (i = 0; i < groupSize; i++) {
		for (inodeID = 2; inodeID < (int)superBlock.s_inodes_count; inodeID++) {
			inode_valid = 1;
			inode_found = 0;
			for (j = 0; j < blockSize; j++) {
				int byte = inodeBitmap[i + j];
				byteMask = 1;
				for (k = 0; k < 8; k++) {
					curr_inodeID = i * superBlock.s_inodes_per_group + j * 8 + k + 1;
					if ((int)curr_inodeID == inodeID) {
						inode_found = 1;
						if ((byte & byteMask) == 0)
							inode_valid = 0;
						break;
					}
					byteMask <<= 1;
					if (inode_found)
						break;
				}
			}

			if (!inode_found) {
				printf("%s", "Error in bitmap.\n");
				if (groupDesc != NULL)
					free(groupDesc);
				if (inodeBitmap != NULL)
					free(inodeBitmap);
				close(imageFileDes);
				exit(2);
			}

			if (!inode_valid)
				continue;

			off_t offset = OFFSET_BLOCK(groupDesc[i].bg_inode_table) + (inodeID - 1) * sizeof(struct ext2_inode);

			int temp_inodeRead = pread(imageFileDes, &inode_s, sizeof(struct ext2_inode), offset);
			if (temp_inodeRead < 0) {
				printf("%s", "Error in reading inode.\n");
				if (groupDesc != NULL)
					free(groupDesc);
				if (inodeBitmap != NULL)
					free(inodeBitmap);
				close(imageFileDes);
				exit(2);
			}

			getTime(inode_s.i_ctime, i_ctime);
			getTime(inode_s.i_mtime, i_mtime);
			getTime(inode_s.i_atime, i_atime);

			char fileType;
			if (S_ISDIR(inode_s.i_mode)) {
				fileType = 'd';
			}
			else if (S_ISREG(inode_s.i_mode)) {
				fileType = 'f';
			}
			else if (S_ISLNK(inode_s.i_mode)) {
				fileType = 's';
			}
			else {
				fileType = '?';
			}

			printf("INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u", inodeID, fileType, inode_s.i_mode & 0x0FFF, inode_s.i_uid, inode_s.i_gid, inode_s.i_links_count, i_ctime, i_mtime, i_atime, inode_s.i_size, inode_s.i_blocks);

			for (k = 0; k < EXT2_N_BLOCKS; k++) {
				printf(",%u", inode_s.i_block[k]);
			}
			printf("\n");

			if (fileType == 'd') {
				if (S_ISDIR(inode_s.i_mode) == 0) {
					printf("%s", "Error with file type.\n");
					exit(2);
				}
				getAllDirectory_function(&inode_s, inodeID);
			}

			if (inode_s.i_block[EXT2_IND_BLOCK] > 0) {
				indirect_func_levelOne(inodeID, inode_s.i_block[EXT2_IND_BLOCK], 12);
			}
			if (inode_s.i_block[EXT2_DIND_BLOCK] > 0) {
				indirect_func_levelTwo(inodeID, inode_s.i_block[EXT2_DIND_BLOCK], 268);
			}
			if (inode_s.i_block[EXT2_TIND_BLOCK] > 0) {
				indirect_func_levelThree(inodeID, inode_s.i_block[EXT2_TIND_BLOCK], 65804);
			}
			if (inodeID == 2) {
				inodeID = superBlock.s_first_ino - 1;
			}
		}
	}

	// Cleaning Up =============================================================================================

	if (groupDesc != NULL)
		free(groupDesc);
	if (inodeBitmap != NULL)
		free(inodeBitmap);
	close(imageFileDes);

	exit(0);
}