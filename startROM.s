
	nolist

	write "HXC20.ROM"

	db 1 ; rom type = background rom
	db 1, 1, 0 ; version 1.00

	dw names
	jp rom_init
	jp command_hxc

names:
	db "Hx", 'C'+&80
	db "HX", 'C'+&80
	db 0

rom_init:
	push hl
	push de

	ld hl, text_rom
	call displayText
	   
	pop de
	pop hl

	scf
	ret

displayText:
	ld a, (hl)
	inc hl
	or a
	ret z
	call &bb5a
	jp displayText

text_rom:
	db " HxC Floppy Emulator Manager V2.2", 10, 13
	db 10, 13
	db 0

command_hxc:	
ENDIF
