/* Code by Raphael Assenat [raphnet] */
#include "inlib.h"

// Disable unreferenced function argument warning caused
// by naked functions
#pragma disable_warning 85

// Delays implemented using DJNZ (13 cycles)

// Delay after lowering TR before reading buttons
#define TR_LOW_BTN_DELAY  16
// Delay after changing TH before reading a nibble
#define TH_POST_DELAY     24

void inlib_readGraphicBoard(unsigned char port) __z88dk_fastcall __naked
{
  __asm

  ld a, l
  or a
  jp nz, read_graphicboard_B

read_graphicboard_A:
  call _inlib_portA_TH_high
  call _inlib_portA_TR_low
  call _inlib_portA_TH_out_TR_out

  ld b, #TR_LOW_BTN_DELAY
delayA1:
  djnz delayA1
 

	ld hl, (#_inlib_portA + 1)  ; read current button bits
	ld (#_inlib_portA + 3), hl  ; copy to previous button bits

  in a,(#0xDC)                ; Get current button status
  rlca                        ; Bits 0-2 hold the buttons.
  rlca
  rlca
  rlca
  rlca

  bit 1, a                    ; Check TL
  jp Z, pen_activeA
  and a, #0xe0                ; Keep only buttons, report pen (bit 4) as inactive
  jp pen_doneA
pen_activeA:
  and a, #0xe0
  or a, #0x10                 ; Report pen active
pen_doneA:
  xor a, #0xe0                ; Make buttons active high

  ld (_inlib_portA + 1), a    ; Write buttons low byte
  xor a
  ld (_inlib_portA + 2), a    ; Write buttons high byte (0)

  call graphicboard_getByteA
  ld (_inlib_portA + 5), a    ; Write pressure byte
  call graphicboard_getByteA
  ld (_inlib_portA + 6), a    ; Write X byte
  ld d, a                     ; Copy to D
  call graphicboard_getByteA
  ld (_inlib_portA + 7), a    ; Write Y byte

  ; A disconnected board, or SMS controller will read all 1s, resulting
  ; in a 255,255 coordinate which falls outside the observed range on
  ; a real Graphic Board.
  cp a, #0xff                 ; Check Y value still in A
  jp nz, seemsValidA
  ld a, d                     ; Retreive X value
  cp a, #0xff
  jp nz, seemsValidA

  ld a, #INLIB_TYPE_NONE
  jp doneA
seemsValidA:
  ld a, #INLIB_TYPE_GRAPHIC_BOARD
doneA:
  ld (_inlib_portA), a        ; Write device type
  call _inlib_portA_TR_high
 
  ret 

  ; Returns one byte in A
  ; Changes BC
  ; Returns with TH high
graphicboard_getByteA:
  call _inlib_portA_TH_low    ; read nibble upper bits

  ld b, #TH_POST_DELAY        ; Wait until the graphic board reacts
delayA2:
  djnz delayA2

  in a,(#0xDC) 
  and a, #0x0f
  rlca
  rlca
  rlca
  rlca
  ld c,a                      ; Save upper nibble in C

  call _inlib_portA_TH_high   ; read nibble lower bits

  ld b, #TH_POST_DELAY        ; Wait until the graphic board reacts
delayA3:
  djnz delayA3


  in a,(#0xDC)          
  and a, #0x0f                ; Keep only low 4 bits
  or a,c                      ; Combine with previous nibble

  ret 


read_graphicboard_B:

  call _inlib_portB_TH_high
  call _inlib_portB_TR_low
  call _inlib_portB_TH_out_TR_out

  ld b, #TR_LOW_BTN_DELAY
delayB1:
  djnz delayB1

	ld hl, (#_inlib_portB + 1)  ; read current button bits
	ld (#_inlib_portB + 3), hl  ; copy to previous button bits

  call _inlib_getportB        ; Returns value in A and L
  rlca                        ; Align
  rlca
  rlca
  rlca
  rlca

  bit 1, a                    ; Check TL
  jp Z, pen_activeB
  and a, #0xe0                ; Keep only buttons, report pen (bit 4) as inactive
  jp pen_doneB
pen_activeB:
  and a, #0xe0
  or a, #0x10                 ; Report pen active
pen_doneB:
  xor a, #0xe0                ; Make buttons active high

  ld (_inlib_portB + 1), a    ; Write buttons low byte
  xor a
  ld (_inlib_portB + 2), a    ; Write buttons high byte (0)

  call graphicboard_getByteB
  ld (_inlib_portB + 5), a    ; Write pressure byte
  call graphicboard_getByteB
  ld (_inlib_portB + 6), a    ; Write X byte
  ld d, a                     ; Copy to D
  call graphicboard_getByteB
  ld (_inlib_portB + 7), a    ; Write Y byte
 
  ; A disconnected board, or SMS controller will read all 1s, resulting
  ; in a 255,255 coordinate which falls outside the observed range on
  ; a real Graphic Board.
  cp a, #0xff                 ; Check Y value still in A
  jp nz, seemsValidB
  ld a, d                     ; Retreive X value
  cp a, #0xff
  jp nz, seemsValidB

  ld a, #INLIB_TYPE_NONE
  jp doneB
seemsValidB:
  ld a, #INLIB_TYPE_GRAPHIC_BOARD
doneB:
  ld (_inlib_portB), a        ; Write device type
  
  call _inlib_portB_TR_high
 
  ret 

  ; Returns one byte in A
  ; Changes C
  ; Returns with TH high
graphicboard_getByteB:
  call _inlib_portB_TH_low    ; read nibble upper bits

  ld b, #TH_POST_DELAY        ; Wait until the graphic board reacts
delayB2:
  djnz delayB2

  call _inlib_getportB        ; Returns value in A and L
  and a, #0x0f
  rlca
  rlca
  rlca
  rlca
  ld c,a                      ; Save upper nibble in C

  call _inlib_portB_TH_high   ; read nibble lower bits

  ld b, #TH_POST_DELAY        ; Wait until the graphic board reacts
delayB3:
  djnz delayB3

  call _inlib_getportB
  and a, #0x0f                ; Keep only low 4 bits
  or a,c                      ; Combine with previous nibble

  ret 

  __endasm;
}

