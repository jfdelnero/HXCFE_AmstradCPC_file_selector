#ifndef __FAT_FILELIB_H__
#define __FAT_FILELIB_H__

#include "fat_opts.h"
#include "fat_access.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifndef SEEK_CUR
	#define SEEK_CUR    1
#endif

#ifndef SEEK_END
	#define SEEK_END    2
#endif

#ifndef SEEK_SET
	#define SEEK_SET    0
#endif

#ifndef EOF
	#define EOF			(-1)
#endif

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct sFL_FILE;

typedef struct sFL_FILE
{
	unsigned long			parentcluster;
	unsigned long			startcluster;
	unsigned long			bytenum;
	unsigned long			filelength;
	int						filelength_changed;
	char					path[FATFS_MAX_LONG_FILENAME];
	char					filename[FATFS_MAX_LONG_FILENAME];
	unsigned char			shortfilename[11];

	// Read/Write sector buffer
	struct sector_buffer	file_data;

	// File fopen flags
	unsigned char			flags;
#define FILE_READ	(1 << 0)
#define FILE_WRITE	(1 << 1)
#define FILE_APPEND	(1 << 2)
#define FILE_BINARY	(1 << 3)
#define FILE_ERASE	(1 << 4)
#define FILE_CREATE	(1 << 5)

	struct sFL_FILE			*next;
} FL_FILE;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

// External
void				fl_init(void);
void				fl_attach_locks(struct fatfs *fs, void (*lock)(void), void (*unlock)(void));
int					fl_attach_media(fn_diskio_read rd, fn_diskio_write wr);
void				fl_shutdown(void);

// Standard API
void*				fl_fopen(const char *path, const char *modifiers);
void				fl_fclose(void *file);
int					fl_fflush(void *file);
int					fl_fgetc(void *file);
int					fl_fputc(int c, void *file);
int					fl_fputs(const char * str, void *file);
int					fl_fwrite(const void * data, int size, int count, void *file );
int                                     fl_fswrite(unsigned char * buffer, int size, void *f);
int					fl_fread(void * data, int size, int count, void *file );
int					fl_fseek(void *file , long offset , int origin );
int					fl_fgetpos(void *file , unsigned long * position);
long				fl_ftell(void *f);
int					fl_feof(void *f);
int					fl_remove( const char * filename );	

// Extensions
void				fl_listdirectory(const char *path);
int					fl_createdirectory(const char *path);
unsigned long fl_change_entry(UINT32 cluster,int dir,unsigned long entryindex,FAT32_ShortEntry *sfEntry,unsigned char * filename) ;

UINT32 get_root_cluster();


#endif
