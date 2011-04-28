; FILE: cpcbioscall.s

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

;char _wait_key2();

_wait_key2::
        call  0xBB06
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
;void fastPrintChar(unsigned char *screenBuffer, unsigned char c );
.globl _fastPrintChar
_fastPrintChar::
	;.db 0xed, 0xff
	
	pop bc
	pop de
	pop hl
	
	push hl
	push de
	push bc
	
	ld ix, #charText
	ld ( ix ), l
	
	.db #0xc3, #0x00, #0x9A ; jp &9A00
	
charText::
	.db #0, #0
	
;-------------------------------------------------------------------------------------------
;void fastPrintString(unsigned char *screenBuffer, unsigned char *string );
.globl _fastPrintString
_fastPrintString::
	;.db 0xed, 0xff
	
	pop bc
	pop de
	pop ix
	
	push ix
	push de
	push bc
	
	.db #0xc3, #0x00, #0x9A ; jp &9A00
	
;-------------------------------------------------------------------------------------------
;void clear_line(unsigned char y_pos);

_clear_line::

        ld      hl,#2
        add     hl,sp
        ld      a,(hl)
        
        ld hl, #0xc000
        ld bc, #0x0050
        
        or a
        
clearLineCalcYLoop:
        jp z, endClearLineCalcY
        add hl, bc
        dec a
        jp clearLineCalcYLoop
        
endClearLineCalcY:

		ld a, #8
clearLineDrawLoop:
		push af
		push hl
		ld d, h
		ld e, l
		inc de
		ld bc, #79
		ld ( hl ), #0
		ldir
		pop hl
		ld bc, #0x800
		add hl, bc
		pop af
		dec a
		jp nz, clearLineDrawLoop

        ret
        
;-------------------------------------------------------------------------------------------
;void invert_line(unsigned char y_pos);

_invert_line::

        ld      hl,#2
        add     hl,sp
        ld      a,(hl)
        
        ld hl, #0xc000
        ld bc, #0x0050
        
        or a
        
invertLineCalcYLoop:
        jp z, endInvertLineCalcY
        add hl, bc
        dec a
        jp invertLineCalcYLoop
        
endInvertLineCalcY:

		ld a, #8
invertLineDrawLoop:
		push af
		push hl
		ld c, #79
invertLineDrawLoopScanline:
		ld a, (hl)
		xor #255
		ld ( hl ), a
		inc hl
		dec c
		jp nz, invertLineDrawLoopScanline
		pop hl
		ld bc, #0x800
		add hl, bc
		pop af
		dec a
		jp nz, invertLineDrawLoop

        ret