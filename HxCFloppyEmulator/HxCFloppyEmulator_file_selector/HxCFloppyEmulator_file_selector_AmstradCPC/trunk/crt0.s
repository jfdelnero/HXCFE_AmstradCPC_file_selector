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

;; Initialise global variables
        call    gsinit
	
	
	ld a, #2
	call #0xbc0e
	
       ld   b,#1
       ld   c,b
       call #0xBC38 ; SCR SET BORDER
       
       ld a, #0
       ld   b,#1
       ld   c,b
       call #0xBC32 ; background
       
       ld a, #1
       ld   b,#20
       ld   c,b
       call #0xBC32 ; background
       
       
	ld bc, #0xbc06
	out (c), c
	inc b
	ld a, #25
	out (c), a
       
	call	_main
	jp	_exit

	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
    .area   _GSINIT
    .area   _GSFINAL
        
	.area	_DATA
    .area   _BSS
    .area   _HEAP

   .area   _CODE
__clock::
;;	ld		a,#2
;;  rst     0x08
	ret
	
_exit::
	ret
	
	.area   _GSINIT
gsinit::	

    .area   _GSFINAL
    ret
