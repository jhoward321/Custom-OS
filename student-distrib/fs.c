#include "fs.h"


boot_block_t* boot_block;

//search the file system for
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){

	if(fname == NULL)
		return -1;

	if(dentry == NULL)
		return -1;


	uint32_t name_length = strlen((int8_t*)fname); 		//length of name

	//make sure MAX_FILE_NAME_LENGTH doesn't exceed max length allowed for file length
	name_length = (name_length > MAX_FILE_NAME_LENGTH-1) ? MAX_FILE_NAME_LENGTH-1 : name_length;

	uint32_t i;										//iterator

	//changed to strncmp instead, because the file name strings used in kernel.c won't have a zero-padding
	for(i=0; i<boot_block->total_dirs; i++){
		if (strncmp((int8_t*)fname, (int8_t*)boot_block->dir_entries[i].file_name, name_length) == 0) {
			memcpy(dentry, &boot_block->dir_entries[i], DENTRY_SIZE);

	/*		memcpy(dentry->file_name, boot_block->dir_entries[i].file_name, MAX_FILE_NAME_LENGTH);
			dentry->file_type = boot_block->dir_entries[i].file_type;
			dentry->inode_number = boot_block->dir_entries[i].inode_number;*/
			return 0;
		}
	}
	return -1;
}


int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	if(index > boot_block->total_dirs || index < 0 || dentry == NULL)
		return -1;

	memcpy(dentry, &boot_block->dir_entries[index], DENTRY_SIZE);
/*	memcpy(dentry->file_name, boot_block->dir_entries[index].file_name, MAX_FILE_NAME_LENGTH);
	dentry->file_type = boot_block->dir_entries[index].file_type;
	dentry->inode_number = boot_block->dir_entries[index].inode_number;*/


	return 0;
}


uint32_t read_dir(uint8_t* buf){

	dentry_t temp;

	int index = 0;
	int i, j;

	for(i=0; i<MAX_NUM_FILES; i++){
		if(read_dentry_by_index(i, &temp) == 0){
			for(j=0; j<32; j++){
				buf[index++] = temp.file_name[j];
			}
			buf[index++] ='\n';
		}
	}

	return index;

}

//copy data from a file from at offset, of length bytes, and store in buf
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	if(inode > boot_block->total_inodes || inode < 0 || buf == NULL)
		return -1;

	//address of the current inode where data will be extracted
	uint32_t* curr_inode_address = ((uint32_t*)((uint32_t)boot_block + BYTES_PER_BLOCK * (inode + 1)));
	//first element of the inode structure gives the length of the file to be read
	uint32_t file_length = *curr_inode_address;
	//check to see if the offset points to outside of the file's size
	if(file_length < offset){
		return -1;
	}

	//printf("READ DATA:\n\n");

	//address of the first data block
	uint32_t* data_block_0_address = (uint32_t*)((uint32_t)boot_block + (boot_block->total_inodes + 1) * BYTES_PER_BLOCK);
	//printf("Data block base address = %d\n",data_block_0_address);

	//address of current data block number to be read (list of data block numbers are stored in the inode)
	uint32_t* curr_data_block_index_addr = (uint32_t*)((uint32_t)curr_inode_address + offset/BYTES_PER_BLOCK + sizeof(uint32_t)); 	// 4 = sizeof(int)
	//printf("Current data block index address = %d\n", curr_data_block_index_addr);
	//current data block number to be read (stored in the inode)
	uint32_t curr_data_block_index = *curr_data_block_index_addr;
	//printf("Current data block index = %d\n", curr_data_block_index);

	//address of the current data block to be read
	uint32_t* curr_data_block_address = (uint32_t*)((uint32_t)data_block_0_address + (curr_data_block_index * BYTES_PER_BLOCK));
	//printf("Current data block address = %d\n", curr_data_block_address);

	uint32_t read_count = 0; 				//number of bytes read thus far
	uint32_t byte_position = 0; 			//byte position into the file
	// uint8_t curr_byte;						//current byte that is being read into the buffer

	//variable to check how far in the file we are reading
	byte_position = offset + read_count;

	while(length > read_count){

		//read the byte from the corresponding offset of the current data block
		buf[read_count] = *((uint8_t*)((uint32_t)curr_data_block_address + byte_position % BYTES_PER_BLOCK));

		read_count++;							//increment bytes read counter
		byte_position = offset + read_count; 	//re-set the byte position

		//EOF reached ?
		if (byte_position == file_length){
			return 0;
		}
		//end of block reached? : read from next block
		if(byte_position % BYTES_PER_BLOCK == 0){

			//VVVVV-----MAKE SURE THIS INCREMENTS BY THE APPROPRIATE AMOUNT
			curr_data_block_index_addr++; 					//address points to the next data block index in inode
			curr_data_block_index = *curr_data_block_index_addr;	//get the next data block index
			//formula for getting the current data address is the same
			curr_data_block_address = (uint32_t*)((uint32_t)data_block_0_address + (curr_data_block_index * BYTES_PER_BLOCK));
		}
	}


	//if EOF not reached, return the number of bytes read
	return read_count;
}


//returns the given file length represented by the inode (all calculations are copied from the above read_data function)
uint32_t read_file_length(uint32_t inode){
	//address of the current inode where data will be extracted
	uint32_t* curr_inode_address = ((uint32_t*)((uint32_t)boot_block + BYTES_PER_BLOCK * (inode + 1)));
	//first element of the inode structure gives the length of the file to be read
	uint32_t file_length = *curr_inode_address;
	return file_length;
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