; FILE: cpcbioscall.s

;-------------------------------------------------------------------------------------------

;void putchar(unsigned char c);
.globl _putchar

_putchar::
        ld      hl,#2
        add     hl,sp

        ld      a,(hl)
        call	0xBB5A
        ret

;-------------------------------------------------------------------------------------------
;void myfunc(int a, unsigned char b);
;Just an example on how parameter passing works – does not do anything to the values!
.globl _myfunc

_myfunc::
        ld      hl,#2
        add     hl,sp
        ld      e,(hl)

        ld      hl,#3
        add     hl,sp
        ld      d,(hl)

        ld      hl,#4
        add     hl,sp
        ld      a,(hl)
; Do something with DE(=a) and A(=b)
;
; etc. etc.
        ret

;-------------------------------------------------------------------------------------------
;char _read_sector(unsigned char * buffer,unsigned char drive,unsigned char track,unsigned char sector);

_read_sector::

        ld a,#01
        ld (0xbe66),a ; inhibe les erreurs
        ld (0xbe78),a ;       1 seul essai
        ld      c,#07
        call    0xb90f   ;ROM7 disquette

        ld      hl,#4
        add     hl,sp
        ld      e,(hl)   ;drive number

        ld      hl,#5
        add     hl,sp
        ld      d,(hl)   ;track number

        ld      hl,#6
        add     hl,sp
        ld      c,(hl)   ; sector number

        ld      hl,#2
        add     hl,sp
        ld      a,(hl)

        ld      hl,#3
        add     hl,sp
        ld      h,(hl)   ; PTR
        ld      l,a

        call  0xC03C
        ld l,a

        call    0xb903   ; disable upper ROM

        ret

;-------------------------------------------------------------------------------------------
;char _write_sector(unsigned char * buffer,unsigned char drive,unsigned char track,unsigned char sector);

_write_sector::

        ld a,#01
        ld (0xbe66),a ; inhibe les erreurs
        ld (0xbe78),a ;       1 seul essai
        ld      c,#07
        call    0xb90f   ;ROM7 disquette

        ld      hl,#4
        add     hl,sp
        ld      e,(hl)   ;drive number

        ld      hl,#5
        add     hl,sp
        ld      d,(hl)   ;track number

        ld      hl,#6
        add     hl,sp
        ld      c,(hl)   ; sector number

        ld      hl,#2
        add     hl,sp
        ld      a,(hl)

        ld      hl,#3
        add     hl,sp
        ld      h,(hl)   ; PTR
        ld      l,a

        call  0xC03F
        ld l,a

        call    0xb903   ; disable upper ROM

        ret

;-------------------------------------------------------------------------------------------
;char _read_file(unsigned char * buffer,unsigned char * dest_buffer,unsigned char filenamelen);

_read_file::

        ld      a,#0
        ld      hl, (0xBE7D)
        ld      (hl), a

        ld      hl,#4
        add     hl,sp
        ld      e,(hl)

        ld      hl,#5
        add     hl,sp
        ld      d,(hl)   ; dest PTR

        ld      hl,#6
        add     hl,sp
        ld      b,(hl)   ; filenamelen

        ld      hl,#2
        add     hl,sp
        ld      a,(hl)

        ld      hl,#3
        add     hl,sp
        ld      h,(hl)   ; file name PTR
        ld      l,a
        call 0xBC77
        jr nc,error

        ld      hl,#4
        add     hl,sp
        ld      a,(hl)

        ld      hl,#5
        add     hl,sp
        ld      h,(hl)   ; dest PTR
        ld      l,a
        call 0xBC83

        call 0xBC7A
        ld l,a

        ret
error:
        ld      l,a
        ret

;-------------------------------------------------------------------------------------------
;void _init_fpamsdos;

_init_fpamsdos::
        ld hl, ( #0xbe7d )
        ld a, ( hl )
        push hl
        push af
        ld hl, #0xabff
        ld de, #0x0040
        ld c, #0x07
        call #0xbccd
        pop af
        pop hl
        ld ( hl ),a
         ret

;-------------------------------------------------------------------------------------------
;void _cfg_disk_drive(unsigned char * buffer);

_cfg_disk_drive::


        ld      c,#07
        call    0xb90f   ;ROM7 disquette

        ld      hl,#2
        add     hl,sp
        ld      a,(hl)

        ld      hl,#3
        add     hl,sp
        ld      h,(hl)   ; cfg buffer PTR
        ld      l,a
        call 0xC036
       
        DI
        ld bc,#0xFB7E
        ld a,#0x03
        call putfdc
        ld a,#0xF1
        call putfdc
        ld a,#0x03
        call putfdc

        EI
        
        call    0xb903   ; disable upper ROM
        ret


putfdc:
        push af
lieta:
        in a,(c)
        rla
        jr nc,lieta
sors: 
        inc c
        pop af
        out (c),a
        dec  c
        ret

;-------------------------------------------------------------------------------------------
;void _move_to_track(unsigned char track);

_move_to_track::

        ld      c,#07
        call    0xb90f   ;ROM7 disquette


        LD HL,(#0xBE7D)
        LD E,(HL) ; lecteur courant dans A (0 ou 1)

        ld      hl,#2
        add     hl,sp
        ld      d,(hl)
        call 0xC045

        call    0xb903   ; disable upper ROM
        ret
;-------------------------------------------------------------------------------------------
;char _wait_key();

_wait_key::
        call  0xBB18
        ld l,a

        ret

;char _reboot();

_reboot::
        jp 0x0
        ret

;-------------------------------------------------------------------------------------------
;void _init_key(unsigned char c);

_init_key::
        ld      hl,#2
        add     hl,sp
        ld      a,(hl)
        ld b,a
        call  0xBB27
        ret

;-------------------------------------------------------------------------------------------

;unsigned char init_screen(void);
.globl _init_screen

_init_screen::
       ld   a,#2
       call 0xBC0E
       ld   a,#0
       call 0xBC59 ; SCR ACCESS
       call 0xBC14 ; SCR CLEAR
       ld   b,#0
       ld   c,#0
       call 0xBC38 ; SCR SET BORDER

       ld bc,#0x7F00
       ld a,#0
       out (c),a
       call 0xBC0B ; SCR GET LOCATION
       ld l,a
       ret


