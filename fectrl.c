

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <mint/osbind.h>
#include <time.h>
//#include <vt52.h>

#include "cpcbioscall.h"

#include "gui_utils.h"
#include "cfg_file.h"

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"
#define L_INDIAN(var) ((var&0x000000FF)<<24) |((var&0x0000FF00)<<8) |((var&0x00FF0000)>>8) |((var&0xFF000000)>>24)

#define NUMBER_OF_FILE_ON_DISPLAY 10
#define NUMBER_OF_SLOT 8

typedef struct direct_access_status_sector_
{
	char DAHEADERSIGNATURE[8];
	char FIRMWAREVERSION[12];
	unsigned long lba_base;
	unsigned char cmd_cnt;
	unsigned char read_cnt;
	unsigned char write_cnt;
	unsigned char last_cmd_status;
} direct_access_status_sector ;


typedef struct direct_access_cmd_sector_
{
	char DAHEADERSIGNATURE[8];
	unsigned char cmd_code;
	unsigned char parameter_0;
	unsigned char parameter_1;
	unsigned char parameter_2;
	unsigned char parameter_3;
	unsigned char parameter_4;
	unsigned char parameter_5;
	unsigned char parameter_6;
	unsigned char parameter_7;
	unsigned char cmd_checksum;
}direct_access_cmd_sector  ;

#define LFN_MAX_SIZE 128

typedef struct DirectoryEntry_ {
	unsigned char name[12];
	unsigned char attributes;
	unsigned long firstCluster;
	unsigned long size;
	unsigned char longName[LFN_MAX_SIZE];	// boolean
}DirectoryEntry;


unsigned char slotnumber;
unsigned char floppydrive;

unsigned char sector[512];
DirectoryEntry DirectoryEntry_tab[NUMBER_OF_FILE_ON_DISPLAY];
unsigned long browser_page_index_tab[255];
unsigned char filename[257];
disk_in_drive disks[2];
disk_in_drive disks_slot_a[NUMBER_OF_SLOT];
disk_in_drive disks_slot_b[NUMBER_OF_SLOT];

FAT32_ShortEntry sfEntry;
unsigned long last_setlbabase;
unsigned char sdfecfg_file[2048];
unsigned long cluster;

unsigned short   floppyselector,floppyselectorindex;
char selectorpos;

long  entryindex,entryindex2,startentryindex,lastentryindex;
unsigned char browser_page_index;
unsigned char read_entry;
unsigned char y_pos;

const char mess_reboot[]="    >>>>>Rebooting...<<<<<     ";

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


char setlbabase(unsigned long lba)
{
	char ret;
	unsigned char * ptr;
	direct_access_cmd_sector * dacs;

        dacs=(direct_access_cmd_sector  *)sector;

	sprintf(dacs->DAHEADERSIGNATURE,"HxCFEDA");
	dacs->cmd_code=1;

	ptr=(unsigned char*)&lba;
	dacs->parameter_0=ptr[0];
	dacs->parameter_1=ptr[1];
	dacs->parameter_2=ptr[2];
	dacs->parameter_3=ptr[3];
	dacs->parameter_4=0xA5;

	ret=Flopwr( sector, floppydrive, 0, 255, 1 );
	return ret;
}


int media_init()
{
	unsigned char ret;
	direct_access_status_sector * dass;
	FL_FILE *file;
        unsigned char ** ptr;
        unsigned char fpcfgbuffer[9];

        ptr=0xBE7D;

        floppydrive=**ptr;

        last_setlbabase=0xFFFFFF00;
        
        fpcfgbuffer[0]=0x23;
        fpcfgbuffer[1]=0x00;

        fpcfgbuffer[2]=0xFF;
        fpcfgbuffer[3]=0x03;

        fpcfgbuffer[4]=0xAF;
        fpcfgbuffer[5]=0x02;
        fpcfgbuffer[6]=0x0A;
        fpcfgbuffer[7]=0x01;
        fpcfgbuffer[8]=0x03;
        cfg_disk_drive((unsigned char *)&fpcfgbuffer);
        

	ret=Floprd((unsigned char*)&sector, floppydrive, 0, 255, 1 );

	if(!ret)
	{
		dass=(direct_access_status_sector *)sector;
		if(!strcmp(dass->DAHEADERSIGNATURE,"HxCFEDA"))
		{
                    hxc_printf(0,0,200-30,"Firmware %s" ,dass->FIRMWAREVERSION);
                    return 1;
		}

 	        hxc_printf_box("Bad signature - HxC Floppy Emulator not found!");

		return 0;
	}
	hxc_printf_box("ERROR: Floppy Access error!  [%d]",ret);

	return 0;
}


