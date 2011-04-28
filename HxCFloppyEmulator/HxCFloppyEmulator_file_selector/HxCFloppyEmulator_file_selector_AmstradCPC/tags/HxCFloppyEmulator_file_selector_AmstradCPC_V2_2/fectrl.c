/*
//
// Copyright (C) 2009, 2010, 2011 Jean-François DEL NERO
//
// This file is part of the HxCFloppyEmulator file selector.
//
// HxCFloppyEmulator file selector may be used and distributed without restriction
// provided that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator file selector is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator file selector; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : fectrl.c
// Contains: SD HxC Floppy Emulator manager main routines (Amstrad CPC version).
//
// Written by:	Jean-François DEL NERO  (Jeff)
//              Arnaud STORQ (Norecess)
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpcbioscall.h"
#include "gui_utils.h"
#include "cfg_file.h"
#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"
#include "fat_access.h"
#include "sdhxcfloppyemu_directaccess.h"

#define LFN_MAX_SIZE 74
#define FL_STATUSTABLEN 32

typedef struct DirectoryEntry_ {
	unsigned char name[12];
	unsigned char attributes;
	unsigned long firstCluster;
	unsigned long size;
	unsigned char longName[17];	// boolean
}DirectoryEntry;


char selectorpos;
unsigned char slotnumber;
unsigned char floppydrive;

char currentPath[256];
char temp_buffer[257];
unsigned char sector[512];
unsigned char sdfecfg_file[512];

DirectoryEntry DirectoryEntry_tab[NUMBER_OF_FILE_ON_DISPLAY];
disk_in_drive disks_slot_a[NUMBER_OF_SLOT];

struct fat_dir_entry sfEntry;
struct fs_dir_list_status file_list_status;
struct fs_dir_list_status file_list_status_tab[FL_STATUSTABLEN];
struct fat_dir_entry sfEntry;
struct fs_dir_ent dir_entry;

unsigned long last_setlbabase;

unsigned char read_entry;
unsigned char y_pos;

const char mess_reboot[]="Rebooting...";
const char mess_init[]="Initializing...                                                                 ";
const unsigned char fpcfgbuffer[9] = {0x23, 0x00, 0xFF, 0x03, 0xAF, 0x02, 0x0A, 0x01, 0x03};
static char filter[17];

unsigned char g_step_sound;     //0x00 -> off 0xFF->on
unsigned char g_ihm_sound;     //0x00 -> off 0xFF->on
unsigned char g_back_light_tmr; //0x00 always off, 0xFF always on, other -> on x second
unsigned char g_standby_tmr;    //0xFF disable, other -> on x second
unsigned char g_buzzer_duty_cycle;
unsigned char g_buzzer_step_duration;  // 0xD8 <> 0xFF


/*void hxc_memcpy(void* d,void* s,unsigned short copyCount)
{
  unsigned short c;
  unsigned char * dst;
  unsigned char * src;

  dst=(unsigned char *)d;
  src=(unsigned char *)s;
	c=0;
	do
	{

		*dst=*src;
		dst++;
                src++;
		c++;
	}while(c<copyCount);
}
*/

/*
unsigned long indexptr;

void print_hex(unsigned char * buffer, int size)
{
	int c,i;

	c=0;

	for(i=0;i<size;i++)
	{
		printf("%.2X ",buffer[i]);
		c++;
		if(!(c&0xF))
		{
			printf("\r\n%.3X: ",indexptr);
			indexptr=indexptr+16;
		}
	}
} */

unsigned char Flopwr( unsigned char * buffer,unsigned char diskdrive, unsigned char sector_id, unsigned char track, unsigned char count )
{
	unsigned char ret;
	do
	{
		ret=write_sector(buffer,diskdrive,track,sector_id);
		buffer=buffer+512;
		count--;
		sector_id++;
	}while(count && !ret);

	return ret;
}

unsigned char Floprd( unsigned char * buffer,unsigned char diskdrive, unsigned char sector_id, unsigned char track, unsigned char count )
{
	unsigned char ret;

	do
	{
		ret=read_sector(buffer,diskdrive,track,sector_id);
		buffer=buffer+512;
		count--;
		sector_id++;
	}while(count && !ret);

	return ret;
}

