;; File: crt0.s
;; Generic crt0.s for a Z80
;; From SDCC..
;; Modified to suit execution on the Amstrad CPC!
;; by H. Hansen 2003

    .module crt0
	.globl	_main

	.area _HEADER (ABS)

	.org 	0x100 ;; Start from address &100

;; &0000-&0040 is used by low kernel jumpblock
;; Stack is already setup by CPC firmware.

init:
	call gsinit
        
	call #0xb903
       
    call _main
    
    jp _exit
    
    ;ld hl, #0xbf04
    ;ld a, (hl)
    ;or a
    ;ret z
    
    ;ld a, #2
    ;call #0xbc0e
    
	;ld hl, #0xbf03
	;ld a, (hl)
	;call #0xbc0e
	
       ;ld hl, #0xbf00
	;	ld a, (hl)
;		ld   b,a
;       ld   c,b
 ;      call #0xBC38 ; SCR SET BORDER
       
  ;     ld hl, #0xbf01
  ;     ld a, (hl)
;		ld b, a		
;       ld   c,b
;		xor a
 ;      call #0xBC32 ; background
       
  ;     ld hl, #0xbf02
;		ld a, (hl)
;		ld b, a	
 ;      ld   c,b
  ;     ld a, #1
   ;    call #0xBC32 ; background
	
	ret

	.area	_HOME
	.area	_CODE
    .area   _GSINIT
    .area   _GSFINAL
        
	.area	_DATA
    .area   _BSS
    .area   _HEAP

   .area   _CODE
__clock::
	ret
	
_exit::
	ret
	
	.area   _GSINIT
gsinit::	

    .area   _GSFINAL
    ret
