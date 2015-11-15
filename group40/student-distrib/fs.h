#include "types.h"
#include "lib.h"
#include "exceptions.h"

#define NUM_DATA_BLOCKS 1023
#define BOOT_BLOCK_PADDING 52
#define MAX_NUM_FILES 63
#define MAX_FILE_NAME_LENGTH 32
#define DENTRY_PADDING 24
#define DENTRY_SIZE 64
#define MAX_FILE_DESC 0
#define BYTES_PER_BLOCK 4096



//structures used in the file system
typedef struct inode{
        uint32_t size;				//4B
        uint32_t blocks[NUM_DATA_BLOCKS];	//1023*4B pointers
}inode_t;					//4kb total


typedef struct dentry{
        uint8_t file_name[MAX_FILE_NAME_LENGTH];//32B
        uint32_t file_type;			//4B
        uint32_t inode_number;			//4B
        uint8_t reserved[DENTRY_PADDING];	//24B Padding
}dentry_t;					//64B total


typedef struct boot_block{
        uint32_t total_dirs;                    //4B
        uint32_t total_inodes;                  //4B
        uint32_t total_blocks;                  //4B
        uint8_t reserved[BOOT_BLOCK_PADDING];   //52B filler so 64B total
        dentry_t dir_entries[MAX_NUM_FILES];    //63*64B
}boot_block_t;                                  //4kB total struct


typedef struct data_block{
	uint8_t data[BYTES_PER_BLOCK];
}data_t;

extern boot_block_t* boot_block;

//functions used to modify the file system
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

uint32_t read_dir(uint8_t* buf);

int32_t write_data(uint32_t inode, uint32_t offset, const uint8_t* buf, uint32_t length);

int32_t open_file(const uint8_t* fname);
int32_t close_file(int32_t fd);

int32_t file_name_cmp(const uint8_t* s1, const uint8_t* s2);