int media_read(unsigned long lba_sector, unsigned char *buffer)
{
	unsigned char ret;
	unsigned long diff;

	hxc_printf(0,8*79,0,"%c",23);

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
         hxc_printf(0,8*79,0," ");
         return 1;
        }
        else
        {
         hxc_printf_box("ERROR: Floppy Write Access error!  [%d:%d]",floppydrive,ret);
         for(;;);
        }
}

int media_write(unsigned long lba_sector, unsigned char *buffer)
{
	unsigned long diff;
	unsigned char ret;

	hxc_printf(0,8*79,0,"%c",23);

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
         hxc_printf(0,8*79,0," ");
         return 1;
        }
        else
        {
         hxc_printf_box("ERROR: Floppy Write Access error!  [%d]",ret);
         for(;;);
        }

}

void lockup()
{
	for(;;);
}

void printslotstatus(unsigned char slotnumber,  disk_in_drive * disks)
{
   char tmp_str[17];
   #define SLOT_Y_POS 100
    hxc_printf(0,0,SLOT_Y_POS,"Slot %.2d:", slotnumber);

    clear_line(SLOT_Y_POS+8,0);
    hxc_printf(0,0,SLOT_Y_POS+8,"Drive A:");
    if( disks[0].DirEnt.size)
    {
        memcpy(tmp_str,disks[0].DirEnt.longName,16);
        tmp_str[16]=0;
	hxc_printf(0,0,SLOT_Y_POS+8,"Drive A: %s", tmp_str);
     }

    clear_line(SLOT_Y_POS+16,0);
    hxc_printf(0,0,SLOT_Y_POS+16,"Drive B:");
    if(disks[1].DirEnt.size)
    {
        memcpy(tmp_str,disks[1].DirEnt.longName,16);
        tmp_str[16]=0;
	hxc_printf(0,0,SLOT_Y_POS+16,"Drive B: %s", tmp_str);
    }
}



char save_cfg_file(unsigned char * sdfecfg_file)
{
        unsigned short i,j,k;
	cfgfile * cfgfile_ptr;
	short   floppyselector,floppyselectorindex;
	disk_in_drive * disk_ptr;
	FL_FILE *file;

	cfgfile_ptr=(cfgfile * )sdfecfg_file;
	cfgfile_ptr->number_of_slot=1;
	cfgfile_ptr->slot_index=1;

        i=0;
        j=(512*2) + 128;

        do
	{
             floppyselector=i+1;
	     floppyselectorindex=(floppyselector<<7)+(2*512);
	     disk_ptr=(disk_in_drive *)&sdfecfg_file[floppyselectorindex];
	     if( disk_ptr->DirEnt.size)
	     {
		cfgfile_ptr->number_of_slot++;
		memcpy(&sdfecfg_file[j],&sdfecfg_file[floppyselectorindex],128);
		j=j+128;
	     }
	     i++;
	}while(i<(NUMBER_OF_SLOT-1));

        j=(512*2) + (cfgfile_ptr->number_of_slot*128);
        memset(&sdfecfg_file[j],0,2048-j);


        if(cfgfile_ptr->slot_index>=cfgfile_ptr->number_of_slot)
        {
           cfgfile_ptr->slot_index=cfgfile_ptr->number_of_slot-1;
        }


        file = fl_fopen("/HXCSDFE.CFG", "r");
	if (file)
	{
		// Write some data
		if (fl_fswrite((unsigned char*)sdfecfg_file, 2048/512, file) != 2048/512)
		{
                      	restore_box();
                        hxc_printf_box("ERROR: Write file failed !");
                        lockup();
                }
                
                 //print_hex(sdfecfg_file, 2048);

	}
	else
        {
                restore_box();
                hxc_printf_box("ERROR: Create file failed !");
                lockup();
        }
        // Close file
	fl_fclose(file);
}


void clear_list()
{
  unsigned char y_pos,i;
  
  y_pos=16;

 for(i=0;i<NUMBER_OF_FILE_ON_DISPLAY;i++)
 {
        clear_line(y_pos,0);
        y_pos=y_pos+8;
 }

}


void next_slot()
{

    slotnumber++;
    if(slotnumber>(NUMBER_OF_SLOT-1))   slotnumber=1;
    floppyselector=slotnumber;
    floppyselectorindex=(floppyselector<<7)+(2*512);
    memcpy((void*)&disks[0],(void*)&sdfecfg_file[floppyselectorindex],sizeof(disk_in_drive));
    memcpy((void*)&disks[1],(void*)&sdfecfg_file[(floppyselectorindex)+64],sizeof(disk_in_drive));

    printslotstatus(slotnumber, (disk_in_drive *) &disks) ;
}


