/* Code by Raphael Assenat [raphnet] */
#include "inlib.h"

// Disable unreferenced function argument warning caused
// by naked functions
#pragma disable_warning 85

#define CLKHANDSHAKE	255

void inlib_readMDmouse(unsigned char port) __naked __z88dk_fastcall
{
	__asm

	ld a, l
	or a
	jp nz, read_mouse_portB

read_mouse_portA:
	ld bc, (#_inlib_portA + 1) ; Read current button bits
	ld (#_inlib_portA + 3), bc ; Copy to previous button bits

	call _inlib_portA_TH_high
	call _inlib_portA_TR_high
	call _inlib_portA_TH_out_TR_out
	; TH and TR high here.

	; Check ID (0)
	in a, (#0xDC)              ; Read pin status
	and #0x0f                  ; Check nibble bits
	jp NZ, mouseA_error        ; This should be 0

	call _inlib_portA_TH_low
	in a, (#0xDC)              ; Read pin status
	and a, #0xf                ; Keep only nibble bits
	cp #0x0b                   ; Compare with expected value
	jp NZ, mouseA_error        ; This should be 0xB

	; Clocking starts here

	call mouse_getByteA        ; First byte is always FF (this could be tested)
	or a                       ; Check if there was
	jp nz, mouseA_error        ; a timeout...

	call mouse_getByteA        ; Second byte is buttons
	or a                       ; Check if there was
	jp nz, mouseA_error        ; a timeout...
	ld a, l                    ; Get returned byte from L
	rrca                       ; Bit 4 is the X MSb
	rrca                       ; Bit 5 is the Y MSb
	rrca                       ; Bit 0-3 are buttons
	rrca                       ; Buttons moved to high nibble, to skip direction bits
	ld e, a                    ; Save this rotated byte in E for later use of the X/Y MSb
	and a, #0xF0               ; Clear non-button bits
	ld (_inlib_portA + 1), a   ; Write button byte to structure (low byte)
	xor a
	ld (_inlib_portA + 2), a   ; Clear buttons high byte

	call mouse_getByteA        ; Third byte is X
	or a                       ; Check if there was
	jp nz, mouseA_error        ; a timeout...
	ld a, l                    ; Get returned byte from L
	ld (_inlib_portA + 5), a   ; Write to structure (X LSB)
	ld a, e                    ; Retreive 9th bit
	and a, #0x1
	dec a                      ; 0x01 becomes 0xFF (0x01->0x00->0xFF)
	cpl                        ; 0x00 becomes 0x00 (0x00->0xFF->0x00)
	ld (_inlib_portA + 6), a   ; Write X MSB

	call mouse_getByteA        ; Fourth byte is Y
	or a                       ; Check if there was
	jp nz, mouseA_error        ; a timeout...
	ld a, l                    ; Get returned byte from L
	ld (_inlib_portA + 7), a   ; Write to structure (Y LSB)
	ld a, e                    ; Retreive 9th bit
	rrca
	and a, #0x1
	dec a
	cpl
	ld (_inlib_portA + 8), a   ; Write Y MSB

	ld a, #INLIB_TYPE_MDMOUSE  ; Set type member to indicate a mouse
	ld (_inlib_portA), a

 	; Invert Y axis to match screen
	ld hl, (_inlib_portA + 5)
	xor a
	sub l
	ld l,a
	sbc a,a
	sub h
	ld h,a
	ld (_inlib_portA + 5), hl

	call _inlib_portA_TH_high
	ret

mouseA_error:
	; Not a mouse or mouse disconnected. Reconfigure port.
	call _inlib_portA_TH_out_TR_in
	call _inlib_portA_TH_high
	ld a, #INLIB_TYPE_NONE
	ld (_inlib_portA), a
	ret

    ;;; Subroutine to receive one byte
	; Returns Byte in L. If A non-zero, a timeout occured.
mouse_getByteA:
	call _inlib_portA_TR_low
	ld b, #CLKHANDSHAKE
wait_tl_lowA:
    in a, (#0xDC)
	bit 4, a
	jp Z, tl_lowA
	djnz wait_tl_lowA
	jp timeoutA
tl_lowA:
    ; bits 0-3 if A hold the upper nibble. Shift left...
	sla a
	sla a
	sla a
	sla a
	ld l, a ; Save in L for later

	call _inlib_portA_TR_high
	ld b, #CLKHANDSHAKE
wait_tl_highA:
    in a, (#0xDC)
	bit 4, a
	jp NZ, tl_highA
	djnz wait_tl_highA
	jp timeoutA
tl_highA:
	and a, #0x0f        ; bits 0-3 of A holds the lower nibble.
	or a, l             ; Combine with upper nibble in L
	ld l, a             ; Value returned in L
	xor a               ; A = 0 means OK
	ret

timeoutA:
	ld a, #0x01         ; A != 0 means timeout
	ret


read_mouse_portB:
	ld bc, (#_inlib_portB + 1) ; Read current button bits
	ld (#_inlib_portB + 3), bc ; Copy to previous button bits
	call _inlib_portB_TH_high
	call _inlib_portB_TR_high
	call _inlib_portB_TH_out_TR_out
	; TH and TR high here.

	; Check ID (0)
	call _inlib_getportB       ; Read pin status
	ld a, l
	and #0x0f                  ; Check nibble bits
	jp NZ, mouseB_error        ; This should be 0

	call _inlib_portB_TH_low
	call _inlib_getportB       ; Read pin status
	ld a, l
	and a, #0xf                ; Keep only nibble bits
	cp #0x0b                   ; Compare with expected value
	jp NZ, mouseB_error        ; This should be 0xB

	; Clocking starts here

	call mouse_getByteB        ; First byte is always FF (this could be tested)
	or a                       ; Check if there was
	jp nz, mouseB_error        ; a timeout...

	call mouse_getByteB        ; Second byte is buttons
	or a                       ; Check if there was
	jp nz, mouseB_error        ; a timeout...
	ld a, l                    ; Get returned byte from L
	rrca                       ; Bit 4 is the X MSb
	rrca                       ; Bit 5 is the Y MSb
	rrca                       ; Bit 0-3 are buttons
	rrca                       ; Buttons moved to high nibble, to skip direction bits
	ld e, a                    ; Save this rotated byte in E for later use of the X/Y MSb
	and a, #0xF0               ; Clear non-button bits
	ld (_inlib_portB + 1), a   ; Write button byte to structure (low byte)
	xor a
	ld (_inlib_portB + 2), a   ; Clear buttons high byte

	call mouse_getByteB        ; Third byte is X
	or a                       ; Check if there was
	jp nz, mouseB_error        ; a timeout...
	ld a, l                    ; Get returned byte from L
	ld (_inlib_portB + 5), a   ; Write to structure (X LSB)
	ld a, e                    ; Retreive 9th bit
	and a, #0x1
	dec a                      ; 0x01 becomes 0xFF (0x01->0x00->0xFF)
	cpl                        ; 0x00 becomes 0x00 (0x00->0xFF->0x00)
	ld (_inlib_portB + 6), a   ; Write X MSB

	call mouse_getByteB        ; Fourth byte is Y
	or a                       ; Check if there was
	jp nz, mouseB_error        ; a timeout...
	ld a, l                    ; Get returned byte from L
	ld (_inlib_portB + 7), a   ; Write to structure (Y LSB)
	ld a, e                    ; Retreive 9th bit
	rrca
	and a, #0x1
	dec a
	cpl
	ld (_inlib_portB + 8), a   ; Write Y MSB

	ld a, #INLIB_TYPE_MDMOUSE  ; Set type member to indicate a mouse
	ld (_inlib_portB), a

 	; Invert Y axis to match screen
	ld hl, (_inlib_portB + 5)
	xor a
	sub l
	ld l,a
	sbc a,a
	sub h
	ld h,a
	ld (_inlib_portB + 5), hl


	call _inlib_portB_TH_high
	ret

mouseB_error:
	; Not a mouse or mouse disconnected. Reconfigure port.
	call _inlib_portB_TH_out_TR_in
	call _inlib_portB_TH_high
	ld a, #INLIB_TYPE_NONE
	ld (_inlib_portB), a
	ret

    ;;; Subroutine to receive one byte
	; Returns Byte in L. If A non-zero, a timeout occured.
mouse_getByteB:
	call _inlib_portB_TR_low
	ld b, #CLKHANDSHAKE
wait_tl_lowB:
	call _inlib_getportB
	ld a, l
	bit 4, a
	jp Z, tl_lowB
	djnz wait_tl_lowB
	jp timeoutB
tl_lowB:
    ; bits 0-3 if A hold the upper nibble. Shift left...
	sla a
	sla a
	sla a
	sla a
	ld h, a ; Save in L for later

	call _inlib_portB_TR_high
	ld b, #CLKHANDSHAKE
wait_tl_highB:
	call _inlib_getportB
	ld a, l
	bit 4, a
	jp NZ, tl_highB
	djnz wait_tl_highB
	jp timeoutB
tl_highB:
	and a, #0x0f        ; bits 0-3 of A holds the lower nibble.
	or a, h             ; Combine with upper nibble in H
	ld l, a             ; Value returned in L
	xor a               ; A = 0 means OK
	ret
timeoutB:
	ld a, #0x01         ; A != 0 means timeout
	ret

	__endasm;
}
