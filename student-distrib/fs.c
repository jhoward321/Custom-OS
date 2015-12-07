#include "fs.h"

boot_block_t* boot_block;

uint32_t dir_index = 0; //file directory index

/*
* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
*   Inputs: const uint8_t* fname = file name
*			dentry_t* dentry = directory entry pointer
*   Return Value: 0 on success, -1 on failure
*	Function: find the directory entry in the file system specified by fname,
* 			set it to 'dentry'.
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){

	if(fname == NULL)
		return -1;

	if(dentry == NULL)
		return -1;


	uint32_t name_length = strlen((int8_t*)fname); 		//length of name

	//make sure MAX_FILE_NAME_LENGTH doesn't exceed max length allowed for file length
	name_length = (name_length > MAX_FILE_NAME_LENGTH-1) ? MAX_FILE_NAME_LENGTH-1 : name_length;

											//iterator
	uint32_t i;
	//changed to strncmp instead, because the file name strings used in kernel.c won't have a zero-padding
	for(i=0; i<boot_block->total_dirs; i++){
		if (strncmp((int8_t*)fname, (int8_t*)boot_block->dir_entries[i].file_name, name_length) == 0) {
			if (name_length != MAX_FILE_NAME_LENGTH && boot_block->dir_entries[i].file_name[name_length] != 0)	//if file has characters after name length then strings are not equal
				return -1;
			memcpy(dentry, &boot_block->dir_entries[i], DENTRY_SIZE);
			return 0;
		}
	}
	return -1;
}

/*
* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
*   Inputs: uint32_t index = dir entry index number
*			dentry_t* dentry = directory entry pointer
*   Return Value: 0 on success, -1 on failure
*	Function: set dentry to the directory entry in the boot block at index
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	if(index > boot_block->total_dirs || index < 0 || dentry == NULL)
		return -1;
	memcpy(dentry, &boot_block->dir_entries[index], DENTRY_SIZE);
	return 0;
}


/*
* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
*   Inputs: uint32_t inode = index node
*		uint32_t offset = offset into file
* 		uint8_t* buf = output buffer
* 		uint32_t length = desired length to be read
*   Return Value: number of bytes read on success, -1 on failure
*	Function: read 'length' number of bytes from file given in inode number 
* (starting offset bytes into the file), and store it in buffer
*/
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


	//address of the first data block
	uint32_t* data_block_0_address = (uint32_t*)((uint32_t)boot_block + (boot_block->total_inodes + 1) * BYTES_PER_BLOCK);

	//address of current data block number to be read (list of data block numbers are stored in the inode)
	uint32_t* curr_data_block_index_addr = (uint32_t*)((uint32_t)curr_inode_address + (offset/BYTES_PER_BLOCK)*4 + sizeof(uint32_t)); 	// 4 = sizeof(int)
	
	//current data block number to be read (stored in the inode)
	uint32_t curr_data_block_index = *curr_data_block_index_addr;

	//address of the current data block to be read
	uint32_t* curr_data_block_address = (uint32_t*)((uint32_t)data_block_0_address + (curr_data_block_index * BYTES_PER_BLOCK));

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
			return read_count;
		}
		//end of block reached? : read from next block
		if(byte_position % BYTES_PER_BLOCK == 0){

			curr_data_block_index_addr++; 					//address points to the next data block index in inode
			curr_data_block_index = *curr_data_block_index_addr;	//get the next data block index
			//formula for getting the current data address is the same
			curr_data_block_address = (uint32_t*)((uint32_t)data_block_0_address + (curr_data_block_index * BYTES_PER_BLOCK));
		}
	}


	//if EOF not reached, return the number of bytes read
	return read_count;
}
/*
* uint32_t read_file_length(uint32_t inode);
*   Inputs: uint32_t inode = index node
*   Return Value: length of file given in inode
*	Function: returns the given file length represented by the inode 
*/
uint32_t read_file_length(uint32_t inode){
	return *((uint32_t*)((uint32_t)boot_block + BYTES_PER_BLOCK * (inode + 1)));
}




// ============FILE OPERATIONS==============

/*
* int32_t read_file(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: length of file given in inode
*	Function: read 'length' number of bytes from file given in the file in current pcb
* located at its fd index in its file array (starting offset bytes into the file), and store it in buffer
*/
int32_t read_file(int32_t fd, uint8_t* buf, int32_t length){

	uint32_t curr_inode_number = curr_task[current_terminal]->file_array[fd].inode_number;
	uint32_t file_len = read_file_length(curr_inode_number);

	if(file_len <= curr_task[current_terminal]->file_array[fd].file_position)
		return 0;

	uint32_t offset = curr_task[current_terminal]->file_array[fd].file_position;
	uint32_t read_amount = read_data(curr_inode_number, offset, buf, length);
	curr_task[current_terminal]->file_array[fd].file_position += read_amount;
	return read_amount;
}

/*
* int32_t write_file(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: -1
*	Function: none
*/
int32_t write_file(int32_t fd, uint8_t* buf, int32_t length){
	return -1; //write data always fails, read only
}

/*
* int32_t open_file(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: 0
*	Function: set the flag of the file in fd to used and set its file position to 0
*/
int32_t open_file(int32_t fd, uint8_t* buf, int32_t length){
	curr_task[current_terminal]->file_array[fd].file_position = 0;
	curr_task[current_terminal]->file_array[fd].flags = USED;
	return 0;
}

/*
* int32_t close_file(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: 0
*	Function: set the flag of the file in fd to free
*/
int32_t close_file(int32_t fd, uint8_t* buf, int32_t length){
	curr_task[current_terminal]->file_array[fd].flags = FREE;
	return 0;
}



// ============DIR OPERATIONS==============

/*
* int32_t read_dir(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: number of files read in success, 0 in failure
*	Function: set the flag of the file in fd to free
*/
int32_t read_dir(int32_t fd, uint8_t* buf, int32_t length){

	dentry_t temp;

	//is this right berk? not sure if i interpreted this part right
	//i moved index to dir_index which is global
	if(dir_index >= boot_block->total_dirs){
		dir_index = 0;
		return 0;
	}

	int i;
	read_dentry_by_index(dir_index, &temp);

	for(i = 0; i < strlen((int8_t*)temp.file_name); i++){
		buf[i] = temp.file_name[i];
	}
	buf[i+1] = '\0'; //might want to null terminate instead but we'll try this

	dir_index++; //increment read file index
	return i; //i is same as doing strlen(buf) but we already have it calculated
}

/*
* int32_t write_dir(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: -1
*	Function: none
*/
int32_t write_dir(int32_t fd, uint8_t* buf, int32_t length){
	return -1;
}

/*
* int32_t open_file(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: 0
*	Function: set the flag of the directory in fd to used and set its file position to 0
*/
int32_t open_dir(int32_t fd, uint8_t* buf, int32_t length){

	curr_task[current_terminal]->file_array[fd].inode_number = 0;
	curr_task[current_terminal]->file_array[fd].file_position = 0;
	curr_task[current_terminal]->file_array[fd].flags = USED;
	return 0;
}

/*
* int32_t open_file(int32_t fd, uint8_t* buf, int32_t length)
*   Inputs: int32_t fd = file descriptor
*		uint8_t* buf = buffer
*		int32_t length = length
*   Return Value: 0
*	Function: none
*/
int32_t close_dir(int32_t fd, uint8_t* buf, int32_t length){
	return 0;
}
