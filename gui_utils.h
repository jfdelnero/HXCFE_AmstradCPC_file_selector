void init_display(void);
void show_help(void);
void show_parameters(void);

void hxc_printf(unsigned char x_pos, unsigned char y_pos, char * chaine, ...);
void hxc_fastprintf( unsigned char x_pos, unsigned char y_pos, char *string );
void hxc_printf_box(char * chaine, ...);
void hxc_printf_box_error(char * chaine, ...);

#define NUMBER_OF_SLOT 16
#define CFG_FILE_SIZE (1024 + (NUMBER_OF_SLOT * 128))

#define PATH_Y_POS 2
#define FILELIST_Y_POS 4
#define NUMBER_OF_FILE_ON_DISPLAY 19
#define SLOT_Y_POS 24
#define INSTRUCTIONS_Y_POS 2
#define ALLSLOTS_Y_POS 2
#define FIRMWARE_VERSION_X_POS 40
#define FIRMWARE_VERSION_Y_POS 0
#define MESSAGE_Y_POS 24