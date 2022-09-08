#include "inlib.h"

// Disable unreferenced function argument warning caused
// by naked functions
#pragma disable_warning 85

// Reference output when reading a 6 button pad
//
// TH high      Up/Down/Left/Right/B/C
// TH low       Up/Down/0/0      << If left/right bits not both 0, SMS controller
//
// TH high      Up/Down/Left/Right/B/C
// TH low       Up/Down/0/0
//
// TH high      Up/Down/Left/Right/B/C
// TH low       0/0/0/0          << If any of those is not 0, 3 button pad
//
// TH high      Z/Y/X/Mode
// TH low       1/1/1/1
//

void inlib_readMDpad(unsigned char port) __naked __z88dk_fastcall
{
	__asm

	ld a, l
	or a
	jr nz, pollmd6_portB

pollmd6_portA:
	call _inlib_portA_TH_out_TR_in
	call _inlib_portA_TH_high
	in a,(#0xDC)                  ; Get port A bits (up/down/left/right/tl/tr)
	and a, #0x3F                  ; Keep only port A bits (bits 0-5)
	ld b, a                       ; Save in B
	; Refuse opposite directions
	and a, #0x03                  ; Keep only up/down bits
	jr z, portA_is_INVALID        ; Both low = invalid
	ld a, b                       ; Get full value again
	and a, #0x0C                  ; Keep only left/right bits
	jr z, portA_is_INVALID        ; Both low = invalid

	call _inlib_portA_TH_low
	in a,(#0xDC)                  ; Get port A bits
	ld c, a                       ; Save A/Start in C
	and a, #0x0C                  ; If left/right are not both 0, this is not a MD controller.
	jr nz, portA_is_SMS

	; Second pulse returns the same data again... Do an additional check
	call _inlib_portA_TH_high
	call _inlib_portA_TH_low
	in a,(#0xDC)                  ; Get port A bits
	and a, #0x0C                  ; If left/right are not both 0, this is not a MD controller.
	jr nz, portA_is_SMS

	; Thrid pulse returns same data when TH high, then all 0s when TH low
	call _inlib_portA_TH_high
	call _inlib_portA_TH_low
	in a,(#0xDC)
	and a, #0x0F                  ; If all of left/right/up/down are 0, this is a 6 button controller
	jp nz, portA_is_MD3

	; Fourth pulse returns extra buttons when high
	call _inlib_portA_TH_high
	in a,(#0xDC)
	ld d, a                       ; Save in D
	call _inlib_portA_TH_low

	; fallthrough
portA_is_MD6:
	call _inlib_portA_TH_high
	ld a, #INLIB_TYPE_MD6
	ld hl, #_inlib_portA
	jp common_6

portA_is_MD3:
	call _inlib_portA_TH_high
	ld a, #INLIB_TYPE_MD3
	ld hl, #_inlib_portA
	jp common_3

portA_is_SMS:
	call _inlib_portA_TH_high
	ld a, #INLIB_TYPE_SMS
	ld hl, #_inlib_portA
	jp common_sms

portA_is_INVALID:
	call _inlib_portA_TH_high
	ld a, #INLIB_TYPE_NONE
	ld hl, #_inlib_portA
	ret

pollmd6_portB:
	call _inlib_portB_TH_out_TR_in
	call _inlib_portB_TH_high
	call _inlib_getportB
	ld b, l                       ; Copy bits to B
	; Refuse opposite ditections
	ld a, l
	and a, #0x03                  ; Keep only up/down bits
	jr z, portB_is_INVALID        ; Both low = invalid
	ld a, l                       ; Get full value again
	and a, #0x0C                  ; Keep only left/right bits
	jr z, portB_is_INVALID        ; Both low = invalid

	call _inlib_portB_TH_low
	call _inlib_getportB
	ld a, l                       ; Copy to A for inspection
	and a, #0x0C                  ; If left/right are not both 0, this is not a MD controller.
	jr nz, portB_is_SMS
	ld c, l                       ; Save A/Start in C

	; Second pulse returns the same data again...
	call _inlib_portB_TH_high
	call _inlib_portB_TH_low

	; Thrid pulse returns same data when TH high, then all 0s when TH low
	call _inlib_portB_TH_high
	call _inlib_portB_TH_low
	call _inlib_getportB          ; Get all directions bits 0 (indicates 6 buttons controller)
	ld a, l                       ; Copy to A for inspection
	and a, #0x0F                  ; If all of left/right/up/down are 0, this is a 6 button controller
	jp nz, portB_is_MD3

	; Fourth pulse returns extra buttons when high
	call _inlib_portB_TH_high
	call _inlib_getportB		  ; Get X/Y/Z and Mode button bits
	ld d, l                       ; Save in D
	call _inlib_portB_TH_low

	; fallthrough
portB_is_MD6:
	call _inlib_portB_TH_high
	ld a, #INLIB_TYPE_MD6
	ld hl, #_inlib_portB
	jp common_6

portB_is_MD3:
	call _inlib_portB_TH_high
	ld a, #INLIB_TYPE_MD3
	ld hl, #_inlib_portB
	jp common_3

portB_is_SMS:
	call _inlib_portB_TH_high
	ld a, #INLIB_TYPE_SMS
	ld hl, #_inlib_portB
	jp common_sms

portB_is_INVALID:
	call _inlib_portB_TH_high
	ld a, #INLIB_TYPE_NONE
	ld hl, #_inlib_portB
	ret


common_3:
	ld d, #0xff
common_6:
	ld (hl), a					  ; Structure type member
	inc hl
	ld a, c                       ; Get Start/A extra buttons bits from C (bits 4,5)
	and a, #0x30
	rlca                          ; Align them to the left
	rlca
	or a, b                       ; Combine with up/down/left/right/b/c
	cpl                           ; Invert bits
	ld e, (hl)                    ; Save old button bits to E
	ld (hl), a                    ; Write new buttons low byte
	inc hl
	ld a, d                       ; Retreive X/Y/Z/mode from D
	or a, #0xf0                   ; Set unused bits
	cpl                           ; Invert bits
	ld d, (hl)                    ; Save old button bits to D
	ld (hl), a                    ; Write new button high byte
	inc hl
	ld (hl), e                    ; Store previous button low byte
	inc hl
	ld (hl), d                    ; Store previous button high byte
	ret

common_sms:
	ld (hl), a                    ; Structure type member
	inc hl
	ld a, b                       ; Get SMS buttons bits from B
	cpl                           ; Invert bits
	and a, #0x3F                  ; Clear unused bits
	ld e, (hl)                    ; Save old buttons bits to E
	ld (hl), a                    ; Write buttons low byte
	inc hl
	xor a
	ld d, (hl)                    ; Save old button high byte
	ld (hl), a                    ; Clear buttons high byte
	inc hl
	ld (hl), e                    ; Store old button low byte
	inc hl
	ld (hl), d                    ; Store old button high byte
	ret

	__endasm;
}


