void putchar(unsigned char c);
void myfunc(int a, unsigned char b);
unsigned char init_screen();
char read_sector(unsigned char * buffer,unsigned char drive,unsigned char track,unsigned char sector);
char write_sector(unsigned char * buffer,unsigned char drive,unsigned char track,unsigned char sector);
char wait_key();
void init_key(unsigned char c);
char reboot();
char read_file(unsigned char * buffer,unsigned char * dest_buffer,unsigned char filenamelen);
void cfg_disk_drive(unsigned char * buffer);
void move_to_track(unsigned char track);
void init_fpamsdos();


