
typedef struct cfgfile_
{
	char signature[16]; //"HXCFECFGV1.0"
	unsigned char step_sound;     //0x00 -> off 0xFF->on
	unsigned char ihm_sound;     //0x00 -> off 0xFF->on
	unsigned char back_light_tmr; //0x00 always off, 0xFF always on, other -> on x second
	unsigned char standby_tmr;    //0xFF disable, other -> on x second
	unsigned char disable_drive_select;
	unsigned char buzzer_duty_cycle;
	unsigned char number_of_slot;
	unsigned char slot_index;
	unsigned short update_cnt;

}cfgfile;

//unsigned char get_device_parameters(struct DirectoryEntry *pDirEnt);
//void update_cfgfile_parameters(struct DirectoryEntry *pDirEnt,unsigned char slot_index);



struct ShortDirectoryEntry {
	unsigned char name[12];
	unsigned char attributes;
	unsigned long firstCluster;
	unsigned long size;
	unsigned char longName[17];	// boolean
};

extern struct DirectoryEntry directoryEntry;


typedef struct disk_in_drive_
{
	struct ShortDirectoryEntry DirEnt;
	unsigned char numberoftrack;
	unsigned char numberofside;
	unsigned short rpm;
	unsigned short bitrate;
	unsigned short tracklistoffset;
}disk_in_drive;