void strlwr(unsigned char * str)
{
  unsigned char i;
  i=0;
  while( str[i])
  {
         if( str[i]>='A' && str[i]<='Z')
         {
            str[i]=str[i]+ ('a'-'A');
         }
  }
  
}
char setlbabase(unsigned long lba)
{
	char ret;
	unsigned char * ptr;
	direct_access_cmd_sector * dacs;

	dacs=(direct_access_cmd_sector  *)sector;

	sprintf(dacs->DAHEADERSIGNATURE,"HxCFEDA");
	dacs->cmd_code=1;

	ptr=(unsigned char*)&lba;
	dacs->parameter_0=*ptr++;
	dacs->parameter_1=*ptr++;
	dacs->parameter_2=*ptr++;
	dacs->parameter_3=*ptr++;
	dacs->parameter_4=0xA5;
	dacs->parameter_5=0x00;

	ret=Flopwr( sector, floppydrive, 0, 255, 1 );
	return ret;
}


int media_init()
{
	unsigned char ret;
	direct_access_status_sector * dass;
	unsigned char ** ptr;

	ptr=(unsigned char **)0xBE7D;

	floppydrive=**ptr;

	last_setlbabase=0xFFFFFF00;

	cfg_disk_drive((unsigned char *)&fpcfgbuffer);


	ret=Floprd((unsigned char*)&sector, floppydrive, 0, 255, 1 );
	if(!ret)
	{
		dass=(direct_access_status_sector *)sector;
		if(!strcmp(dass->DAHEADERSIGNATURE,"HxCFEDA"))
		{
			hxc_printf(FIRMWARE_VERSION_X_POS,FIRMWARE_VERSION_Y_POS,"- Firmware %s - 'H' for help" ,dass->FIRMWAREVERSION);
			return 1;
		}

		hxc_printf_box_error("Bad signature - HxC Floppy Emulator not found!");
	}
	hxc_printf_box_error("ERROR: Floppy Access error!  [%d]",ret);

	return 0;
}


int media_read(unsigned long lba_sector, unsigned char *buffer)
{
	unsigned char ret;
	unsigned long diff;

	hxc_fastprintf(76,0,"READ");

	diff=lba_sector-last_setlbabase;

	if(diff<8)
	{
		ret=Floprd( (unsigned char*)buffer, floppydrive, (diff)+1, 255, 1 );
	}
	else
	{
		setlbabase(lba_sector);
		ret=Floprd( (unsigned char*)buffer, floppydrive, 1, 255, 1 );
		last_setlbabase=lba_sector;
	}

	if(!ret)
	{
		hxc_fastprintf(76,0,"    ");
		return 1;
	}
	else
	{
		hxc_printf_box_error("ERROR: Floppy Write Access error!  [%d:%d]",floppydrive,ret);
		return 0;
	}
}

int media_write(unsigned long lba_sector, unsigned char *buffer)
{
	unsigned long diff;
	unsigned char ret;

	hxc_fastprintf(75,0,"WRITE");

	diff=lba_sector-last_setlbabase;

	if(diff<8)
	{
		ret=Flopwr( (unsigned char*)buffer, floppydrive, (diff)+1, 255, 1 );
	}
	else
	{
		setlbabase(lba_sector);
		ret=Flopwr( (unsigned char*)buffer, floppydrive, 1, 255, 1 );
		last_setlbabase=lba_sector;
	}
	
	if(!ret)
	{
		hxc_fastprintf(75,0,"     ");
		return 1;
	}

	hxc_printf_box_error("ERROR: Floppy Write Access error!  [%d]",ret);
	return 0;
}

void show_all_slots(void)
{
	char tmp_str[17];
	int i;

	for ( i = 1; i < NUMBER_OF_SLOT; i++ )
	{
		if( disks_slot_a[i].DirEnt.size)
		{
			memcpy(tmp_str,disks_slot_a[i].DirEnt.longName,16);
			tmp_str[16]=0;
		}
		else
		{
			tmp_str[0]=0;
		}
		hxc_printf(0,ALLSLOTS_Y_POS + i,"Slot %d: %s", i, tmp_str);
	}

	hxc_printf(0,ALLSLOTS_Y_POS + NUMBER_OF_SLOT + 1,"Press a key...");
	wait_key();
}

