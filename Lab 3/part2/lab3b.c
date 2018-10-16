#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* inputFile_name;
FILE* inputFile;

struct SuperBlock {
	int sb_numBlocks;
	int sb_numInodes;
	int sb_blockSize;
	int sb_inodeSize;
	int sb_blocksPerGroup;
	int sb_inodePerGroup;
	int sb_firstInode;
};

struct Inode {
	int ino_inodeNum;
	char ino_fileType;
	int ino_mode;
	int ino_owner;
	int ino_group;
	int ino_linkCount;
	int ino_ctime;
	int ino_mtime;
	int ino_atime;
	int ino_size;
	int ino_numBlocks;
	int ino_blocks[12];
	int ino_single;
	int ino_double;
	int ino_triple;
};

struct Indirect {
	int id_inodeNum;
	int id_level;
	int id_offset;
	int id_blockNum;
	int id_refBlockNum;
};

struct Dirent {
	int d_parentInodeNum;
	int d_offset;
	int d_inodeNum;
	int d_entryLength;
	int d_nameLength;
	int d_filename;
};

char* getfield(char* line, int num) {
	char* output = malloc(sizeof(char) * 50);
	int index = 1;
	int start = 0, end = 0, i = 0, found = 0;
	start = i;
	while (line[i] != '\0') {
		if (line[i] == ',') {
			if (index == num) {
				found = 1;
				end = i;
				break;
			}
			else {
				start = i + 1;
				index++;
			}
		}
		i++;
	}
	if (line[i] == '\0' && index == num) {
		found = 1;
		end = i - 1;
	}
	if (!found)
		return NULL;
	memcpy(output, &line[start], end - start);
	output[end - start + 1] = '\0';
	return output;
}

void add_SuperBlock(struct SuperBlock* sb, char* line) {
	sb->sb_numBlocks = atoi(getfield(line, 2));	
	sb->sb_numInodes = atoi(getfield(line, 3));
	sb->sb_blockSize = atoi(getfield(line, 4));
	sb->sb_inodeSize = atoi(getfield(line, 5));
	sb->sb_blocksPerGroup = atoi(getfield(line, 6));
	sb->sb_inodePerGroup = atoi(getfield(line, 7));
	sb->sb_firstInode = atoi(getfield(line, 8));
}

void add_Inode(struct Inode* ino, char* line) {
	ino->ino_inodeNum = atoi(getfield(line, 2));
	ino->ino_fileType = (getfield(line, 3))[0];
	ino->ino_mode = atoi(getfield(line, 4));
	ino->ino_owner = atoi(getfield(line, 5));
	ino->ino_group = atoi(getfield(line, 6));
	ino->ino_linkCount = atoi(getfield(line, 7));
	ino->ino_ctime = atoi(getfield(line, 8));
	ino->ino_mtime = atoi(getfield(line, 9));
	ino->ino_atime = atoi(getfield(line, 10));
	ino->ino_size = atoi(getfield(line, 11));
	ino->ino_numBlocks = atoi(getfield(line, 12));
	int i;
	for (i = 0; i < 12; i++)
		ino->ino_blocks[i] = atoi(getfield(line, i + 13));
	ino->ino_single = atoi(getfield(line, 25));
	ino->ino_double = atoi(getfield(line, 26));
	ino->ino_triple = atoi(getfield(line, 27));
}

void add_Indirect(struct Indirect* id, char* line) {
	id->id_inodeNum = atoi(getfield(line, 2));
	id->id_level = atoi(getfield(line, 3));
	id->id_offset = atoi(getfield(line, 4));
	id->id_blockNum = atoi(getfield(line, 5));
	id->id_refBlockNum = atoi(getfield(line, 6));
}

void add_Dirent(struct Dirent* d, char* line) {
	d->d_parentInodeNum = atoi(getfield(line, 2));
	d->d_offset = atoi(getfield(line, 3));
	d->d_inodeNum = atoi(getfield(line, 4));
	d->d_entryLength = atoi(getfield(line, 5));
	d->d_nameLength = atoi(getfield(line, 6));
	d->d_filename = atoi(getfield(line, 7));
}

int main(int argc, char** argv) {

	// VARIABLES
	struct SuperBlock superBlock;
	int* freeBlocks;
	int* freeInodes;
	struct Inode* inode;
	struct Indirect* indirect;
	struct Dirent* dirent;

	// ARGUMENT
	if (argc != 2) {
		printf("%s", "Invalid argument. Pass in a csv file with the executable.\n");
		exit(1);
	}

	// OPENING FILE
	inputFile_name = argv[1];
	inputFile = fopen(inputFile_name, "r");
	if (inputFile == NULL) {
		printf("%s", "Cannot open image file.\n");
		exit(1);
	}

	// COUNT LINES IN FILE
	char line[1024];
	int sb_numlines = 3, bfree_numlines = 3, ifree_numlines = 3, inode_numlines = 3, dirent_numlines = 3, indirect_numlines = 3; // 3 just to be safe
	while (fgets(line, 1024, inputFile))
	{
		char* type = getfield(line, 1);

		if (strcmp(type, "SUPERBLOCK") == 0)
			sb_numlines++;
		else if (strcmp(type, "GROUP") == 0)
			continue;
		else if (strcmp(type, "BFREE") == 0)
			bfree_numlines++;
		else if (strcmp(type, "IFREE") == 0)
			ifree_numlines++;
		else if (strcmp(type, "INODE") == 0)
			inode_numlines++;
		else if (strcmp(type, "DIRENT") == 0)
			dirent_numlines++;
		else if (strcmp(type, "INDIRECT") == 0)
			indirect_numlines++;
		else {
			printf("%s", "Error formatting in file.\n");
			exit(2);
		}
	}

	// ALLOCATE ARRAYS
	freeBlocks = malloc(bfree_numlines * sizeof(int));
	freeInodes = malloc(ifree_numlines * sizeof(int));
	inode = malloc(inode_numlines * sizeof(struct Inode*));
	indirect = malloc(indirect_numlines * sizeof(struct Indirect*));
	dirent = malloc(dirent_numlines * sizeof(struct Dirent*));

	// PARSE THE FILE
	rewind(inputFile);
	int freeBlocks_index = 0;
	int freeInodes_index = 0;
	int inode_index = 0;
	int indirect_index = 0;
	int dirent_index = 0;
	while (fgets(line, 1024, inputFile))
	{
		char* type = getfield(line, 1);

		if (strcmp(type, "SUPERBLOCK") == 0)
			add_SuperBlock(&superBlock, line);
		else if (strcmp(type, "GROUP") == 0)
			continue;
		else if (strcmp(type, "BFREE") == 0) {
			freeBlocks[freeBlocks_index] = (getfield(line, 2))[0];
			freeBlocks_index++;
		}
		else if (strcmp(type, "IFREE") == 0) {
			freeInodes[freeInodes_index] = (getfield(line, 2))[0];
			freeInodes_index++;
		}
		else if (strcmp(type, "INODE") == 0) {
			add_Inode(&inode[inode_index], line);
			inode_index++;
		}
		else if (strcmp(type, "DIRENT") == 0) {
			add_Dirent(&dirent[dirent_index], line);
			dirent_index++;
		}
		else if (strcmp(type, "INDIRECT") == 0) {
			add_Indirect(&indirect[indirect_index], line);
			indirect_index++;
		}
		else {
			printf("%s", "Error formatting in file.\n");
			exit(2);
		}
    }

}