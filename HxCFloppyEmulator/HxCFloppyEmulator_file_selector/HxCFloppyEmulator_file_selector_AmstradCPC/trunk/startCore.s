
	org &6200
	
	LD      HL,(#BE7D)              ; Adresses variables Amsdos
    LD      A,(HL)                  ; Lecteur courant
    PUSH    AF                      ; Sauvegarde lecteur courant
    LD      C,7
    LD      DE,#40
    LD      HL,#ABFF
    CALL    #BCCE                   ; Initalisation ROM DISC
    POP     AF
    LD      HL,(#BE7D)
    LD      (HL),A 
    
	jp codeStart
	
hxcPackedData
	INCBIN "hxc.prg.pck"
hxcPackedData_end
	
codeStart:	
	
	di
	ex af, af'
	push af
	ex af, af'	
	exx
	push hl
	push de
	push bc
	exx	
	ld hl, hxcPackedData
	ld de, &100
	call unpack	
	exx
	pop bc
	pop de
	pop hl
	exx	
	ex af, af'	
	pop af
	ex af, af'	
	ei
	 
	
	ld a, 2
	call #bc0e
	
   ld   b,13
   ld   c,b
   call #BC38 ; SCR SET BORDER
   
   xor a
   ld   b,13
   ld   c,b
   call #BC32 ; background
   
   ld a, 1
   ld   b,0
   ld   c,b
   call #BC32 ; background
   
   jp &100
	

; HL = source
; DE = destination
unpack:
	inc hl;
	inc hl;
	inc hl;
	inc hl;

	ld a, 128;

	exx;
	ld de, 1;
	exx;

depack_loop
	add a, a;
	jr nz, getbit1;

	ld a, (hl);
	inc hl;
	rla;

getbit1
	jr c, output_compressed;
	ldi;

	add a, a;
	jr nz, getbit0;

	ld a, (hl);
	inc hl;
	rla;

getbit0
 jr c, output_compressed;
	ldi;

	add a, a;
	jr nz, getbit2;

	ld a, (hl);
	inc hl;
	rla;

getbit2
	jr c,output_compressed;
	ldi;

	add a, a;
	jr nz, getbit3;

	ld a, (hl);
	inc hl;
	rla;

getbit3
	jr c, output_compressed;
	ldi;
	jr depack_loop;

output_compressed
	ld c, (hl);
	inc hl;

output_match
	ld b, 0;
	bit 7, c;
	jr z, output_match1;

	add a, a;
	jr nz, getbit4;

	ld a, (hl);
	inc hl;
	rla;

getbit4
	rl b;

	add a, a;
	jr nz, getbit5;

	ld a, (hl);
	inc hl;
	rla;

getbit5
	rl b;

	add a, a;
	jr nz, getbit6;

	ld a, (hl);
	inc hl;
	rla;

getbit6
	rl b;

	add a, a;
	jr nz, getbit7;

	ld a, (hl);
	inc hl;
	rla;

getbit7 
	jr c, output_match1;
	res 7, c;

output_match1
	inc bc;

	exx;
	ld h, d;
	ld l, e;
	ld b, e;

get_gamma_value_size
	exx;

	add a, a;
	jr nz, getbit8;

	ld a, (hl);
	inc hl;
	rla;

getbit8
	exx;
	jr nc, get_gamma_value_size_end;
	inc b;
	jr get_gamma_value_size;

get_gamma_value_bits
	exx;

	add a, a;
	jr nz, getbit9;

	ld a, (hl);
	inc hl;
	rla;

getbit9
	exx;
	adc hl, hl;

get_gamma_value_size_end
	djnz get_gamma_value_bits;

get_gamma_value_end
	inc hl;
	exx;

	ret c;

	push hl;

	exx;
	push hl;
	exx;

	ld h, d;
	ld l, e;
	sbc hl, bc;

	pop bc;

	ldir;

	pop hl;

	add a, a;
	jr nz, getbita;

	ld a, (hl);
	inc hl;
	rla;

getbita
	jr c, output_compressed;
	ldi;

	add a, a;
	jr nz, getbitb;

	ld a, (hl);
	inc hl;
	rla;

getbitb
	jr c, output_compressed;
	ldi;
	JP depack_loop;
