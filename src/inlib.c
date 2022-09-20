#include "inlib.h"

// Disable unreferenced function argument warning caused
// by naked functions
#pragma disable_warning 85

/*
	Port $3F: I/O port control
	Bit	Function
	7	Port B TH pin output level (1=high, 0=low)
	6	Port B TR pin output level (1=high, 0=low)
	5	Port A TH pin output level (1=high, 0=low)
	4	Port A TR pin output level (1=high, 0=low)
	3	Port B TH pin direction (1=input, 0=output)
	2	Port B TR pin direction (1=input, 0=output)
	1	Port A TH pin direction (1=input, 0=output)
	0	Port A TR pin direction (1=input, 0=output)
*/

/* Port $DC: I/O port A and B
	Bit	Function
	7	Port B Down pin input
	6	Port B Up pin input
	5	Port A TR pin input
	4	Port A TL pin input
	3	Port A Right pin input
	2	Port A Left pin input
	1	Port A Down pin input
	0	Port A Up pin input

	(1= not pressed, 0= pressed)
*/

/*
Port $DD: I/O port B and miscellaneous
Bit	Function
	7	Port B TH pin input
	6	Port A TH pin input
	5	Cartridge slot CONT pin *
	4	Reset button *
	3	Port B TR pin input
	2	Port B TL pin input
	1	Port B Right pin input
	0	Port B Left pin input
*/


volatile struct inlibDevice inlib_portA;
volatile struct inlibDevice inlib_portB;
unsigned char inlib_port3F_last;


void inlib_getportA(void) __naked __z88dk_fastcall __preserves_regs(b,c,d,e,iyl,iyh)
{
	__asm

	in a,(#0xDC)     ; Get port A bits (up/down/left/right/tl/tr)
	and a, #0x3F     ; Keep only port A bits (bits 0-5)

	ld l, a          ; Return value in L
	ret

	__endasm;
}

void inlib_getportB(void) __naked __z88dk_fastcall __preserves_regs(b,c,d,e,iyl,iyh)
{
	__asm

	in a,(0xDD)      ; Get port B left, right, tl and tr bits (0-3)
	and a, #0x0F     ; Keep only port B bits
	ld l, a          ; Save this in L
	in a,(0xDC)      ; Read port B down up bits (6-7)
	and a, #0xC0     ; Keep only port B
	or a, l          ; Combine
	rlca             ; Rotate bits to get same aligments
	rlca             ; as when reading port A

	ld l, a          ; Return value in L
	ret

	__endasm;
}

void inlib_portA_TH_in_TR_in(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	or a, #0x03                 ; Set TR and TH to 1 (input)
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portB_TH_in_TR_in(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	or a, #0x0C                 ; Set TR and TH to 1 (input)
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portA_TH_out_TR_in(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0xFC                ; Zero two last bits
	or a, #0x01                 ; TR out, TH in
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portB_TH_out_TR_in(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0xF3
	or a, #0x04
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portA_TH_out_TR_out(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0xFC                ; Zero two last bits
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portB_TH_out_TR_out(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0xF3
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portA_TH_low(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0xDF                ; Clear bit 5
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portA_TH_high(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	or a, #0x20                 ; Set bit 5
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portB_TH_low(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0x7F                ; Clear bit 7
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portB_TH_high(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	or a, #0x80                 ; Set bit 7
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portA_TR_low(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0xEF                ; Clear bit 4
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portA_TR_high(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	or a, #0x10                 ; Set bit 4
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portB_TR_low(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	and a, #0xBF                ; Clear bit 6
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}

void inlib_portB_TR_high(void) __naked __z88dk_fastcall
{
	__asm
	ld a, (_inlib_port3F_last)  ; Get last written value
	or a, #0x40                 ; Set bit 6
	out (0x3F), a               ; Write to port
	ld (_inlib_port3F_last), a  ; Save last written value
	ret
	__endasm;
}


void inlib_pollSMS(unsigned char port) __naked __z88dk_fastcall
{
	__asm

	ld a, l
	or a
	jr nz, pollsms_portB

pollsms_portA:
	ld a, #INLIB_TYPE_SMS       ; set controller type
	ld (_inlib_portA), a

	ld hl, (#_inlib_portA + 1)  ; read current button bits
	ld (#_inlib_portA + 3), hl  ; copy to previous button bits

	call _inlib_getportA        ; get button bits in l
	ld h, #0                    ; button high byte is unused

	ld (#_inlib_portA + 1), hl  ; write new button state

	ret

pollsms_portB:
	ld a, #INLIB_TYPE_SMS       ; set controller type
	ld (_inlib_portB), a

	ld hl, (#_inlib_portB + 1)  ; read current button bits
	ld (#_inlib_portB + 3), hl  ; copy to previous button bits

	call _inlib_getportB        ; get button bits in l
	ld h, #0                    ; button high byte is unused

	ld (#_inlib_portB + 1), hl  ; write new button state

	ret

	__endasm;
}

void inlib_init(void)
{
	inlib_port3F_last = 0x0F;

}

unsigned short inlib_keysPressed(unsigned char port) __z88dk_fastcall __naked
{
	__asm

	; We want bits set to 1 to indicate a new button press.
	;
	; prev_btn   btn -> Wanted
	; 0          1   -> 1  (new press)
	; 1          1   -> 0  (still pressed)
	; 1          0   -> 0  (release)
	; 0          0   -> 0  (still not pressed)

	ld a, l
	or a
	jp nz, keys_pressed_port2

keys_pressed_port1:
	ld bc, (#_inlib_portA + 1) ; Read current button bits
	ld de, (#_inlib_portA + 3) ; Read previous button bits

	ld a, d                   ; Get previous bits
	cpl                       ; Invert
	and a, b                  ; And with current bits
	ld h,a                    ; Return in H

	ld a, e                   ; Get previous bits
	cpl                       ; Invert
	and a, c                  ; And with current bits
	ld l,a                    ; Return in L

	ret

keys_pressed_port2:
	ld bc, (#_inlib_portB + 1) ; Read current button bits
	ld de, (#_inlib_portB + 3) ; Read previous button bits

	ld a, d                   ; Get previous bits
	cpl                       ; Invert
	and a, b                  ; And with current bits
	ld h,a                    ; Return in H

	ld a, e                   ; Get previous bits
	cpl                       ; Invert
	and a, c                  ; And with current bits
	ld l,a                    ; Return in L

	ret

  __endasm;
}