void printslotstatus(unsigned char slotnumber,  disk_in_drive * disks)
{
	char tmp_str[17];

	clear_line(SLOT_Y_POS);

	if( disks[0].DirEnt.size)
	{
		memcpy(tmp_str,disks[0].DirEnt.longName,16);
		tmp_str[16]=0;
	}
	else
	{
		tmp_str[0]=0;
	}
	hxc_printf(0,SLOT_Y_POS,"Slot %d: %s", slotnumber, tmp_str);
}


char read_cfg_file(unsigned char * sdfecfg_file)
{
	char ret;
	unsigned char number_of_slot;
	unsigned short i;
	cfgfile * cfgfile_ptr;
	FL_FILE *file;

	memset((void*)&disks_slot_a,0,sizeof(disk_in_drive)*NUMBER_OF_SLOT);

	ret=0;
	file = fl_fopen("/HXCSDFE.CFG", "r");
	if (file)
	{
		cfgfile_ptr=(cfgfile * )sdfecfg_file;

		fl_fread(sdfecfg_file, 1, 512 , file);
		number_of_slot=cfgfile_ptr->number_of_slot;

		g_step_sound = cfgfile_ptr->step_sound;     //0x00 -> off 0xFF->on
		g_ihm_sound = cfgfile_ptr->ihm_sound;     //0x00 -> off 0xFF->on
		g_back_light_tmr = cfgfile_ptr->back_light_tmr; //0x00 always off, 0xFF always on, other -> on x second
		g_standby_tmr = cfgfile_ptr->standby_tmr;    //0xFF disable, other -> on x second
		g_buzzer_duty_cycle = cfgfile_ptr->buzzer_duty_cycle;
		g_buzzer_step_duration = cfgfile_ptr->buzzer_step_duration;  // 0xD8 <> 0xFF    		

		fl_fseek(file , 1024 , SEEK_SET);

		fl_fread(sdfecfg_file, 1, 512 , file);
		i=1;
		do
		{
			if(!(i&3))
			{
				fl_fread(sdfecfg_file, 1, 512 , file);
			}

			memcpy(&disks_slot_a[i],&sdfecfg_file[(i&3)*128],sizeof(disk_in_drive));
			i++;
		}while(i<number_of_slot);

		fl_fclose(file);
	}
	else
	{
		ret=1;
	}

	if(ret)
	{
		hxc_printf_box_error("ERROR: Access HXCSDFE.CFG file failed! [%d]",ret);
	}

	return ret;
}

char save_cfg_file(unsigned char * sdfecfg_file)
{
	unsigned char number_of_slot,slot_index;
	unsigned char i,sect_nb,ret;
	cfgfile * cfgfile_ptr;
	unsigned short  floppyselectorindex;
	FL_FILE *file;

        ret=0;
	file = fl_fopen("/HXCSDFE.CFG", "r");
	if (file)
	{
		number_of_slot=1;
		slot_index=1;
		i=1;

		floppyselectorindex=128;                      // Fisrt slot offset
		memset( sdfecfg_file,0,512);                  // Clear the sector
		sect_nb=2;                                    // Slots Sector offset

		do
		{
			if( disks_slot_a[i].DirEnt.size)            // Valid slot found
			{
				// Copy it to the actual file sector
				memcpy(&sdfecfg_file[floppyselectorindex],&disks_slot_a[i],sizeof(disk_in_drive));

				//Next slot...
				number_of_slot++;
				floppyselectorindex=(floppyselectorindex+128)&0x1FF;

				if(!(number_of_slot&0x3))                // Need to change to the next sector
				{
					// Save the sector
					if (fl_fswrite((unsigned char*)sdfecfg_file, 1,sect_nb, file) != 1)
					{
						hxc_printf_box_error("ERROR: Write file failed!");
                            			ret=1;
					}
					// Next sector
					sect_nb++;
					memset( sdfecfg_file,0,512);                  // Clear the next sector
				}
			}

			i++;
		}while(i<NUMBER_OF_SLOT);

		if(number_of_slot&0x3)
		{
			if (fl_fswrite((unsigned char*)sdfecfg_file, 1,sect_nb, file) != 1)
			{
				hxc_printf_box_error("ERROR: Write file failed!");
				ret=1;
			}
        }

		if(slot_index>=number_of_slot)
		{
			slot_index=number_of_slot-1;
		}

		fl_fseek(file , 0 , SEEK_SET);

		// Update the file header
		fl_fread(sdfecfg_file, 1, 512 , file);

		cfgfile_ptr=(cfgfile * )sdfecfg_file;
		cfgfile_ptr->number_of_slot=number_of_slot;
		cfgfile_ptr->slot_index=slot_index;

		cfgfile_ptr->step_sound = g_step_sound;     //0x00 -> off 0xFF->on
		cfgfile_ptr->ihm_sound = g_ihm_sound;     //0x00 -> off 0xFF->on
		cfgfile_ptr->back_light_tmr = g_back_light_tmr; //0x00 always off, 0xFF always on, other -> on x second
		cfgfile_ptr->standby_tmr = g_standby_tmr;    //0xFF disable, other -> on x second
		cfgfile_ptr->buzzer_duty_cycle = g_buzzer_duty_cycle;
		cfgfile_ptr->buzzer_step_duration = g_buzzer_step_duration;  // 0xD8 <> 0xFF

		if (fl_fswrite((unsigned char*)sdfecfg_file, 1,0, file) != 1)
		{
			hxc_printf_box_error("ERROR: Write file failed!");
			ret=1;
		}

	}
	else
	{
		hxc_printf_box_error("ERROR: Create file failed!");
		ret=1;
	}
	// Close file
	fl_fclose(file);

	return ret;
}


