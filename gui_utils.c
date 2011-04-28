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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui_utils.h"
#include "cpcbioscall.h"

extern char temp_buffer[257];

const char *helpText[] =
{
	"Keys:",
	"",
	"  S ....................... Show complete slot list",
	"  B ....................... Buzzer settings",
	"  Up/Down/Left/Right ...... Navigate",
	"  '.' ..................... Go to first page",
	"  F1 ...................... Clear slot",
	"  F2 ...................... Next slot",
	"  Space/Enter ............. Selected file into slot",
	"  ESC ..................... Cancel and Quit",
	"  F9 ...................... Save and Quit",
	"",
	"Credits:",
	"",
	"  Programming by Jean-Francois Del Nero (HxC2001)",
	"  and Arnaud Storq (http://www.norecess.net), 2010-2011",
	"  also using Rob's fatlib, exomizer and PDT's char output routine",
	"",
	"Visit http://hxc2001.free.fr/floppy_drive_emulator for updates",
	"",
	"Press a key..."
};
int helpTextCount = sizeof( helpText ) / sizeof( char * );

const char *szSoundVolumeUI[] =
{
	"Emits sound for UI operations ? (Y/N)"
};
int soundVolumeUICount = sizeof( szSoundVolumeUI ) / sizeof( char * );

const char *szSoundVolumeDiscAccess[] =
{
	"Emits sound for disc access ? (Y/N)"
};
int soundVolumeDiscAccessCount = sizeof( szSoundVolumeDiscAccess ) / sizeof( char * );

void init_display(void)
{
	hxc_fastprintf( 0, 0, "SDCard HxC Floppy Emulator Manager V2.2 - Firmware V0.0.0.0 - 'H' for help" );
//	hxc_fastprintf( 0, 1, "Press 'H' key for help !");
}

extern void clear_list();

char showPage( char **p, int size )
{
	char i;

	clear_list();
	clear_line(SLOT_Y_POS);
	clear_line(PATH_Y_POS);
	clear_line(PATH_Y_POS+1);

	for ( i = 0; i < size; i++ )
	{
		hxc_fastprintf( 0, 2 + i, p[ i ] );
	}

	i = wait_key2();

	clear_line(24);
	clear_line(23);
	
	return i;
}

void show_help(void)
{
	showPage(helpText, helpTextCount);
}

extern unsigned char g_step_sound;     //0x00 -> off 0xFF->on
extern unsigned char g_ihm_sound;     //0x00 -> off 0xFF->on
extern unsigned char g_back_light_tmr; //0x00 always off, 0xFF always on, other -> on x second
extern unsigned char g_standby_tmr;    //0xFF disable, other -> on x second
extern unsigned char g_buzzer_duty_cycle;
extern unsigned char g_buzzer_step_duration;  // 0xD8 <> 0xFF

void show_parameters(void)
{
	g_buzzer_duty_cycle = 0x20;

	switch ( showPage(szSoundVolumeUI, soundVolumeUICount) )
	{
	case 'N': // no
	case 46:
		g_ihm_sound = 0x00;
		break;
	default:
	case 'Y': // yes
	case 43:
		g_ihm_sound = 0xFF;
		break;
	}

	switch ( showPage(szSoundVolumeDiscAccess, soundVolumeDiscAccessCount) )
	{
	case 'N': // no
	case 46:
		g_step_sound = 0x00;
		break;
	default:
	case 43:
	case 'Y': // yes
		g_step_sound = 0xFF;
		break;
	}
}

void hxc_fastprintf(unsigned char x_pos,unsigned char y_pos, char *string)
{
	fastPrintString( 0xC000 + (y_pos*80) + x_pos, string );
}

void hxc_printf( unsigned char x_pos, unsigned char y_pos, char *chaine, ...)
{
      va_list marker;
      va_start( marker, chaine );

      vsprintf(temp_buffer,chaine,marker);
      hxc_fastprintf(x_pos, y_pos, temp_buffer);

      va_end( marker );
}

void hxc_printf_box(char * chaine, ...)
{
      va_list marker;
      va_start( marker, chaine );

      vsprintf(temp_buffer,chaine,marker);
      clear_line(MESSAGE_Y_POS);
	  hxc_fastprintf(0,MESSAGE_Y_POS,temp_buffer);

      va_end( marker );
}

void hxc_printf_box_error(char * chaine, ...)
{
      va_list marker;
      va_start( marker, chaine );

      vsprintf(temp_buffer,chaine,marker);
      clear_line(MESSAGE_Y_POS);
	  hxc_fastprintf(0,MESSAGE_Y_POS, temp_buffer);

		for(;;)
		{
			unsigned char i;
			for(i=0;i<50;i++)
			{
__asm
	call #0xbd19
__endasm;
			}

			invert_line(MESSAGE_Y_POS);
		}
}