void enter_sub_dir(unsigned long first_cluster)
{

 cluster=ENDIAN_32BIT(first_cluster);
 if(!cluster) cluster=get_root_cluster();
 selectorpos=0;
 startentryindex=-1;
 lastentryindex=-1;
 browser_page_index=0;
 entryindex2=-1;
 entryindex=0;
 clear_list();
 read_entry=1;
}


int main(int argc, char* argv[])
{

  unsigned char i,ret;
  FL_FILE *file;
  unsigned char key,entrytype;
  disk_in_drive * disk_ptr;


  init_display();
  /*Initialise media*/
  hxc_printf_box("Init SD HxC Floppy Emulator...");

  if(media_init())
  {
        	// Initialise File IO Library
		fl_init();

                hxc_printf_box("       Mounting SDCard...     ");

		// Attach media access functions to library
		if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
		{
                  for(;;);
                }
                hxc_printf_box("     Reading HXCSDFE.CFG ...  ");

                ret=0;
		file = fl_fopen("/HXCSDFE.CFG", "r");
		if (file)
		{
			if (fl_fread(sdfecfg_file, 1, 2048, file) != 2048)
			{
                              ret=2;
			}
			fl_fclose(file);
		}
		else
		{
                  ret=1;
		}

                if(ret)
                {
              		hxc_printf_box("ERROR: Access HXCSDFE.CFG file failed ! [%d]",ret);
			for(;;);
                }

                slotnumber=1;

		floppyselector=slotnumber;
		floppyselectorindex=(floppyselector<<7)+(2*512);

                memset((void*)&disks[0],0,sizeof(disk_in_drive));
                memset((void*)&disks[1],0,sizeof(disk_in_drive));

		memcpy((void*)&disks[0],(void*)&sdfecfg_file[floppyselectorindex],sizeof(disk_in_drive));
		memcpy((void*)&disks[1],(void*)&sdfecfg_file[(floppyselectorindex)+64],sizeof(disk_in_drive));

                printslotstatus(slotnumber, (disk_in_drive *) &disks) ;

                clear_list();

                i=0;
                do{
                init_key(i);
                i++;
                }while(i);

                read_entry=0;
		selectorpos=0;
		browser_page_index=0;
		cluster=get_root_cluster();
		for(;;)
		{
			entryindex2=-1;
			entryindex=0;
			y_pos=16;
			startentryindex=-1;
			lastentryindex=-1;
			browser_page_index=0;
			for(;;)
			{
       				entryindex2=startentryindex;
				i=0;
				y_pos=16;
				do
				{
					entryindex=entryindex2;
					entryindex2=fl_change_entry(cluster,1,entryindex,&sfEntry,filename);
                                        if(!i)
                                        {
                                          browser_page_index_tab[browser_page_index]= entryindex;
                                        }
					if(entryindex2!=entryindex)
					{

                                                entrytype=12;
                                                if(sfEntry.Attr&FILE_ATTR_DIRECTORY)   entrytype=10;

						hxc_printf(0,0,y_pos," %c%s",entrytype,filename);
						y_pos=y_pos+8;
						if(strlen(filename))
						memcpy(DirectoryEntry_tab[i].name,filename,11);
						else
						memcpy(DirectoryEntry_tab[i].name,sfEntry.Name,11);


						DirectoryEntry_tab[i].name[11]=0;
						sprintf(DirectoryEntry_tab[i].longName,filename);
						DirectoryEntry_tab[i].attributes=sfEntry.Attr;
						DirectoryEntry_tab[i].firstCluster =  ENDIAN_32BIT((((unsigned long)ENDIAN_16BIT(sfEntry.FstClusHI))<<16) + ENDIAN_16BIT(sfEntry.FstClusLO));
						DirectoryEntry_tab[i].size =  ENDIAN_32BIT(ENDIAN_32BIT(sfEntry.FileSize));
					}

					i++;
				}while((i<NUMBER_OF_FILE_ON_DISPLAY) && entryindex2!=entryindex);

				lastentryindex= entryindex2;
				hxc_printf(0,0,16+(selectorpos*8),">");
				invert_line(16+(selectorpos*8));

                               read_entry=0;
                                do
				{
                                        key=wait_key();
					if(1)
					{
						//hxc_printf(0,0,0,"%.8X",key);

                                        	switch(key)
						{
						case 0: // UP
							invert_line(16+(selectorpos*8));
							hxc_printf(0,0,16+(selectorpos*8)," ");

							selectorpos--;
							if(selectorpos<0)
                                                        {
                                                          selectorpos=NUMBER_OF_FILE_ON_DISPLAY-1;
                                                        if(browser_page_index) browser_page_index--;
							startentryindex=browser_page_index_tab[browser_page_index];//startentryindex-16;
							if(  startentryindex<-1) startentryindex=-1;
                                                        clear_list();

							read_entry=1;
                                                        }
                                                        else
                                                        {

							hxc_printf(0,0,16+(selectorpos*8),">");
							invert_line(16+(selectorpos*8));
                                                        }
							break;
						case 2: // Down
							invert_line(16+(selectorpos*8));
							hxc_printf(0,0,16+(selectorpos*8)," ");

							selectorpos++;
							if(selectorpos>=NUMBER_OF_FILE_ON_DISPLAY)
                                                        {
                                                          selectorpos=0;
                                                          if(browser_page_index<(sizeof(browser_page_index_tab)/4)) browser_page_index++;

    							  startentryindex=lastentryindex;//+1;
    							  if(  startentryindex<-1) startentryindex=-1;
                                                        clear_list();

							read_entry=1;
                                                        }
                                                        else
                                                        {
							hxc_printf(0,0,16+(selectorpos*8),">");
							invert_line(16+(selectorpos*8));
                                                        }

							break;
						case 5:
							startentryindex=-1;
							break;
						case 1: // Right
							startentryindex=lastentryindex;
                                                        if(browser_page_index<(sizeof(browser_page_index_tab)/4)) browser_page_index++;
							if(  startentryindex<-1) startentryindex=-1;
                                                        clear_list();
							read_entry=1;
							break;
						case 8:
						        if(browser_page_index) browser_page_index--;

							startentryindex=browser_page_index_tab[browser_page_index];
							if(  startentryindex<-1) startentryindex=-1;
                                                        clear_list();
							read_entry=1;
							break;


						case 14:
                                                        next_slot();
							break;

						case 10:
                                                	hxc_printf_box("Saving selection...");
                                                	save_cfg_file(sdfecfg_file);
                                                       	restore_box();
                                                        clear_list();
							read_entry=1;
							break;

						case 15:
                                                         disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

						        if(disk_ptr->DirEnt.attributes&0x10)
						        {
                                                             enter_sub_dir(disk_ptr->DirEnt.firstCluster);
                                                        }
                                                        else
                                                        {
							memcpy((void*)&sdfecfg_file[floppyselectorindex],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
							memcpy((void*)&disks[0],(void*)&sdfecfg_file[(floppyselectorindex)],sizeof(disk_in_drive));

                                		        printslotstatus(slotnumber, (disk_in_drive *) &disks) ;
                  		                      }
							break;

						 case 47:
                                                 case 18:
	                                         case 6:
                                                         disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

						        if(disk_ptr->DirEnt.attributes&0x10)
						        {
                                                            enter_sub_dir(disk_ptr->DirEnt.firstCluster);
                                                        }
                                                        else
                                                        {
  							     memcpy((void*)&sdfecfg_file[floppyselectorindex],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
                                                             memcpy((void*)&disks[0],(void*)&sdfecfg_file[(floppyselectorindex)],sizeof(disk_in_drive));
  
                                                             next_slot();
                  		                      }
							break;
						case 7:
                                                        disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

						        if(disk_ptr->DirEnt.attributes&0x10)
						        {
                                                          enter_sub_dir(disk_ptr->DirEnt.firstCluster);
                                                        }
                                                        else
                                                        {

							memcpy((void*)&sdfecfg_file[floppyselectorindex+64],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
							memcpy((void*)&disks[1],(void*)&sdfecfg_file[(floppyselectorindex)+64],sizeof(disk_in_drive));

                                		        printslotstatus(slotnumber, (disk_in_drive *) &disks) ;
                                		                      }
							break;
                                                case 16:
                                                case 13:
                                                      memset((void*)&sdfecfg_file[floppyselectorindex],0,128);
						      memset((void*)&disks,0,sizeof(disk_in_drive)*2);

                               		              printslotstatus(slotnumber, (disk_in_drive *) &disks) ;
                                                break;

                                                case 79:
                                                      memset((void*)&sdfecfg_file[floppyselectorindex],0,128);
						      memset((void*)&disks,0,sizeof(disk_in_drive)*2);

                                                      next_slot();
                                                break;
                                               	case 3:
                                                	hxc_printf_box("Saving selection and restart...");
                                                	save_cfg_file(sdfecfg_file);
                                                       	restore_box();
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
	return 0;
}