void clear_list()
{
	unsigned char y_pos,i;

	//clear_line(24);

	y_pos=FILELIST_Y_POS;

	for(i=0;i<NUMBER_OF_FILE_ON_DISPLAY;i++)
	{
        clear_line(y_pos);
        y_pos++;
	}
}


void next_slot()
{
	slotnumber++;
	if(slotnumber>(NUMBER_OF_SLOT-1))   slotnumber=1;
	printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber]) ;
}

void displayFolder()
{
	clear_line( PATH_Y_POS );
	hxc_fastprintf( 0, PATH_Y_POS, "Directory:" );
	hxc_fastprintf( 11, PATH_Y_POS, currentPath );
}

void backup_flstatus(unsigned char fl_index)
{
		unsigned char * src;
		unsigned char * dst;
		unsigned char i;

		dst=(unsigned char*)&file_list_status_tab[fl_index&(FL_STATUSTABLEN-1)];
		src=(unsigned char*)&file_list_status;

		for(i=0;i<sizeof(struct fs_dir_list_status);i++)
		{
			dst[i]=src[i];
		}
		//memcpy(&file_list_status_tab[fl_index&(FL_STATUSTABLEN-1)],&file_list_status ,sizeof(struct fs_dir_list_status));
}


void restore_flstatus(unsigned char fl_index)
{
		unsigned char * src;
		unsigned char * dst;
		unsigned char i;

		src=(unsigned char*)&file_list_status_tab[fl_index&(FL_STATUSTABLEN-1)];
		dst=(unsigned char*)&file_list_status;

		for(i=0;i<sizeof(struct fs_dir_list_status);i++)
		{
			dst[i]=src[i];
		}
		//memcpy(&file_list_status,&file_list_status_tab[fl_index&(FL_STATUSTABLEN-1)],sizeof(struct fs_dir_list_status));
}

void enter_sub_dir(disk_in_drive *disk_ptr)
{
	unsigned char currentPathLength;
	unsigned char c,i,j;

	currentPathLength = strlen( currentPath );
	if (currentPathLength && (disk_ptr->DirEnt.longName[0] == (unsigned char)'.') && (disk_ptr->DirEnt.longName[1] == (unsigned char)'.') )
	{
		currentPath[ currentPathLength-1 ]=0;
		while (currentPathLength && currentPath[ currentPathLength-1 ] != (unsigned char)'/')
		{
			currentPath[ currentPathLength-1 ] = 0;
			currentPathLength--;
		}
	}
	else
	{
		if((disk_ptr->DirEnt.longName[0] != (unsigned char)'.'))
		{

			j=strlen( currentPath );
			i=0;
			c = disk_ptr->DirEnt.longName[i];
			while(( c >= (32+1) ) && (c <= 127))
			{
				currentPath[j+i]=c;
				i++;
				c = disk_ptr->DirEnt.longName[i];
			}
			currentPath[j+i]='/';
			currentPath[j+i+1]=0;
		}
	}

	displayFolder();
	selectorpos=0;

	fl_list_opendir(currentPath, &file_list_status);
	for(i=0;i<FL_STATUSTABLEN;i++)
	{
		backup_flstatus(i);
	}
 	clear_list();
	read_entry=1;
}



