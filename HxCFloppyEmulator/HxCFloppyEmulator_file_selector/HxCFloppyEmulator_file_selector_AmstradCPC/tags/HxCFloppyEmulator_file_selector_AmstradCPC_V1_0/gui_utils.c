#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpcbioscall.h"
#include "graphx/data_bmp_font8x8_bmp.h"

unsigned char * screen_buffer;
char temp_buffer[257];

void initpal()
{

}

unsigned short calc_ptr(unsigned char y_pos)
{
  return ( (y_pos>>3) * 0x50) + (0x800 * (y_pos & 0x7));
}



void print_char8x8(bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
  unsigned short k;
  unsigned char *ptr_src,j;
  unsigned short yoff;

  ptr_src=(unsigned char*)&font->data[0];
  x=x>>3;
  k=((c>>4)<<7)+(c&0xF);
  ptr_src=ptr_src+k;
  j=8;
  while(j)
  {
    yoff=calc_ptr(y)+x;
    screen_buffer[yoff]  =*ptr_src;
    ptr_src=ptr_src + (16);
    y++;
    j--;
  }

}


void print_str(char * buf,unsigned short x_pos,unsigned short y_pos)
{
  unsigned char i;
  i=0;

      while(buf[i])
      {
        if(x_pos<=(640-8))
        {
          print_char8x8(&bitmap_font8x8_bmp,x_pos,y_pos,buf[i]);
          x_pos=x_pos+8;
        }
        else
        {
         break;
        }
       i++;
     }

}

int hxc_printf(unsigned char mode,unsigned short x_pos,unsigned short y_pos,char * chaine, ...)
{

      va_list marker;
      va_start( marker, chaine );

      vsprintf(temp_buffer,chaine,marker);
      switch(mode)
      {
        case 0:
        print_str(temp_buffer,x_pos,y_pos);
        break;
        case 1:
        print_str(temp_buffer,(640-(strlen(temp_buffer)*8))/2,y_pos);
        break;
      }

      va_end( marker );

      return 0;
}

void h_line(unsigned short y_pos,unsigned char val)
{
  unsigned short yoff;
  unsigned short i;
  yoff=calc_ptr(y_pos);

  for(i=0;i<(640/8);i++)
  {
   screen_buffer[yoff+i]=val;
  }

}

void clear_line(unsigned short y_pos,unsigned short val)
{
  unsigned char i;
 for(i=0;i<8;i++)
 h_line(y_pos+i,val);

}


void invert_line(unsigned short y_pos)
{
  unsigned char i,j;
  unsigned char *ptr_dst;
  unsigned short ptroffset;

 for(j=0;j<8;j++)
 {
  ptr_dst=(unsigned char*)screen_buffer;
  ptroffset=calc_ptr(y_pos+j);

  for(i=0;i<80;i++)
  {
     ptr_dst[ptroffset]=ptr_dst[ptroffset]^0xFF;
     ptroffset++;
  }
 }

}

void restore_box()
{

}
void hxc_printf_box(char * chaine, ...)
{
      int str_size;
      unsigned short i;
      #define BOX_YPOS 60
      va_list marker;
      va_start( marker, chaine );

      vsprintf(temp_buffer,chaine,marker);

      str_size=strlen(temp_buffer) * 8;
      str_size=str_size+(4*8);

      for(i=0;i< str_size;i=i+8)
      {
        print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2)+i,BOX_YPOS-8,8);
      }
      print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2)+(i-8),BOX_YPOS-8,3);
      print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2),BOX_YPOS-8,2);

      for(i=0;i< str_size;i=i+8)
      {
        print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2)+i,BOX_YPOS,' ');
      }

      print_str(temp_buffer,((640-str_size)/2)+(2*8),BOX_YPOS);
      print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2)+(i-8),BOX_YPOS,7);
      print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2),BOX_YPOS,6);

      for(i=0;i< str_size;i=i+8)
      {
        print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2)+i,BOX_YPOS+8,9);
      }
      print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2)+(i-8),BOX_YPOS+8,5);
      print_char8x8(&bitmap_font8x8_bmp,((640-str_size)/2),BOX_YPOS+8,4);

      va_end( marker );

}

void init_display(void)
{
        screen_buffer=0xC000;//(unsigned char*)((init_screen())<<8);
        hxc_printf(0,0,200-(8*1),"Ver 1.3a");
}
