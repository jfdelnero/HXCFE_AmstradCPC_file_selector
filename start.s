
	org &4000
	
codeStart:
	ld bc, &bc06
	out (c), c
	inc b
	xor a
	out (c), a

	LD      HL,(#BE7D)              ; Adresses variables Amsdos
    LD      A,(HL)                  ; Lecteur courant
    PUSH    AF                      ; Sauvegarde lecteur courant
    LD      C,7
    LD      DE,#40
    LD      HL,#ABFF
    CALL    #BCCE                   ; Initalisation ROM DISC
    POP     AF
    LD      HL,(#BE7D)
    LD      (HL),A                  ; Restore lecteur courant
 
	ld hl, coreData
	ld de, &c000
	ld bc, coreData_end - coreData
	ldir
	
	jp &c000
	
coreData
	INCBIN "startCore.bin"
coreData_end
 
END codeStart