int main(int argc, char* argv[])
{
	unsigned char i,ret;
	char key;
	char numFiles;
	unsigned char page_number,filtermode,displayentry,last_file;
	char endOfPage;
	disk_in_drive * disk_ptr;

	init_display();
	/*Initialise media*/
	hxc_printf_box(mess_init);

	if(media_init())
	{
		// Initialise File IO Library
		fl_init();

		hxc_printf_box("Mounting SDCard...");

		// Attach media access functions to library
		if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
		{
			hxc_printf_box("ERROR: Can not attach media!");
		}
		hxc_printf_box("Reading HXCSDFE.CFG...");

		ret=0;

		read_cfg_file(sdfecfg_file);

		memset(currentPath,0,256);
		currentPath[0]='/';

		displayFolder();

		slotnumber=1;
		printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber]) ;

		clear_list();

		i=0;
		do{
			init_key(i);
			i++;
		}while(i);

		read_entry=0;
		selectorpos=0;
		page_number=0;
		last_file=0;
		filtermode=0;

		fl_list_opendir(currentPath, &file_list_status);
		for(i=0;i<FL_STATUSTABLEN;i++) backup_flstatus(i);

		for(;;)
		{
			y_pos=FILELIST_Y_POS;

			for(;;)
			{
				i=0;
				do
				{
					memset(&DirectoryEntry_tab[i],0,sizeof(DirectoryEntry));
					i++;
				}while((i<NUMBER_OF_FILE_ON_DISPLAY));

				i=0;
				y_pos=FILELIST_Y_POS;
				numFiles = 0;
				last_file=0;
				do
				{
					displayentry=0xFF;
					if(fl_list_readdir(&file_list_status, &dir_entry))
					{
						if(filtermode)
						{
							strlwr(dir_entry.filename);

							if(!strstr(dir_entry.filename,filter))
							{
								displayentry=0x00;
							}
						}

						if(displayentry)
						{

							memcpy(DirectoryEntry_tab[i].longName,dir_entry.filename,16);
							DirectoryEntry_tab[i].longName[16]=0;

							//memcpy(DirectoryEntry_tab[i].name,dir_entry.filename,12);
							//DirectoryEntry_tab[i].name[12-1]=0;

							dir_entry.filename[74]=0;
							if(dir_entry.is_dir)
							{
								hxc_printf(0,y_pos,"<DIR> %s",dir_entry.filename);
								DirectoryEntry_tab[i].attributes=0x10;
							}
							else
							{
								hxc_printf(0,y_pos,"      %s",dir_entry.filename);
								DirectoryEntry_tab[i].attributes=0x00;
							}

							y_pos++;
							numFiles++;
						
							DirectoryEntry_tab[i].firstCluster = ENDIAN_32BIT(dir_entry.cluster) ;
							DirectoryEntry_tab[i].size =  ENDIAN_32BIT(dir_entry.size);

							i++;
						}
					}
					else
					{
						last_file=0xFF;
						i=NUMBER_OF_FILE_ON_DISPLAY;
					}

				}while(i<NUMBER_OF_FILE_ON_DISPLAY);

				backup_flstatus(page_number+1);

				if ( selectorpos > (numFiles-1) )
				{
					selectorpos = numFiles-1;					
					if ( selectorpos < 0 )
					{
						selectorpos = 0;
					}
				}

				//hxc_fastprintf(0,FILELIST_Y_POS+(selectorpos),">");
				invert_line(FILELIST_Y_POS+(selectorpos));

				read_entry=0;
				do
				{
					key=wait_key();
					if(1)
					{
						//hxc_printf(0,0,"%.8X",key);

						switch(key)
						{
						case 0: // UP
							if ( ((selectorpos > 0)&&(page_number==0))||
								(page_number))
							{
								invert_line(FILELIST_Y_POS+(selectorpos));
								//hxc_fastprintf(0,FILELIST_Y_POS+(selectorpos)," ");

								selectorpos--;
								if(selectorpos<0)
								{
									clear_list();
									selectorpos=NUMBER_OF_FILE_ON_DISPLAY-1;
									page_number--;
									clear_list();
									read_entry=1;
									restore_flstatus(page_number);
								}
								else
								{
									//hxc_fastprintf(0,FILELIST_Y_POS+(selectorpos),">");
									invert_line(FILELIST_Y_POS+(selectorpos));
								}
							}
							break;
						case 2: // Down
							if ( (selectorpos < (numFiles-1)) || ((selectorpos>=(NUMBER_OF_FILE_ON_DISPLAY-1))) )
							{
								invert_line(FILELIST_Y_POS+(selectorpos));
								//hxc_fastprintf(0,FILELIST_Y_POS+(selectorpos)," ");
								selectorpos++;
								if(selectorpos>=NUMBER_OF_FILE_ON_DISPLAY)
								{
									if(last_file)
									{
										invert_line(FILELIST_Y_POS+(selectorpos));
									}
									else
									{
										selectorpos=0;
										clear_list();
										read_entry=1;
										page_number++;
										restore_flstatus(page_number);
									}
								}
								else
								{
									//hxc_fastprintf(0,FILELIST_Y_POS+(selectorpos),">");
									invert_line(FILELIST_Y_POS+(selectorpos));
								}
							}

							break;
						case 1: // Right
							if(!last_file)
							{
								clear_list();
								read_entry=1;
								page_number++;
								restore_flstatus(page_number);
							}
							break;

						case 7: // .
							if(page_number!=0) 
							{
								selectorpos=0;
								page_number=0;
								restore_flstatus(page_number);
								clear_list();
								read_entry=1;
							}
							break;

						case 8: // left
							if(page_number!=0) 
							{
								page_number--;
								restore_flstatus(page_number);
								clear_list();
								read_entry=1;
							}
							break;

						case 14:
							next_slot();
							break;

						case 15:
							disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

							if(disk_ptr->DirEnt.attributes&0x10)
							{
								enter_sub_dir(disk_ptr);
							}
							else
							{
								memcpy((void*)&disks_slot_a[slotnumber],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
								printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber]) ;
							}
							break;

						case 47:
						case 18:
						case 6:
							disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

							if(disk_ptr->DirEnt.attributes&0x10)
							{
								enter_sub_dir(disk_ptr);
							}
							else
							{
								memcpy((void*)&disks_slot_a[slotnumber],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
								next_slot();
							}
							break;

						case 60: // s key
							clear_list();
							clear_line(SLOT_Y_POS);
							clear_line(PATH_Y_POS);
							clear_line(PATH_Y_POS+1);
							show_all_slots();
							displayFolder();
							clear_line(PATH_Y_POS+1);
							clear_list();
							read_entry=1;
							restore_flstatus(page_number);
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber]) ;
							break;

						case 44: // h key
							clear_list();
							clear_line(SLOT_Y_POS);
							clear_line(PATH_Y_POS);
							clear_line(PATH_Y_POS+1);
							show_help();
							displayFolder();
							clear_line(PATH_Y_POS+1);
							clear_list();
							read_entry=1;
							restore_flstatus(page_number);
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber]) ;
							break;

						case 54: // p key
							clear_list();
							clear_line(SLOT_Y_POS);
							clear_line(PATH_Y_POS);
							clear_line(PATH_Y_POS+1);
							show_parameters();
							displayFolder();
							clear_line(PATH_Y_POS+1);
							clear_list();
							restore_flstatus(page_number);
							read_entry=1;
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber]) ;
							break;

						case 16:
						case 13:
							memset((void*)&disks_slot_a[slotnumber],0,sizeof(disk_in_drive));
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber]) ;
							break;

						case 79:
							memset((void*)&disks_slot_a[slotnumber],0,sizeof(disk_in_drive));
							next_slot();
							break;
						case 3:
							hxc_printf_box("Writing info...");
							save_cfg_file(sdfecfg_file);
							hxc_printf_box((char*)&mess_reboot);
							move_to_track(0);
							reboot();
							break;

						case 66:
						case 11:
							hxc_printf_box((char*)&mess_reboot);
							move_to_track(0);
							reboot();

							break;

						default:
							//printf("err %d!\n",key);
							break;
						}
					}
				}while(!read_entry);
                        }
                }
        }

        for(;;);
}

