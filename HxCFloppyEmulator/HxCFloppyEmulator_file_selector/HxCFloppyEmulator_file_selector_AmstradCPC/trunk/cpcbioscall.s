; FILE: cpcbioscall.s

;-------------------------------------------------------------------------------------------
;char _read_sector(unsigned char * buffer,unsigned char drive,unsigned char track,unsigned char sector);

_read_sector::

        ld a,#01
        ld (0xbe66),a ; inhibe les erreurs
        ld (0xbe78),a ;       1 seul essai
        ld      c,#07
        call    0xb90f   ;ROM7 disquette

	.db #0xed, #0xff
	
		LD HL,(#0xBE7D)
        LD E,(HL) ; lecteur courant dans A (0 ou 1)

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
		LD HL,(#0xBE7D)
        LD E,(HL) ; lecteur courant dans A (0 ou 1)

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

		ld a, #1
reboot1:
		push af
		call #0xBD19
		
		ld hl, #0x1000
		ld de, #0x1000
		ld bc, #0x200
		ldir
		
		pop af
		jr nz, reboot1

        call 0x0

;-------------------------------------------------------------------------------------------
;void _init_key(unsigned char c);

_init_key::
        ld      hl,#2
        add     hl,sp
        ld      a,(hl)
        ld b,a
        call  0xBB27
        ret
        
_init_all::

	;.db 0xed, 0xff
	call #0xB906 ; KL_L_ROM_ENABLE
	
	ld hl, #0x3800
	ld de, #0x9A00
	ld bc, #1016
	ldir	
	
	call #0xB909 ; KL_L_ROM_DISABLE
	
	ret

; IN: a = char to print
; IN: de = screen ptr
PrivatePrintChar:	
	ld l, a
	ld h, #0
	add hl, hl ; * 2
	add hl, hl ; * 4
	add hl, hl ; * 8
	ld bc, #0x9A00
	add hl, bc
	
	ld b, #8
	ld a, ( hl )
	ld ( de ), a
	inc hl
	set 3, d
	ld a, ( hl )
	ld ( de ), a
	inc hl
	ld a, d
	add a, b
	ld d, a
	ld a, ( hl )
	ld ( de ), a
	inc hl
	set 3, d
	ld a, ( hl )
	ld ( de ), a
	inc hl
	ld a, d
	add a, b
	ld d, a
	ld a, ( hl )
	ld ( de ), a
	inc hl
	set 3, d
	ld a, ( hl )
	ld ( de ), a
	inc hl
	ld a, d
	add a, b
	ld d, a
	ld a, ( hl )
	ld ( de ), a
	inc hl
	set 3, d
	ld a, ( hl )
	ld ( de ), a

	ret
	
;-------------------------------------------------------------------------------------------
;void fastPrintChar(unsigned char *screenBuffer, unsigned char c );
.globl _fastPrintChar
_fastPrintChar::
	pop bc
	pop de
	pop hl
	
	push hl
	push de
	push bc
	
	ld a, l
	call PrivatePrintChar	
	ret


;-------------------------------------------------------------------------------------------
;void fastPrintString(unsigned char *screenBuffer, unsigned char *string );
.globl _fastPrintString
_fastPrintString::
	pop bc
	pop de
	pop ix
	
	push ix
	push de
	push bc
	
	; ix = string
	; de = screenBuffer
	
	;.db 0xed, 0xff
	
fastPrintString_loop:
	ld a, ( ix )
	or a
	ret z
	inc ix
	push de
	call PrivatePrintChar
	pop de
	inc de
	jr fastPrintString_loop
	
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