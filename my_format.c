#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "fat12.h"

#define OEM_ID_LENGTH 8

int fid; /* global variable set by the open() function */

#define DEFAULT_SECTOR_SIZE 512

int fd_read(int sector_number, char *buffer) {
	int dest, len;
	int bps = DEFAULT_SECTOR_SIZE;

	dest = lseek(fid, sector_number * DEFAULT_SECTOR_SIZE, SEEK_SET);
	if (dest != sector_number * bps){
	/* Error handling */
	}

	len = read(fid, buffer, bps);
	if (len != bps){
		/* error handling here */
	}

	return len;
}

/* Writes to disk */
int fd_write(int sector_number, char *buffer){
	int dest, len;
	int bps = DEFAULT_SECTOR_SIZE;

	dest = lseek(fid, sector_number * DEFAULT_SECTOR_SIZE, SEEK_SET);
	if (dest != sector_number * bps){
		/* Error handling */
	}

	len = write(fid, buffer, bps);

	if (len != bps){
		/* error handling here */
	}
	return len;
}


void setup_boot_oemid(boot_record_t* boot_sector) {
	short int i;
	char oem_id_string[OEM_ID_LENGTH] = "MSWIN4.1";

	for (i = 0; i <= sizeof(oem_id_string); i++) {
		boot_sector->oem_id[i] = oem_id_string[i];
	}
}

int main(int argc, char *argv[])
{
	boot_record_t boot;

	if (argc != 2)
	{
		printf("Usage: %s <floppy_image>\n", argv[0]);
		exit(1);
	}

	// fid is file descriptor to floppy.img
	if ( (fid = open (argv[1],  O_RDWR|O_CREAT, 0777))  < 0 )
	{
		perror("Error: ");
		return -1;
	}
	
	/* See fat12.pdf for layout detail	s */
	setup_boot_oemid(&boot);
	boot.sector_size = DEFAULT_SECTOR_SIZE;
	boot.sectors_per_cluster = 1;
	boot.reserved_sector_count = 1;
	boot.number_of_fats = 2;
	boot.number_of_dirents = 224;
	boot.sector_count = 2880;
	boot.media_type = 0xF0; // Removable media
	boot.fat_size_sectors = 9;
	boot.sectors_per_track = 18;
	boot.nheads = 2;
	boot.sectors_hidden = 0;
	boot.sector_count_large = 0;


	// Step 1. Create floppy.img with the school solution. Read the values 
	// from the boot sector of the floppy.img and initialize boot sector
	// with those values. If you read bootsector of the floppy.img correctly,
	// the values will be:



	//	sector_size: 512 bytes
	//	sectors_per_cluster: 1 sectors
	//	reserved_sector_count: 1
	//	number_of_fats: 2
	//	number_of_dirents: 224
	//	sector_count: 2880 sectors
	//	media_type: 0xf0
	//	fat_size_sectors: 9
	//	sectors_per_track: 18
	//	nheads: 2
	//	sectors_hidden: 0
	//	sector_count_large: 0 sectors
	


	// boot_record_t boot;
    // if (read(fid, &boot, sizeof (boot)) > 0){
	  printf("sector_size: %d\n", boot.sector_size);
	  printf("sectors_per_cluster: %d\n", boot.sectors_per_cluster);
	  printf("reserved_sector_count: %d\n", boot.reserved_sector_count);
	  printf("number_of_fats: %d\n", boot.number_of_fats);
	  printf("number_of_dirents: %d\n", boot.number_of_dirents);
	  printf("sector_count: %d\n", boot.sector_count);
	// }
    // etc ...
	
	// Step 2. Zero FAT1 and FAT2 tables according to the fat12.pdf.

	  // First, write the boot sector to the file (To the first sector)
	  // Writing only the size of the sector size
	  char trimmed_boot_buffer[DEFAULT_SECTOR_SIZE];
	  memcpy(trimmed_boot_buffer, &boot, DEFAULT_SECTOR_SIZE);
	  fd_write(0, &boot);

	  // Second, We will zero the values of the fat table.
	  // A 12-bit FAT entry Could have different values according to the cluster's usage,
	  // Whereas 0x000 is unused cluster, and 0xFF0 - 0xFF6 is a reserved cluster (like FAT entries 1 and 2)
	  char reservedEntry[DEFAULT_SECTOR_SIZE] = {0xFF};
	  char unusedEntry[DEFAULT_SECTOR_SIZE] = {0x0};

	  // There There  are  3072  FAT  entries  in  each  FAT  table  (512  bytes  per  sector  *  9  sectors  =  4608
	  //bytes. 4608 bytes / 1.5 bytes per FAT entry = 3072 FAT entries).
	  // Only the first sector out of total 9 is reserved
	  fd_write(1, reservedEntry);

	  for (short i = 2; i <= 9; i++) {
		  fd_write(i, unusedEntry);
	  }

	  // Doing the same for the second fat entry
	  fd_write(10, reservedEntry);

	  for (short i = 11; i <= 18; i++) {
	  	  fd_write(i, unusedEntry);
	  }


	// Step 3. Set direntries as free according to the fat12.pdf.

	  /*The  Root  Directory
		is  the  primary  directory  of  the  disk.
		Unlike  other  directories located in the data area of the disk,
		the root directory has a finite size (14 sectors * 16 directory entries per sector = 224 possible entries)
		restricting the total amount of files or directories that can be created therein. */

	  // Creating buffer with the first value of fat_file.name marks it as empty
	  char free_dirent = 0xe5;
	  char empty_dirent_buf[DEFAULT_SECTOR_SIZE] = {0x0};
	  empty_dirent_buf[0] = free_dirent;

	  printf("%s", empty_dirent_buf);

	  // Starting from sector 19, 14 sectors is 19 - 32
	  for (short i = 19; i <= 32; i++) {
		  fd_write(i, empty_dirent_buf);
	  }
	
	// Step 4. Handle data block (e.g you can zero them or just leave 
	// untouched. What are the pros/cons?)

	  // Zeroing the other sectors
	  for (short i = 33; i <= boot.sector_count; i++) {
		  fd_write(i, unusedEntry);
	  }
	

	// For steps 2-3, you can also read the sectors from the image that was 
	// generated by the school solution if not sure what should be the values.
	
	
	close(fid);
	return 0;
}

