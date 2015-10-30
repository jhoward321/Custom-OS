#include "fs.h"


boot_block_t* boot_block;

//search the file system for
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
	if(fname == NULL || dentry == NULL)
		return -1;
	uint32_t i;
	for(i=0; i<MAX_NUM_FILES; i++){
		if (file_name_cmp(fname, boot_block->dir_entries[i].file_name)) {
			memcpy(dentry->file_name, boot_block->dir_entries[i].file_name, MAX_FILE_NAME_LENGTH);
			dentry->file_type = boot_block->dir_entries[i].file_type;
			dentry->inode_number = boot_block->dir_entries[i].inode_number;
			return 0;
		}
	}
	return -1;
}

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	if(index > boot_block->total_dirs || index < 0 || dentry == NULL)
		return -1;
	//fragmentation may be a issue
	memcpy(dentry->file_name, boot_block->dir_entries[index].file_name, MAX_FILE_NAME_LENGTH);
	dentry->file_type = boot_block->dir_entries[index].file_type;
	dentry->inode_number = boot_block->dir_entries[index].inode_number;
	return 0;
}

//copy data from a file from at offset, of length bytes, and store in buf
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	if(inode > boot_block->total_inodes || inode < 0 || buf == NULL)
		return -1;

	uint32_t* inode_addr = ((uint32_t*)((uint32_t)boot_block + BYTES_PER_BLOCK * (inode + 1)));
	uint32_t file_length = *inode_addr;

	uint32_t start_index = offset / BYTES_PER_BLOCK; 		//which data block we're going to start
	uint32_t start_block_offset = offset % BYTES_PER_BLOCK; 			//offset within the current data block

	uint32_t end_index = (offset + length) / BYTES_PER_BLOCK
	uint32_t end_block_offset = (offset + length) % BYTES_PER_BLOCK; 			//offset within the current data block

	uint32_t i;

	
	for(i=start_index ; i <=end_index ; i++){
		if(i == start_index){

		}
		else if(i == end_index){

		}
		else{

		}
	}

	return 0;
}

//write data to a file, should never be called since we have a read only file system
int32_t write_data(uint32_t inode, uint32_t offset, const uint8_t* buf, uint32_t length){
	return -1; //write data always fails, read only
}

//open a file
int32_t open_file(const uint8_t* fname){


	return 0;
}

int32_t close_file(int32_t fd){


	return 0;
}


int32_t file_name_cmp(const uint8_t* s1, const uint8_t* s2)
{
	uint8_t i;
	for(i=0; i<32; i++){
		if(s1[i]!=s2[i])
			return 0;
	}
	return 1;
}
