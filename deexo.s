;; File: crt0.s
;; Generic crt0.s for a Z80
;; From SDCC..
;; Modified to suit execution on the Amstrad CPC!
;; by H. Hansen 2003

    .module crt0

	.area _HEADER (ABS)

	.org 	0x9000 ;; Start from address &100

;; &0000-&0040 is used by low kernel jumpblock
;; Stack is already setup by CPC firmware.

init:
;; screen init
       ld   a,#2
       call 0xBC0E
       ld   a,#0
       call 0xBC59 ; SCR ACCESS
       call 0xBC14 ; SCR CLEAR
       ld   b,#0
       ld   c,#0
       call 0xBC38 ; SCR SET BORDER

       di
       ld hl,#packet_data
       ld de,#0xC000
       call deexo

       EI

        ;; init floppy amsdos
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

        ld      bc,#0x7F00
        ld      a,#0xC1
        out     (c),a

;;        ld      a,#0
;;        ld      hl, (0xBE7D)
;;        ld      (hl), a

        ;;ld de, #0xC000
        ld de, #0x0100
        ld hl,#_filename
        ld b, #7
        call 0xBC77
        jr nc,error

        ld hl, #0x0100
        call 0xBC83

        call 0xBC7A


        
    ;;   di
    ;;   ld hl,#0xC000
    ;;   ld de,#0x100
    ;;   call deexo
    ;;   EI

        ld      bc,#0x7F00
        ld      a,#0xC0
        out     (c),a

        call #0x100

        ret
error:
loop:
     jp loop

deexo:
                ld	a,(hl)
		inc	hl
                .db 0xDD,0x67 ;;ld	(ix),a

		ld	b,#52
		ld	iy,#0x8000 ;;#exo_mapbasebits
		push	de
exo_initbits:	ld	a,b
		sub	#4
		and	#15
		jr	nz,exo_node1

		ld	de,#1		;;DE=b2
exo_node1:	ld	c,#16
exo_get4bits:	call	exo_getbit
		rl	c
		jr	nc,exo_get4bits
		ld	0(iy),c	;;bits[i]=b1

		push	hl
		inc	c
		ld	hl,#0
		scf
exo_setbit:	adc	hl,hl
		dec	c
		jr	nz,exo_setbit
		ld	52 (iy) ,e
		ld	104 (iy) ,d	;;base[i]=b2
		add	hl,de
		ex	de,hl
		inc	iy

		pop	hl
		djnz	exo_initbits
		pop	de

exo_mainloop:	ld	c,#1		;;lenght=1
		call	exo_getbit	;;literal?
		jr	c,exo_literalcopy
		ld	c,#255
exo_getindex:	inc	c
		call	exo_getbit
		jr	nc,exo_getindex
		ld	a,c		;;C=index
		cp	#16
		ret	z
		jr	c,exo_continue
		push	de
		ld	d,#16
		call	exo_getbits
		pop	de
exo_literalcopy:ldir
		jr	exo_mainloop
exo_continue:	push	de
		call	exo_getpair
		push	bc
		pop	af
		ex	af,af'		;;lenght in AF'
		ld	de,#512+48	;;2 bits, 48 offset
		dec	bc
		ld	a,b
		or	c
		jr	z,exo_goforit	;;1?
		ld	de,#1024+32	;;4 bits, 32 offset
		dec	bc
		ld	a,b
		or	c
		jr	z,exo_goforit	;;2?
		ld	e,#16		;;16 offset
exo_goforit:	call	exo_getbits
		ld	a,e
		add	a,c
		ld	c,a
		call	exo_getpair
		pop	de		;;bc=offset
		push	hl		;;de=destination
		ld	h,d
		ld	l,e
		sbc	hl,bc		;;hl=origin
		ex	af,af'
		push	af
		pop	bc		;;bc=lenght
		ldir
		pop	hl		;;Keep HL, DE is updated
		jr	exo_mainloop	;;Next!

exo_getpair:	ld	iy,#0x8000 ;;#exo_mapbasebits
		ld	b,#0
		add	iy,bc
		ld	d,0(iy)
		call	exo_getbits
		ld	a,c
		add	a,52(iy)
		ld	c,a
		ld	a,b
		adc	a,104(iy)
		ld	b,a
		ret

exo_getbits:	ld	bc,#0		;;get D bits in BC
exo_gettingbits:dec	d
		ret	m
		call	exo_getbit
		rl	c
		rl	b
		jr	exo_gettingbits

exo_getbit:
                .db 0xDD,0x7C  ;;; ld a,ixh  ;get one bit
		srl	a
                
                .db 0xDD,0x67 ;; ld ixh,a
		ret	nz
		ld	a,(hl)
		inc	hl
		rra

                .db 0xDD,0x67  ;;ld ixh,a

                ret
_filename:
	.ascii "HXC.PRG"
	.db 0x00

;;exo_mapbasebits::
;;	.ds 156 ;tables for bits, baseL, baseH

packet_data::

