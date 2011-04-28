
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


