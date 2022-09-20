#include "inlib.h"

// Disable unreferenced function argument warning caused
// by naked functions
#pragma disable_warning 85

// VDP vertical counter port : 7E
// VDP horizontal counter port : 7F

#define HORIZONTAL_OFFSET 32

void inlib_pollLightPhaser_position(unsigned char port) __naked __z88dk_fastcall
{
  __asm

wait_vblank_end:
  in a, (#0x7E)                    ; Read vertical counter port
  cp a, #192                      
  jp NC, wait_vblank_end           ; Loop as long as count >= 192

  ld b, #0x00                      ; Init. max Y value
  ld c, #0x00                      ; Init. max X value
  ld d, #0xff                      ; Init. min Y value
  ld e, #0xff                      ; Init. min X value

  ; Jump to the appropriate poll routine
	ld a, l
	or a
	jr nz, pollphaser_raster_portB


pollphaser_raster_portA:
  call _inlib_portA_TH_in_TR_in

  ld hl, (#_inlib_portA + 1)       ; read current button bits
	ld (#_inlib_portA + 3), hl       ; copy to previous button bits

	call _inlib_getportA             ; get button bits in l
  ld a, l
  and a, #0x10                     ; keep only trigger button
  xor a, #0x10                     ; Make it active high
  ld l, a
	ld h, #0                         ; button high byte is unused
	ld (#_inlib_portA + 1), hl       ; write new button state

wait_th_lowA:
  in a, (#0xDD)                    ; Read port DD and check if bit
  and a, #0x40                     ; 6 (TH port A) is low.
  jp Z, th_lowA                    ; If it is, light has been sensed!

  in a, (#0x7E)                    ; Read vertical counter port to 
  cp a, #192                       ; check if this frame is complete.
  jp C, wait_th_lowA
 
  jp frame_doneA

th_lowA:
  in a, (#0x7e)                    ; Read vertical counter
  cp a, d                          ; Is it < D
  jp nc, skp1A
  ld d, a                          ; Record new min Y value
skp1A:
  cp a, b                          ; Is it >= B
  jp c, skp2A
  ld b, a                          ; Record new max Y value
skp2A:

  in a, (#0x7f)                    ; Read horizontal counter
  cp a, e                          ; Is it < E
  jp nc, skp3A
  ld e, a                          ; Record new min X value
skp3A:
  cp a, c                          ; Is it >= C
  jp c, skp4A
  ld c, a                          ; Record new max X value
skp4A:

wait_th_highA:
  in a, (#0xDD)                    ; Read port DD and check if bit
  and a, #0x40                     ; 6 (TH port A) is back high.
  jp NZ, wait_th_lowA              ; If it is, wait for the next falling edge
  
  in a, (#0x7E)                    ; Read vertical counter port to
  cp a, #192                       ; check if this frame is complete.
  jp C, wait_th_highA

  ; Fall through to frame_doneA

frame_doneA:
  ld hl, #_inlib_portA
  jp phaser_common_done


pollphaser_raster_portB:
  call _inlib_portB_TH_in_TR_in

  ld hl, (#_inlib_portB + 1)       ; read current button bits
	ld (#_inlib_portB + 3), hl       ; copy to previous button bits

	call _inlib_getportB             ; get button bits in l
  ld a, l
  and a, #0x10                     ; keep only trigger button
  xor a, #0x10                     ; Make it active high
  ld l, a
	ld h, #0                         ; button high byte is unused
	ld (#_inlib_portB + 1), hl       ; write new button state

wait_th_lowB:
  in a, (#0xDD)                    ; Read port DD and check if bit
  and a, #0x80                     ; 7 (TH port B) is low.
  jp Z, th_lowB                    ; If it is, light has been sensed!

  in a, (#0x7E)                    ; Read vertical counter port to 
  cp a, #192                       ; check if this frame is complete.
  jp C, wait_th_lowB
 
  jp frame_doneB

th_lowB:
  in a, (#0x7e)                    ; Read vertical counter
  cp a, d                          ; Is it < D
  jp nc, skp1B
  ld d, a                          ; Record new min Y value
skp1B:
  cp a, b                          ; Is it >= B
  jp c, skp2B
  ld b, a                          ; Record new max Y value
skp2B:

  in a, (#0x7f)                    ; Read horizontal counter
  cp a, e                          ; Is it < E
  jp nc, skp3B
  ld e, a                          ; Record new min X value
skp3B:
  cp a, c                          ; Is it >= C
  jp c, skp4B
  ld c, a                          ; Record new max X value
skp4B:

wait_th_highB:
  in a, (#0xDD)                    ; Read port DD and check if bit
  and a, #0x80                     ; 7 (TH port B) is high.
  jp NZ, wait_th_lowB              ; If it is, light has been sensed!

  in a, (#0x7E)                    ; Read vertical counter port to
  cp a, #192                       ; check if this frame is complete.
  jp C, wait_th_highB

  ; Fall through to frame_doneB

frame_doneB:
  ld hl, #_inlib_portB
  jp phaser_common_done


  ; Jump here with HL set to _inlib_portA/B
  ; and B,C,D,E holding the min/max X/Y values.
phaser_common_done:

  ; At least one of the min/max X/Y values should have changed.
  ld a, d
  cp a, #0xff                       ; Default if 0xFf
  jp NZ, phaser_common_ok
  cp a, e                           ; so A is 0xFF, check E
  jp NZ, phaser_common_ok

  ld a, b
  and a
  jp NZ, phaser_common_ok           ; Default is 0
  or c                              ; B was 0. check C
  jp NZ, phaser_common_ok

  ; If all min/max are still at the initial values, light was not sensed
  ; at all.
  jp phaser_common_timeout


  ; Jump here with HL set to _inlib_portA/B
phaser_common_ok:
  ld a, #INLIB_TYPE_PHASER_HIT
  ld (hl), a
  inc hl                    ; Skip type

  inc hl                    ; Skip button status low byte
  inc hl                    ; Skip button status high byte
  inc hl                    ; Skip previous status low byte
  inc hl                    ; Skip previous status high byte

  ; The horizontal counter port returns the upper 8 bits
  ; of a 9 bit value, we can add both values directly.

  ; However an offset is present on real hardware and using Emulicious.
  ; I think this is due to the gun reaction time and/or propagation of
  ; the signal. So X values greater than 255 (or 127 in this register)
  ; must be dealt with.

  ld a, c                   ; Add min and max X values
  add a, e
  jp NC, within255          ; No overflow. Good.

  sub a, #HORIZONTAL_OFFSET ; If it got greater than 255, subtract the offset.
  jp M, no_underflow        ; Hopefully this gives a result in range.
  ld a, #0xFF               ; If not, clamp it to the max value.
  jp no_underflow
within255:
  sub a, #HORIZONTAL_OFFSET ; Subtract the offset.
  jp NC, no_underflow       ; Avoid jumping from left to right in case the above
  xor a                     ; value turns out to be too high on some hardware
no_underflow:
  ld (hl), a                ; Store the average X value to abs.x
  inc hl                    ; Skip abs.x

  srl b                     ; Divide Y values by two
  srl d                     ; and then add them together
  ld a, b                   ; to compute the average.
  add a, d 
  ld (hl), a                ; Store the average Y value to abs.y

  ret

phaser_common_timeout:
  ld a, #INLIB_TYPE_PHASER  ; Only button status being reported
  ld (hl), a
  ret

  __endasm;
}

void inlib_pollLightPhaser_trigger(unsigned char port) __naked __z88dk_fastcall
{
	__asm

	ld a, l
	or a
	jr nz, pollphaser_portB

pollphaser_portA:
  call _inlib_portA_TH_in_TR_in

	ld a, #INLIB_TYPE_PHASER    ; set controller type
	ld (_inlib_portA), a

	ld hl, (#_inlib_portA + 1)  ; read current button bits
	ld (#_inlib_portA + 3), hl  ; copy to previous button bits

	call _inlib_getportA        ; get button bits in l
  ld a, l
  and a, #0x10                ; keep only trigger button
  xor a, #0x10                ; Invert it
  ld l, a
	ld h, #0                    ; button high byte is unused

	ld (#_inlib_portA + 1), hl  ; write new button state

	ret

pollphaser_portB:
  call _inlib_portB_TH_in_TR_in

	ld a, #INLIB_TYPE_PHASER    ; set controller type
	ld (_inlib_portB), a

	ld hl, (#_inlib_portB + 1)  ; read current button bits
	ld (#_inlib_portB + 3), hl  ; copy to previous button bits

	call _inlib_getportB        ; get button bits in l
  ld a, l
  and a, #0x10                ; keep only trigger button
  xor a, #0x10                ; Invert it
  ld l, a
	ld h, #0                    ; button high byte is unused

	ld (#_inlib_portB + 1), hl  ; write new button state

	ret

	__endasm;
}


