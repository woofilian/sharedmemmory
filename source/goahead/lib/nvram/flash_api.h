#ifndef __FLASH_API__
#define __FLASH_API__

//int flash_read(char *buf, off_t from, int len);
//int flash_write(char *buf, off_t to, int len);
//int flash_read_oem(char *buf, off_t from, int len);
//int flash_write_oem(char *buf, off_t to, int len);
//int flash_write_other(char *buf, int len);

int flash_read_byname(char *part_name, char *buf, off_t from, int len);
int flash_write_byname(char *part_name, char *buf, int len);


#endif
