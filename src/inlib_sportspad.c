/* Code by Raphael Assenat [raphnet] */
#include "inlib.h"

// Disable unreferenced function argument warning caused
// by naked functions
#pragma disable_warning 85

// Target: 80us
#define LOOPS_WAIT_FIRST_NIBBLE	14

// Target: 40us
#define LOOPS_WAIT_NIBBLE	4

void inlib_readSportsPad(unsigned char port) __z88dk_fastcall __naked
{
  __asm

  ld a, l
  or a
  jp nz, read_sportspad_B

read_sportspad_A:
  call _inlib_portA_TH_out_TR_in

  ld bc, (#_inlib_portA + 1) ; Read current button bits
  ld (#_inlib_portA + 3), bc ; Copy to previous button bits

  ;;; read nibble 1 (X upper bits)
  call _inlib_portA_TH_low
  ld b, #LOOPS_WAIT_FIRST_NIBBLE
$1:
  nop
  djnz $1
  in a,(#0xDC)          ; Read data
  add a,a               ; Move nibble to bits 7-3
  add a,a
  add a,a
  add a,a
  ld c,a                ; Save it in C for later

  ;;; read nibble 2 (X lower bits)
  call _inlib_portA_TH_high
  ld b, #LOOPS_WAIT_NIBBLE
$2:
  nop
  djnz $2
  in a,(#0xDC)          ; Read Data
  and a, #0x0f          ; Keep only low 4 bits
  or a,c                ; Combine with previous nibble
  neg
  ld d, a               ; Save X value in D

  ;;; read nibble 3 (Y upper bits)
  call _inlib_portA_TH_low
  ld b, #LOOPS_WAIT_NIBBLE
$3:
  nop
  djnz $3
  in a,(#0xDC)          ; Read data
  add a,a               ; Move nibble to bits 7-3
  add a,a
  add a,a
  add a,a
  ld c,a                ; Save it in C for later

  ;;; read nibble 4 (Y lower bits)
  call _inlib_portA_TH_high
  ld b, #LOOPS_WAIT_NIBBLE
$4:
  nop
  djnz $4
  in a,(#0xDC)          ; Read Data
  and a, #0x0f          ; Keep only low 4 bits
  or a,c                ; Combine with previous nibble
  neg
  ld e, a               ; Save Y value in E

  ;;; Populate the btn field with current button status.

  in a,(#0xDC)          ; Now stample the button
  and a,#0x30           ; Keep only TL(4) and TR(5)
  xor a,#0x30           ; Invert logic (1 = pressed)
  ld c, a               ; Save button status in C
  ld hl, #_inlib_portA

  jp sportspad_common


read_sportspad_B:
  call _inlib_portB_TH_out_TR_in

  ld bc, (#_inlib_portB + 1) ; Read current button bits
  ld (#_inlib_portB + 3), bc ; Copy to previous button bits

  ;;; read nibble 1 (X upper bits)
  call _inlib_portB_TH_low
  ld b, #LOOPS_WAIT_FIRST_NIBBLE
$5:
  nop
  djnz $5
  in a,(#0xDD)          ; Get nibble bits 2-3 from DD
  and a, #0x03          ; (keep lower 2 bits)
  ld e, a               ; Save in e
  in a,(#0xDC)          ; Get nibble bits 0-1 from DC
  and a, #0xC0          ; (keep upper 2 bits)
  or a, e               ; Combine bits
  rrca                  ; Rotate so the nibble occupies
  rrca                  ; the upper 4 bits in the byte
  ld c,a                ; Save the result for later

  ;;; read nibble 2 (X lower bits)
  call _inlib_portB_TH_high
  ld b, #LOOPS_WAIT_NIBBLE
$6:
  nop
  djnz $6

  in a,(#0xDD)          ; Get nibble bits 2-3 from DD
  and a, #0x03          ; (keep lower 2 bits)
  ld e, a               ; Save in e
  in a,(#0xDC)          ; Get nibble bits 0-1 from DC
  and a, #0xC0          ; (keep upper 2 bits)
  or a, e               ; Combine bits
  rlca                  ; Rotate so the nibble occupies
  rlca                  ; the lower 4 bits in the byte

  or a, c               ; Combine the two nibbles to form a byte
  neg                   ; Adjust direction

  ld d, a               ; Saev X in D

  ;;; read nibble 3 (Y upper bits)
  call _inlib_portB_TH_low
  ld b, #LOOPS_WAIT_NIBBLE
$7:
  nop
  djnz $7
  in a,(#0xDD)          ; Get nibble bits 2-3 from DD
  and a, #0x03          ; (keep lower 2 bits)
  ld e, a               ; Save in e
  in a,(#0xDC)          ; Get nibble bits 0-1 from DC
  and a, #0xC0          ; (keep upper 2 bits)
  or a, e               ; Combine bits
  rrca                  ; Rotate so the nibble occupies
  rrca                  ; the upper 4 bits in the byte
  ld c,a                ; Save the result for later

  ;;; read nibble 3 (Y lower bits)
  call _inlib_portB_TH_high
  ld b, #LOOPS_WAIT_NIBBLE
$8:
  nop
  djnz $8

  in a,(#0xDD)          ; Get nibble bits 2-3 from DD
  and a, #0x03          ; (keep lower 2 bits)
  ld e, a               ; Save in e
  in a,(#0xDC)          ; Get nibble bits 0-1 from DC
  and a, #0xC0          ; (keep upper 2 bits)
  or a, e               ; Combine bits
  rlca                  ; Rotate so the nibble occupies
  rlca                  ; the lower 4 bits in the byte

  or a, c               ; Combine the two nibbles to form a byte
  neg                   ; Adjust direction

  ld e, a               ; Save Y in E

  ;;; Populate the btn field with current button status.
  in a,(#0xDD)
  and a,#0x0c           ; Keep only TL(2) and TR(3)
  xor a,#0x0c           ; Invert logic (1 = pressed)
  rlca                  ; Align button 1 with bit 4 (button 1)
  rlca
  ld c, a               ; Save Button status in C

  ld hl, #_inlib_portB
; jp sportspad_common


  ; Final step, common to both pads. Write to memory structure.
  ; Arguments are
  ;   hl : pointer to start of structure
  ;   c  : button byte
  ;   d  : X data
  ;   e  : Y data
sportspad_common:
  ld a, #INLIB_TYPE_SPORTSPAD
  ld (hl), a           ; Set type member
  inc hl

  ld (hl), c           ; Button status (low byte)
  inc hl

  xor a
  ld (hl), a           ; Button status (high byte - inactive)
  inc hl

  inc hl               ; Skip previous button status low..
  inc hl               ; .. and high bytes

  ld (hl), d           ; X
  inc hl

  ld (hl), e           ; Y

  ret

  __endasm;
}

